#include "terminal.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <regex>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace terminal {
	namespace {
		std::string format_field_display(const std::string& field) {
			return field.empty() ? "---" : field;
		}
	}

	void clear_screen() {
#ifdef _WIN32
		std::system("cls");
#else
		std::system("clear");
#endif
	}

	void show_message(const std::string& msg) {
		std::cout << "  " << msg << '\n';
	}

	void show_error(const std::string& msg) {
		std::cout << "  [!] " << msg << '\n';
	}

	void show_success(const std::string& msg) {
		std::cout << "  [+] " << msg << '\n';
	}

	void show_welcome_banner() {
		std::cout
			<< '\n'
			<< "  ======================================\n"
			<< "        Secure Password Manager\n"
			<< "  ======================================\n"
			<< '\n';
	}

	std::string format_datetime(uint64_t ts) {
		if (ts == 0) {
			return "N/A";
		}
		const auto t = static_cast<std::time_t>(ts);
		std::tm tm_buf{};
#ifdef _WIN32
		localtime_s(&tm_buf, &t);
#else
		localtime_r(&t, &tm_buf);
#endif
		char buf[17];
		std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &tm_buf);
		return buf;
	}

	void display_password_records(const PassportDatabase& db) {
		const auto& all = db.password_records();

		if (all.empty()) {
			show_message("\n  No password records to display.");
			return;
		}
		constexpr std::size_t kDateColWidth = 16;
		std::size_t w_login = 5;
		std::size_t w_pass = 8;
		std::size_t w_url = 3;
		std::size_t w_note = 4;
		for (const auto& r : all) {
			std::string login_display = format_field_display(r.login);
			std::string pass_display = format_field_display(r.password);
			std::string url_display = format_field_display(r.url);
			std::string note_display = format_field_display(r.note);

			if (login_display.size() > w_login) w_login = login_display.size();
			if (pass_display.size() > w_pass) w_pass = pass_display.size();
			if (url_display.size() > w_url) w_url = url_display.size();
			if (note_display.size() > w_note) w_note = note_display.size();
		}
		std::cout << "\n  --- Password Records (* = required) ---\n\n";
		std::cout << "  "
			<< std::left
			<< std::setw(5) << "#"
			<< std::setw(kDateColWidth + 3) << "Date"
			<< std::setw(w_login + 3) << "Login*"
			<< std::setw(w_pass + 3) << "Password*"
			<< std::setw(w_url + 3) << "URL"
			<< std::setw(w_note + 3) << "Note"
			<< '\n';
		const auto total_w = 5 + (kDateColWidth + 3) + (w_login + 3) + (w_pass + 3)
			+ (w_url + 3) + (w_note + 3);
		std::cout << "  " << std::string(total_w, '-') << '\n';
		for (std::size_t i = 0; i < all.size(); ++i) {
			const auto& r = all[i];
			std::string login_display = format_field_display(r.login);
			std::string pass_display = format_field_display(r.password);
			std::string url_display = format_field_display(r.url);
			std::string note_display = format_field_display(r.note);
			std::cout << "  "
				<< std::left
				<< std::setw(5) << (i + 1)
				<< std::setw(kDateColWidth + 3) << format_datetime(r.date)
				<< std::setw(w_login + 3) << login_display
				<< std::setw(w_pass + 3) << pass_display
				<< std::setw(w_url + 3) << url_display
				<< std::setw(w_note + 3) << note_display
				<< '\n';
		}
		std::cout << "\n  Total password records: " << all.size() << '\n';
	}

	void display_note_records(const PassportDatabase& db) {
		const auto& all = db.note_records();
		if (all.empty()) {
			show_message("\n  No note records to display.");
			return;
		}
		constexpr std::size_t kDateColWidth = 16;
		std::size_t w_title = 5;
		std::size_t w_note = 4;
		for (const auto& r : all) {
			std::string title_display = format_field_display(r.title);
			std::string note_display = format_field_display(r.note);
			if (title_display.size() > w_title) w_title = title_display.size();
			if (note_display.size() > w_note) w_note = note_display.size();
		}
		std::cout << "\n  --- Note Records (* = required) ---\n\n";
		std::cout << "  "
			<< std::left
			<< std::setw(5) << "#"
			<< std::setw(kDateColWidth + 3) << "Date"
			<< std::setw(w_title + 3) << "Title*"
			<< std::setw(w_note + 3) << "Note"
			<< '\n';
		const auto total_w = 5 + (kDateColWidth + 3) + (w_title + 3) + (w_note + 3);
		std::cout << "  " << std::string(total_w, '-') << '\n';
		for (std::size_t i = 0; i < all.size(); ++i) {
			const auto& r = all[i];
			std::string title_display = format_field_display(r.title);
			std::string display_note = format_field_display(r.note);

			std::cout << "  "
				<< std::left
				<< std::setw(5) << (i + 1)
				<< std::setw(kDateColWidth + 3) << format_datetime(r.date)
				<< std::setw(w_title + 3) << title_display
				<< std::setw(w_note + 3) << display_note
				<< '\n';
		}
		std::cout << "\n  Total note records: " << all.size() << '\n';
	}

	std::string prompt_for_input(const std::string& prompt) {
		std::cout << prompt;
		std::string line;
		std::getline(std::cin, line);
		return line;
	}

	std::string prompt_for_password(const std::string& prompt) {
		std::cout << prompt;
		std::string pw;
#ifdef _WIN32
		while (true) {
			int ch = _getch();
			if (ch == '\r' || ch == '\n') {
				if (ch == '\r' && _kbhit()) {
					(void)_getch();
				}
				break;
			}
			if (ch == '\b' || ch == 127) {
				if (!pw.empty()) {
					pw.pop_back();
					std::cout << "\b \b";
				}
			}
			else if (ch >= 32 && ch <= 126) {
				pw.push_back(static_cast<char>(ch));
				std::cout << '*';
			}
		}
		FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
#else
		struct termios orig {};
		struct termios hidden {};
		tcgetattr(STDIN_FILENO, &orig);
		hidden = orig;
		hidden.c_lflag &= ~static_cast<tcflag_t>(ECHO);
		hidden.c_lflag |= ICANON;
		tcsetattr(STDIN_FILENO, TCSANOW, &hidden);
		std::getline(std::cin, pw);
		tcsetattr(STDIN_FILENO, TCSANOW, &orig);
#endif
		std::cout << '\n';
		return pw;
	}

	void wait_for_enter() {
		std::cout << "\nPress Enter to continue...";
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	int show_startup_menu() {
		std::cout
			<< "  What would you like to do?\n\n"
			<< "    [C]reate a new password database\n"
			<< "    [O]pen an existing database\n"
			<< "    [Q]uit\n\n";
		const auto choice = prompt_for_input("  Your choice: ");
		if (choice.empty()) {
			return -1;
		}
		const char key = static_cast<char>(
			std::tolower(static_cast<unsigned char>(choice[0])));
		if (key == 'c') {
			return 1;
		}
		if (key == 'o') {
			return 2;
		}
		if (key == 'q') {
			return 0;
		}
		return -1;
	}

	int show_main_menu() {
		std::cout
			<< "\n  --- Main Menu ---\n\n"
			<< "    [L]ist records\n"
			<< "    [A]dd a new record\n"
			<< "    [R]emove a record\n"
			<< "    [S]ave database\n"
			<< "    Save and [E]xit\n"
			<< "    [Q]uit without saving\n\n";
		const auto choice = prompt_for_input("  Your choice: ");
		if (choice.empty()) {
			return -1;
		}
		const char key = static_cast<char>(
			std::tolower(static_cast<unsigned char>(choice[0])));
		if (key == 'l') {
			return 1;
		}
		if (key == 'a') {
			return 2;
		}
		if (key == 'r') {
			return 3;
		}
		if (key == 's') {
			return 4;
		}
		if (key == 'e') {
			return 5;
		}
		if (key == 'q') {
			return 0;
		}
		return -1;
	}

	RecordKind prompt_record_kind_for_add() {
		std::cout << "\n  What type of record would you like to add?\n\n"
			<< "    [P]assword\n"
			<< "    [N]ote\n"
			<< "    [Q]uit to main menu\n\n";

		while (true) {
			const auto choice = prompt_for_input("  Your choice: ");
			if (choice.empty()) {
				continue;
			}
			if (!validation::is_single_char_valid_with_cancel(choice, 'p', 'n')) {
				show_error("Invalid option. Please press P, N, or Q.");
				continue;
			}

			const char key = static_cast<char>(
				std::tolower(static_cast<unsigned char>(choice[0])));

			if (key == 'p') {
				return RecordKind::Password;
			}
			if (key == 'n') {
				return RecordKind::Note;
			}
			if (key == 'q') {
				return RecordKind::Cancel;
			}
		}
	}

	RecordKind prompt_record_kind_for_list() {
		std::cout << "\n  Which records would you like to view?\n\n"
			<< "    [P]asswords\n"
			<< "    [N]otes\n"
			<< "    [Q]uit to main menu\n\n";

		while (true) {
			const auto choice = prompt_for_input("  Your choice: ");
			if (choice.empty()) {
				continue;
			}
			if (!validation::is_single_char_valid_with_cancel(choice, 'p', 'n')) {
				show_error("Invalid option. Please press P, N, or Q.");
				continue;
			}

			const char key = static_cast<char>(
				std::tolower(static_cast<unsigned char>(choice[0])));

			if (key == 'p') {
				return RecordKind::Password;
			}
			if (key == 'n') {
				return RecordKind::Note;
			}
			if (key == 'q') {
				return RecordKind::Cancel;
			}
		}
	}

	RecordKind prompt_record_kind_for_remove() {
		std::cout << "\n  Which type of record would you like to remove?\n\n"
			<< "    [P]assword\n"
			<< "    [N]ote\n"
			<< "    [Q]uit to main menu\n\n";

		while (true) {
			const auto choice = prompt_for_input("  Your choice: ");
			if (choice.empty()) {
				continue;
			}
			if (!validation::is_single_char_valid_with_cancel(choice, 'p', 'n')) {
				show_error("Invalid option. Please press P, N, or Q.");
				continue;
			}

			const char key = static_cast<char>(
				std::tolower(static_cast<unsigned char>(choice[0])));

			if (key == 'p') {
				return RecordKind::Password;
			}
			if (key == 'n') {
				return RecordKind::Note;
			}
			if (key == 'q') {
				return RecordKind::Cancel;
			}
		}
	}

	PasswordRecord prompt_for_new_password_record() {
		PasswordRecord rec;
		std::cout << "\n  --- Add New Password Record (* = required) ---\n\n";
		while (true) {
			rec.login = prompt_for_input("  Login*:    ");
			if (validation::is_ascii_field_valid(rec.login,
				validation::kLoginMinLength_Password,
				validation::kLoginMaxLength_Password, false)) {
				break;
			}
			show_error(
				"Login is required and must be " +
				std::to_string(validation::kLoginMinLength_Password) + "-" +
				std::to_string(validation::kLoginMaxLength_Password) +
				" printable ASCII characters.");
		}
		while (true) {
			rec.password = prompt_for_password("  Password*: ");
			if (validation::is_ascii_field_valid(rec.password,
				validation::kPasswordMinLength_Password,
				validation::kPasswordMaxLength_Password, false)) {
				break;
			}
			show_error(
				"Password is required and must be " +
				std::to_string(validation::kPasswordMinLength_Password) + "-" +
				std::to_string(validation::kPasswordMaxLength_Password) +
				" printable ASCII characters.");
		}
		while (true) {
			rec.url = prompt_for_input("  URL:      (optional, leave empty for ---) ");
			if (validation::is_field_empty(rec.url)) {
				rec.url = "";
				break;
			}
			std::string url_with_protocol = validation::ensure_url_protocol(rec.url);
			if (url_with_protocol.size() < validation::kUrlMinLength_Password ||
				url_with_protocol.size() > validation::kUrlMaxLength_Password) {
				show_error(
					"URL length must be between " +
					std::to_string(validation::kUrlMinLength_Password) + " and " +
					std::to_string(validation::kUrlMaxLength_Password) +
					" characters (including protocol).");
				continue;
			}

			if (validation::is_valid_url(url_with_protocol)) {
				rec.url = url_with_protocol;
				break;
			}
			else {
				show_error(
					"URL must be in format: http(s)://domain.extension\n"
					"Domain can contain letters, numbers, and hyphens\n"
					"Extension must be 2-6 letters only\n"
					"Example: http://example.com");
			}
		}
		while (true) {
			rec.note = prompt_for_input("  Note:     (optional, leave empty for ---) ");
			if (validation::is_field_empty(rec.note)) {
				rec.note = "";
				break;
			}
			if (validation::is_ascii_field_valid(rec.note,
				validation::kNoteMinLength_Password,
				validation::kNoteMaxLength_Password, true)) {
				break;
			}
			else {
				show_error(
					"If note is provided, it must be " +
					std::to_string(validation::kNoteMinLength_Password) + "-" +
					std::to_string(validation::kNoteMaxLength_Password) +
					" printable ASCII characters, or leave empty for ---.");
			}
		}

		return rec;
	}

	NoteRecord prompt_for_new_note_record() {
		NoteRecord rec;
		std::cout << "\n  --- Add New Note Record (* = required) ---\n\n";
		while (true) {
			rec.title = prompt_for_input("  Title*:  ");
			if (validation::is_ascii_field_valid(rec.title,
				validation::kTitleMinLength_Note,
				validation::kTitleMaxLength_Note, false)) {
				break;
			}
			show_error(
				"Title is required and must be " +
				std::to_string(validation::kTitleMinLength_Note) + "-" +
				std::to_string(validation::kTitleMaxLength_Note) +
				" printable ASCII characters.");
		}
		while (true) {
			rec.note = prompt_for_input("  Note:    (optional, leave empty for ---) ");
			if (validation::is_field_empty(rec.note)) {
				rec.note = "";
				break;
			}
			if (validation::is_ascii_field_valid(rec.note,
				validation::kNoteMinLength_Note,
				validation::kNoteMaxLength_Note, true)) {
				break;
			}
			else {
				show_error(
					"If note is provided, it must be " +
					std::to_string(validation::kNoteMinLength_Note) + "-" +
					std::to_string(validation::kNoteMaxLength_Note) +
					" printable ASCII characters, or leave empty for ---.");
			}
		}

		return rec;
	}
}