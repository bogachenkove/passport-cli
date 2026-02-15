#include "application.hpp"
#include "../../interface/interface_terminal.hpp"
#include "../../interface/interface_crypto_service.hpp"
#include "../../interface/interface_database.hpp"
#include "../commands/create_database_cmd.hpp"
#include "../commands/open_database_cmd.hpp"
#include "../commands/list_records_cmd.hpp"
#include "../commands/add_record_cmd.hpp"
#include "../commands/remove_record_cmd.hpp"
#include <cctype>
#include <memory>
#include <cstdlib>

namespace app {
	Application::Application(
		std::shared_ptr<domain::interfaces::ITerminal> terminal,
		std::shared_ptr<domain::interfaces::ICryptoService> crypto,
		std::shared_ptr<domain::interfaces::IDatabase> database) : term_(std::move(terminal)), crypto_(std::move(crypto)), db_(std::move(database)) {
	}
	void Application::run_startup_menu() {
		while (!database_ready_) {
			term_->show_message("What would you like to do?\n");
			term_->show_message("  [C]reate a new database");
			term_->show_message("  [O]pen an existing database");
			term_->show_message("  [Q]uit\n");
			auto choice = term_->prompt_input("  Your choice: ");
			if (choice.empty()) continue;
			char key = std::tolower(static_cast<unsigned char>(choice[0]));
			if (key == 'c') {
				commands::CreateDatabaseCommand cmd(term_, db_, crypto_);
				database_ready_ = cmd.execute(db_path_, master_pw_);
			}
			else if (key == 'o') {
				commands::OpenDatabaseCommand cmd(term_, db_);
				database_ready_ = cmd.execute(db_path_, master_pw_);
			}
			else if (key == 'q') {
				term_->show_message("Goodbye!");
				std::exit(0);
			}
			else {
				term_->show_error("Invalid option. Please press C, O, or Q.");
			}
			if (!database_ready_) term_->show_message("");
		}
	}
	void Application::run_main_menu() {
		bool running = true;
		while (running) {
			term_->show_message("\n--- Main Menu ---\n");
			term_->show_message("  [L]ist records");
			term_->show_message("  [A]dd a new record");
			term_->show_message("  [R]emove a record");
			term_->show_message("  [S]ave database");
			term_->show_message("  Save and [E]xit");
			term_->show_message("  [Q]uit without saving\n");
			auto choice = term_->prompt_input("  Your choice: ");
			if (choice.empty()) continue;
			char key = std::tolower(static_cast<unsigned char>(choice[0]));
			switch (key) {
			case 'l':
				commands::ListRecordsCommand(term_, db_).execute();
				break;
			case 'a':
				commands::AddRecordCommand(term_, db_).execute();
				break;
			case 'r':
				commands::RemoveRecordCommand(term_, db_).execute();
				break;
			case 's':
				if (db_->save_to_file(db_path_, master_pw_))
					term_->show_success("Database saved.");
				else
					term_->show_error("Failed to save database.");
				break;
			case 'e':
				if (db_->save_to_file(db_path_, master_pw_)) {
					term_->show_success("Database saved. Goodbye!");
				}
				else {
					term_->show_error("Failed to save database.");
				}
				running = false;
				break;
			case 'q':
				term_->show_message("Exiting without saving. Goodbye!");
				running = false;
				break;
			default:
				term_->show_error("Invalid option. Please choose from the menu.");
				break;
			}
		}
	}
	int Application::run() {
		try {
			crypto_->initialise();
			term_->clear_screen();
			term_->show_message("");
			term_->show_message("  ======================================");
			term_->show_message("        Secure Password Manager");
			term_->show_message("  ======================================");
			term_->show_message("");
			run_startup_menu();
			run_main_menu();
		}
		catch (const std::exception& e) {
			term_->show_error(std::string("[FATAL] ") + e.what());
			return 1;
		}
		return 0;
	}
}