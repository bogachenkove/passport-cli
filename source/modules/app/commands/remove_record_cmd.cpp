#include "remove_record_cmd.hpp"
#include "../../interface/interface_terminal.hpp"
#include "../../interface/interface_database.hpp"
#include "../../ui/record_formatter.hpp"
#include "../../security/secure_string.hpp"
#include "../utils/input_parser.hpp"
#include <memory>
#include <cctype>
#include <cstddef>
#include <string>

namespace app::commands {
	RemoveRecordCommand::RemoveRecordCommand(
		std::shared_ptr<domain::interfaces::ITerminal> terminal,
		std::shared_ptr<domain::interfaces::IDatabase> database) : term_(std::move(terminal)), db_(std::move(database)) {
	}
	void RemoveRecordCommand::remove_password_records() {
		if (db_->password_record_count() == 0) {
			term_->show_message("No password records to remove.");
			return;
		}
		ui::display_password_records(*db_, term_);
		auto input_secure = term_->prompt_input(
			"\n  Enter record number(s) to remove (separated by spaces, 0 to cancel): ");
		std::string input(input_secure.c_str(), input_secure.size());
		auto indices = app::utils::parse_record_numbers(input, db_->password_record_count());
		if (indices.empty()) {
			if (input.find('0') != std::string::npos)
				term_->show_message("Removal cancelled.");
			else
				term_->show_error("No valid records to remove.");
			return;
		}
		int removed = 0;
		for (std::size_t idx : indices) {
			if (db_->remove_password_record(idx))
				++removed;
		}
		if (removed > 0) {
			if (removed == 1)
				term_->show_success("1 password record removed.");
			else
				term_->show_success(std::to_string(removed) + " password records removed.");
		}
		else {
			term_->show_error("No records were removed.");
		}
	}
	void RemoveRecordCommand::remove_note_records() {
		if (db_->note_record_count() == 0) {
			term_->show_message("No note records to remove.");
			return;
		}
		ui::display_note_records(*db_, term_);
		auto input_secure = term_->prompt_input(
			"\n  Enter record number(s) to remove (separated by spaces, 0 to cancel): ");
		std::string input(input_secure.c_str(), input_secure.size());
		auto indices = app::utils::parse_record_numbers(input, db_->note_record_count());
		if (indices.empty()) {
			if (input.find('0') != std::string::npos)
				term_->show_message("Removal cancelled.");
			else
				term_->show_error("No valid records to remove.");
			return;
		}
		int removed = 0;
		for (std::size_t idx : indices) {
			if (db_->remove_note_record(idx))
				++removed;
		}
		if (removed > 0) {
			if (removed == 1)
				term_->show_success("1 note record removed.");
			else
				term_->show_success(std::to_string(removed) + " note records removed.");
		}
		else {
			term_->show_error("No records were removed.");
		}
	}
	void RemoveRecordCommand::remove_bankcard_records() {
		if (db_->bankcard_record_count() == 0) {
			term_->show_message("No bank card records to remove.");
			return;
		}
		ui::display_bankcard_records(*db_, term_);
		auto input_secure = term_->prompt_input(
			"\n  Enter record number(s) to remove (separated by spaces, 0 to cancel): ");
		std::string input(input_secure.c_str(), input_secure.size());
		auto indices = app::utils::parse_record_numbers(input, db_->bankcard_record_count());
		if (indices.empty()) {
			if (input.find('0') != std::string::npos)
				term_->show_message("Removal cancelled.");
			else
				term_->show_error("No valid records to remove.");
			return;
		}
		int removed = 0;
		for (std::size_t idx : indices) {
			if (db_->remove_bankcard_record(idx))
				++removed;
		}
		if (removed > 0) {
			if (removed == 1)
				term_->show_success("1 bank card record removed.");
			else
				term_->show_success(std::to_string(removed) + " bank card records removed.");
		}
		else {
			term_->show_error("No records were removed.");
		}
	}
	void RemoveRecordCommand::remove_discountcard_records() {
		if (db_->discountcard_record_count() == 0) {
			term_->show_message("No discount card records to remove.");
			return;
		}
		ui::display_discountcard_records(*db_, term_);
		auto input_secure = term_->prompt_input(
			"\n  Enter record number(s) to remove (separated by spaces, 0 to cancel): ");
		std::string input(input_secure.c_str(), input_secure.size());
		auto indices = app::utils::parse_record_numbers(input, db_->discountcard_record_count());
		if (indices.empty()) {
			if (input.find('0') != std::string::npos)
				term_->show_message("Removal cancelled.");
			else
				term_->show_error("No valid records to remove.");
			return;
		}
		int removed = 0;
		for (std::size_t idx : indices) {
			if (db_->remove_discountcard_record(idx))
				++removed;
		}
		if (removed > 0) {
			if (removed == 1)
				term_->show_success("1 discount card record removed.");
			else
				term_->show_success(std::to_string(removed) + " discount card records removed.");
		}
		else {
			term_->show_error("No records were removed.");
		}
	}
	void RemoveRecordCommand::remove_transportcard_records() {
		if (db_->transportcard_record_count() == 0) {
			term_->show_message("No transport card records to remove.");
			return;
		}
		ui::display_transportcard_records(*db_, term_);
		auto input_secure = term_->prompt_input(
			"\n  Enter record number(s) to remove (separated by spaces, 0 to cancel): ");
		std::string input(input_secure.c_str(), input_secure.size());
		auto indices = app::utils::parse_record_numbers(input, db_->transportcard_record_count());
		if (indices.empty()) {
			if (input.find('0') != std::string::npos)
				term_->show_message("Removal cancelled.");
			else
				term_->show_error("No valid records to remove.");
			return;
		}
		int removed = 0;
		for (std::size_t idx : indices) {
			if (db_->remove_transportcard_record(idx))
				++removed;
		}
		if (removed > 0) {
			if (removed == 1)
				term_->show_success("1 transport card record removed.");
			else
				term_->show_success(std::to_string(removed) + " transport card records removed.");
		}
		else {
			term_->show_error("No records were removed.");
		}
	}
	void RemoveRecordCommand::remove_mnemonic_records() {
		if (db_->mnemonic_record_count() == 0) {
			term_->show_message("No mnemonic records to remove.");
			return;
		}
		ui::display_mnemonic_records(*db_, term_);
		auto input_secure = term_->prompt_input(
			"\n  Enter record number(s) to remove (separated by spaces, 0 to cancel): ");
		std::string input(input_secure.c_str(), input_secure.size());
		auto indices = app::utils::parse_record_numbers(input, db_->mnemonic_record_count());
		if (indices.empty()) {
			if (input.find('0') != std::string::npos)
				term_->show_message("Removal cancelled.");
			else
				term_->show_error("No valid records to remove.");
			return;
		}
		int removed = 0;
		for (std::size_t idx : indices) {
			if (db_->remove_mnemonic_record(idx))
				++removed;
		}
		if (removed > 0) {
			if (removed == 1)
				term_->show_success("1 mnemonic record removed.");
			else
				term_->show_success(std::to_string(removed) + " mnemonic records removed.");
		}
		else {
			term_->show_error("No records were removed.");
		}
	}
	void RemoveRecordCommand::remove_wifi_records() {
		if (db_->wifi_record_count() == 0) {
			term_->show_message("No Wi-Fi network records to remove.");
			return;
		}
		ui::display_wifi_records(*db_, term_);
		auto input_secure = term_->prompt_input(
			"\n  Enter record number(s) to remove (separated by spaces, 0 to cancel): ");
		std::string input(input_secure.c_str(), input_secure.size());
		auto indices = app::utils::parse_record_numbers(input, db_->wifi_record_count());
		if (indices.empty()) {
			if (input.find('0') != std::string::npos)
				term_->show_message("Removal cancelled.");
			else
				term_->show_error("No valid records to remove.");
			return;
		}
		int removed = 0;
		for (std::size_t idx : indices) {
			if (db_->remove_wifi_record(idx))
				++removed;
		}
		if (removed > 0) {
			term_->show_success(std::to_string(removed) + " Wi-Fi network record(s) removed.");
		}
		else {
			term_->show_error("No records were removed.");
		}
	}
	void RemoveRecordCommand::remove_key_records() {
		if (db_->key_record_count() == 0) {
			term_->show_message("No key records to remove.");
			return;
		}
		ui::display_key_records(*db_, term_);
		auto input_secure = term_->prompt_input(
			"\n  Enter record number(s) to remove (separated by spaces, 0 to cancel): ");
		std::string input(input_secure.c_str(), input_secure.size());
		auto indices = app::utils::parse_record_numbers(input, db_->key_record_count());
		if (indices.empty()) {
			if (input.find('0') != std::string::npos)
				term_->show_message("Removal cancelled.");
			else
				term_->show_error("No valid records to remove.");
			return;
		}
		int removed = 0;
		for (std::size_t idx : indices) {
			if (db_->remove_key_record(idx))
				++removed;
		}
		if (removed > 0) {
			term_->show_success(std::to_string(removed) + " key record(s) removed.");
		}
		else {
			term_->show_error("No records were removed.");
		}
	}
	void RemoveRecordCommand::execute() {
		if (db_->record_count() == 0) {
			term_->show_message("The database is empty. Nothing to remove.");
			return;
		}
		term_->show_message("\nWhich type of record would you like to remove?");
		term_->show_message("  [P]assword");
		term_->show_message("  [C]ard");
		term_->show_message("  [H]ash");
		term_->show_message("  [N]ote");
		term_->show_message("  [W]iFi");
		term_->show_message("  [Q]uit to main menu\n");
		while (true) {
			auto choice_secure = term_->prompt_input("  Your choice: ");
			if (choice_secure.empty()) continue;
			char key = std::tolower(static_cast<unsigned char>(choice_secure.c_str()[0]));
			if (key == 'p') {
				remove_password_records();
				return;
			}
			else if (key == 'h') {
				while (true) {
					term_->show_message("\nSelect hash type to remove:");
					term_->show_message("  [M]nemonic");
					term_->show_message("  [K]ey");
					term_->show_message("  [Q]uit to previous menu\n");
					auto hash_choice_secure = term_->prompt_input("  Your choice: ");
					if (hash_choice_secure.empty()) continue;
					char hash_key = std::tolower(static_cast<unsigned char>(hash_choice_secure.c_str()[0]));
					if (hash_key == 'm') {
						remove_mnemonic_records();
						return;
					}
					else if (hash_key == 'k') {
						remove_key_records();
						return;
					}
					else if (hash_key == 'q') {
						break;
					}
					else {
						term_->show_error("Invalid option. Please press M, K or Q.");
					}
				}
			}
			else if (key == 'n') {
				remove_note_records();
				return;
			}
			else if (key == 'w') {
				remove_wifi_records();
				return;
			}
			else if (key == 'c') {
				while (true) {
					term_->show_message("\nSelect card type to remove:");
					term_->show_message("  [B]ank Card");
					term_->show_message("  [D]iscount Card");
					term_->show_message("  [T]ransport Card");
					term_->show_message("  [Q]uit to previous menu\n");
					auto card_choice_secure = term_->prompt_input("  Your choice: ");
					if (card_choice_secure.empty()) continue;
					char card_key = std::tolower(static_cast<unsigned char>(card_choice_secure.c_str()[0]));
					if (card_key == 'b') {
						remove_bankcard_records();
						return;
					}
					else if (card_key == 'd') {
						remove_discountcard_records();
						return;
					}
					else if (card_key == 't') {
						remove_transportcard_records();
						return;
					}
					else if (card_key == 'q') {
						break;
					}
					else {
						term_->show_error("Invalid option. Please press B, D, T or Q.");
					}
				}
			}
			else if (key == 'q') {
				term_->show_message("Operation cancelled.");
				return;
			}
			else {
				term_->show_error("Invalid option. Please press P, C, H, N, W or Q.");
			}
		}
	}
}