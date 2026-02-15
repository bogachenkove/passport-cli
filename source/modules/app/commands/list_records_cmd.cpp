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
		term_->show_message("  [B]ank Cards");
		term_->show_message("  [N]otes");
		term_->show_message("  [D]iscount Cards");
		term_->show_message("  [Q]uit to main menu\n");
		while (true) {
			auto choice = term_->prompt_input("  Your choice: ");
			if (choice.empty()) continue;
			char key = std::tolower(static_cast<unsigned char>(choice[0]));
			if (key == 'p') {
				ui::display_password_records(*db_, term_);
				return;
			}
			else if (key == 'b') {
				ui::display_bankcard_records(*db_, term_);
				return;
			}
			else if (key == 'n') {
				ui::display_note_records(*db_, term_);
				return;
			}
			else if (key == 'd') {
				ui::display_discountcard_records(*db_, term_);
				return;
			}
			else if (key == 'q') {
				term_->show_message("Operation cancelled.");
				return;
			}
			else {
				term_->show_error("Invalid option. Please press P, B, N, D, or Q.");
			}
		}
	}
}