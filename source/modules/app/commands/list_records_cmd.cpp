#include "list_records_cmd.hpp"
#include "../../ui/record_formatter.hpp"
#include "../../interface/interface_terminal.hpp"
#include "../../interface/interface_database.hpp"
#include <memory>
#include <cctype>

namespace app::commands {
	ListRecordsCommand::ListRecordsCommand(
		std::shared_ptr<domain::interfaces::ITerminal> terminal,
		std::shared_ptr<domain::interfaces::IDatabase> database) : term_(std::move(terminal)), db_(std::move(database)) {
	}
	void ListRecordsCommand::execute() {
		if (db_->record_count() == 0) {
			term_->show_message("The database is empty. No records to display.");
			return;
		}
		term_->show_message("\nWhich records would you like to view?");
		term_->show_message("  [P]asswords");
		term_->show_message("  [C]ard");
		term_->show_message("  [H]ash");
		term_->show_message("  [N]otes");
		term_->show_message("  [W]iFi");
		term_->show_message("  [Q]uit to main menu\n");
		while (true) {
			auto choice = term_->prompt_input("  Your choice: ");
			if (choice.empty()) continue;
			char key = std::tolower(static_cast<unsigned char>(choice[0]));
			if (key == 'p') {
				ui::display_password_records(*db_, term_);
				return;
			}
			else if (key == 'm') {
				ui::display_mnemonic_records(*db_, term_);
				return;
			}
			else if (key == 'k') {
				ui::display_key_records(*db_, term_);
				return;
			}
			else if (key == 'h') {
				while (true) {
					term_->show_message("\nSelect hash type to remove:");
					term_->show_message("  [M]nemonic");
					term_->show_message("  [K]ey");
					term_->show_message("  [Q]uit to previous menu\n");
					auto hash_choice = term_->prompt_input("  Your choice: ");
					if (hash_choice.empty()) continue;
					char hash_key = std::tolower(static_cast<unsigned char>(hash_choice[0]));
					if (hash_key == 'm') {
						ui::display_mnemonic_records(*db_, term_);
						return;
					}
					else if (hash_key == 'k') {
						ui::display_key_records(*db_, term_);
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
				ui::display_note_records(*db_, term_);
				return;
			}
			else if (key == 'w') {
				ui::display_wifi_records(*db_, term_);
				return;
			}
			else if (key == 'c') {
				while (true) {
					term_->show_message("\nSelect card type to display:");
					term_->show_message("  [B]ank Cards");
					term_->show_message("  [D]iscount Cards");
					term_->show_message("  [T]ransport Cards");
					term_->show_message("  [Q]uit to previous menu\n");
					auto card_choice = term_->prompt_input("  Your choice: ");
					if (card_choice.empty()) continue;
					char card_key = std::tolower(static_cast<unsigned char>(card_choice[0]));
					if (card_key == 'b') {
						ui::display_bankcard_records(*db_, term_);
						return;
					}
					else if (card_key == 'd') {
						ui::display_discountcard_records(*db_, term_);
						return;
					}
					else if (card_key == 't') {
						ui::display_transportcard_records(*db_, term_);
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
				term_->show_error("Invalid option. Please press P, C, M, N or Q.");
			}
		}
	}
}