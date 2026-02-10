#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <filesystem>

#include "crypto.hpp"
#include "database.hpp"
#include "terminal.hpp"
#include "security.hpp"

namespace {
#ifdef _WIN32
	namespace fs = std::filesystem;
#else
	namespace fs = std::filesystem;
#endif
	std::string generate_unique_db_filename() {
		constexpr std::size_t kEntropyBytes = 16;
		const auto random = crypto::random_bytes(kEntropyBytes);
		std::ostringstream oss;
		oss << "passport_";
		oss << std::hex << std::setfill('0');
		for (const auto byte : random) {
			oss << std::setw(2) << static_cast<unsigned>(byte);
		}
		oss << ".db";
		return oss.str();
	}
	bool is_directory(const std::string& path) {
		struct stat info {};
		if (stat(path.c_str(), &info) != 0) {
			return false;
		}
		return (info.st_mode & S_IFDIR) != 0;
	}
	std::string normalise_db_path(const std::string& raw) {
		std::string path = raw;
		while (!path.empty() && (path.back() == ' ' || path.back() == '\t')) {
			path.pop_back();
		}
		if (path.empty()) {
			return path;
		}
		try {
			fs::path fs_path(path);
			if (!fs_path.is_absolute()) {
				fs_path = fs::absolute(fs_path);
			}
			path = fs_path.string();
		}
		catch (const std::exception& e) {
			terminal::show_error("Failed to convert path to absolute: " + std::string(e.what()));
			return "";
		}
		if (is_directory(path)) {
			const char last = path.back();
			const bool has_sep = (last == '/' || last == '\\');
			if (!has_sep) {
#ifdef _WIN32
				path += '\\';
#else
				path += '/';
#endif
			}
			path += generate_unique_db_filename();
		}
		return path;
	}
	bool file_exists(const std::string& path) {
		struct stat info {};
		return stat(path.c_str(), &info) == 0;
	}
	bool prompt_overwrite(const std::string& path) {
		std::cout << "\n  File already exists: " << path << "\n";
		std::cout << "  Do you want to overwrite it? (y/N): ";
		std::string response;
		std::getline(std::cin, response);
		if (response.empty()) {
			return false;
		}
		char first_char = std::tolower(static_cast<unsigned char>(response[0]));
		return (first_char == 'y');
	}
	bool create_new_database(
		PassportDatabase& db,
		std::string& db_path,
		std::string& master_pw)
	{
		const auto raw = terminal::prompt_for_input(
			"  Database path: ");
		if (terminal::is_field_empty(raw)) {
			terminal::show_error("File path cannot be empty.");
			return false;
		}
		db_path = normalise_db_path(raw);
		if (db_path.empty()) {
			terminal::show_error("Invalid file path.");
			return false;
		}
		if (file_exists(db_path)) {
			terminal::show_message("File already exists: " + db_path);
			if (!prompt_overwrite(db_path)) {
				terminal::show_message("Database creation cancelled.");
				return false;
			}
			terminal::show_message("Overwriting existing file...");
		}
		if (!security::check_file_access(db_path, true)) {
			terminal::show_error(
				"Cannot write to \"" + db_path
				+ "\". Check write permissions.");
			return false;
		}
		terminal::show_message("Saving to: " + db_path);
		master_pw = terminal::prompt_for_password(
			"  Set master password (min 12 chars): ");
		if (terminal::is_field_empty(master_pw)) {
			terminal::show_error("Master password cannot be empty.");
			return false;
		}
		if (!terminal::is_master_password_length_valid(master_pw)) {
			terminal::show_error(
				"Master password must be between "
				+ std::to_string(terminal::kPasswordMinLength_MasterPassword) + " and "
				+ std::to_string(terminal::kPasswordMaxLength_MasterPassword) + " characters.");
			return false;
		}
		const auto confirm =
			terminal::prompt_for_password("  Confirm master password: ");
		if (master_pw != confirm) {
			terminal::show_error("Passwords do not match.");
			return false;
		}
		try {
			if (!db.save_to_file(db_path, master_pw)) {
				terminal::show_error(
					"Cannot write to \"" + db_path
					+ "\". Check the path and permissions.");
				return false;
			}
		}
		catch (const std::exception& e) {
			terminal::show_error(std::string("Encryption failed: ") + e.what());
			return false;
		}
		terminal::show_success("New database created successfully.");
		return true;
	}
	bool open_existing_database(
		PassportDatabase& db,
		std::string& db_path,
		std::string& master_pw)
	{
		const auto raw = terminal::prompt_for_input(
			"  Database path: ");
		if (terminal::is_field_empty(raw)) {
			terminal::show_error("File path cannot be empty.");
			return false;
		}
		db_path = raw;
		while (!db_path.empty() && (db_path.back() == ' ' || db_path.back() == '\t')) {
			db_path.pop_back();
		}
		try {
			fs::path fs_path(db_path);
			if (!fs_path.is_absolute()) {
				fs_path = fs::absolute(fs_path);
			}
			db_path = fs_path.string();
		}
		catch (const std::exception& e) {
			terminal::show_error("Failed to convert path to absolute: " + std::string(e.what()));
			return false;
		}
		if (!file_exists(db_path)) {
			terminal::show_error("Database file does not exist: " + db_path);
			return false;
		}
		if (!security::check_file_access(db_path, false)) {
			terminal::show_error(
				"Cannot read from \"" + db_path
				+ "\". Check read permissions.");
			return false;
		}
		terminal::show_message("Opening: " + db_path);
		master_pw = terminal::prompt_for_password("  Master password: ");
		if (terminal::is_field_empty(master_pw)) {
			terminal::show_error("Master password cannot be empty.");
			return false;
		}
		if (!terminal::is_master_password_length_valid(master_pw)) {
			terminal::show_error(
				"Master password must be between "
				+ std::to_string(terminal::kPasswordMinLength_MasterPassword) + " and "
				+ std::to_string(terminal::kPasswordMaxLength_MasterPassword) + " characters."
			);
			return false;
		}
		try {
			if (!db.load_from_file(db_path, master_pw)) {
				terminal::show_error(
					"Failed to open database. File not found or corrupted.");
				return false;
			}
		}
		catch (const crypto::AeadError&) {
			terminal::show_error(
				"Failed to open database. Wrong password or corrupted file.");
			return false;
		}
		catch (const security::DeserialisationError& e) {
			terminal::show_error(
				std::string("Database integrity error: ") + e.what());
			return false;
		}
		catch (const std::exception& e) {
			terminal::show_error(std::string("Error: ") + e.what());
			return false;
		}
		terminal::show_success(
			"Database loaded. "
			+ std::to_string(db.password_record_count()) + " password record(s), "
			+ std::to_string(db.note_record_count()) + " note record(s) found."
		);
		return true;
	}
	void handle_list_records(const PassportDatabase& db) {
		if (db.record_count() == 0) {
			terminal::show_message("The database is empty. No records to display.");
			return;
		}
		const auto kind = terminal::prompt_record_kind_for_list();
		switch (kind) {
		case terminal::RecordKind::Password:
			terminal::display_password_records(db);
			break;
		case terminal::RecordKind::Note:
			terminal::display_note_records(db);
			break;
		}
	}
	void handle_add_record(PassportDatabase& db) {
		const auto kind = terminal::prompt_record_kind_for_add();

		switch (kind) {
		case terminal::RecordKind::Password: {
			auto rec = terminal::prompt_for_new_password_record();
			db.add_password_record(rec);
			terminal::show_success("Password record added successfully.");
			break;
		}
		case terminal::RecordKind::Note: {
			auto rec = terminal::prompt_for_new_note_record();
			db.add_note_record(rec);
			terminal::show_success("Note record added successfully.");
			break;
		}
		}
	}
	void handle_remove_record(PassportDatabase& db) {
		if (db.record_count() == 0) {
			terminal::show_message("The database is empty. Nothing to remove.");
			return;
		}
		const auto kind = terminal::prompt_record_kind_for_remove();
		switch (kind) {
		case terminal::RecordKind::Password: {
			if (db.password_record_count() == 0) {
				terminal::show_message("No password records to remove.");
				return;
			}
			terminal::display_password_records(db);
			const auto input = terminal::prompt_for_input(
				"\n  Enter record number to remove (0 to cancel): ");
			std::size_t idx = 0;
			try {
				idx = std::stoul(input);
			}
			catch (...) {
				terminal::show_error("Invalid input. Expected a number.");
				return;
			}
			if (idx == 0) {
				terminal::show_message("Removal cancelled.");
				return;
			}
			if (db.remove_password_record(idx - 1)) {
				terminal::show_success(
					"Password record #" + std::to_string(idx) + " removed.");
			}
			else {
				terminal::show_error("Invalid record number. No changes made.");
			}
			break;
		}
		case terminal::RecordKind::Note: {
			if (db.note_record_count() == 0) {
				terminal::show_message("No note records to remove.");
				return;
			}
			terminal::display_note_records(db);
			const auto input = terminal::prompt_for_input(
				"\n  Enter record number to remove (0 to cancel): ");
			std::size_t idx = 0;
			try {
				idx = std::stoul(input);
			}
			catch (...) {
				terminal::show_error("Invalid input. Expected a number.");
				return;
			}
			if (idx == 0) {
				terminal::show_message("Removal cancelled.");
				return;
			}
			if (db.remove_note_record(idx - 1)) {
				terminal::show_success(
					"Note record #" + std::to_string(idx) + " removed.");
			}
			else {
				terminal::show_error("Invalid record number. No changes made.");
			}
			break;
		}
		}
	}
	void run_main_menu(
		PassportDatabase& db,
		const std::string& db_path,
		const std::string& master_pw)
	{
		bool running = true;
		while (running) {
			const int choice = terminal::show_main_menu();
			switch (choice) {
			case 1:
				handle_list_records(db);
				break;
			case 2:
				handle_add_record(db);
				break;
			case 3:
				handle_remove_record(db);
				break;
			case 4:
				if (db.save_to_file(db_path, master_pw)) {
					terminal::show_success("Database saved.");
				}
				else {
					terminal::show_error("Failed to save database.");
				}
				break;
			case 5:
				if (db.save_to_file(db_path, master_pw)) {
					terminal::show_success("Database saved. Goodbye!");
				}
				else {
					terminal::show_error("Failed to save database.");
				}
				running = false;
				break;
			case 0:
				terminal::show_message("Exiting without saving. Goodbye!");
				running = false;
				break;
			default:
				terminal::show_error(
					"Invalid option. Please choose from the menu.");
				break;
			}
		}
	}
}
int main() {
	try {
		crypto::initialise();
		terminal::clear_screen();
		terminal::show_welcome_banner();
		PassportDatabase db;
		std::string      db_path;
		std::string      master_pw;
		bool             ready = false;
		while (!ready) {
			const int choice = terminal::show_startup_menu();
			switch (choice) {
			case 1:
				ready = create_new_database(db, db_path, master_pw);
				break;
			case 2:
				ready = open_existing_database(db, db_path, master_pw);
				break;
			case 0:
				terminal::show_message("Goodbye!");
				return 0;
			default:
				terminal::show_error(
					"Invalid option. Please press C, O, or Q.");
				break;
			}
			if (!ready) {
				std::cout << '\n';
			}
		}
		run_main_menu(db, db_path, master_pw);

	}
	catch (const std::exception& e) {
		std::cerr << "\n  [FATAL] " << e.what() << '\n';
		return 1;
	}
	catch (...) {
		std::cerr << "\n  [FATAL] An unknown error occurred.\n";
		return 1;
	}
	return 0;
}