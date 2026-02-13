#include "open_database_cmd.hpp"
#include "../../core/constants.hpp"
#include "../../core/platform.hpp"
#include "../../validation/field_validator.hpp"
#include "../../validation/password_policy.hpp"
#include "../../storage/file_utils.hpp"
#include "../../interface/interface_terminal.hpp"
#include "../../interface/interface_database.hpp"
#include "../../core/errors.hpp"

namespace app::commands {
	OpenDatabaseCommand::OpenDatabaseCommand(
		std::shared_ptr<domain::interfaces::ITerminal> terminal,
		std::shared_ptr<domain::interfaces::IDatabase> database) : term_(std::move(terminal)), db_(std::move(database)) {
	}
	bool OpenDatabaseCommand::execute(std::string& out_db_path,
		std::string& out_master_pw) {
		auto raw = term_->prompt_input("  Database path: ");
		if (domain::validation::is_field_empty(raw)) {
			term_->show_error("File path cannot be empty.");
			return false;
		}
		std::string path = filesystem::storage::normalise_path(raw);
		if (path.empty()) {
			term_->show_error("Invalid file path.");
			return false;
		}
		if (!filesystem::storage::is_regular_file(path)) {
			term_->show_error("Database file does not exist: " + path);
			return false;
		}
		if (!filesystem::storage::check_file_access(path, false)) {
			term_->show_error("Cannot read from \"" + path + "\". Check read permissions.");
			return false;
		}
		term_->show_message("Opening: " + path);
		std::string pw = term_->prompt_password("  Master password: ");
		if (domain::validation::is_field_empty(pw)) {
			term_->show_error("Master password cannot be empty.");
			return false;
		}
		if (!domain::validation::is_master_password_length_valid(pw)) {
			term_->show_error(
				"Master password must be between " +
				std::to_string(core::constants::kPasswordMinLength_MasterPassword) +
				" and " +
				std::to_string(core::constants::kPasswordMaxLength_MasterPassword) +
				" characters.");
			return false;
		}
		try {
			if (!db_->load_from_file(path, pw)) {
				term_->show_error("Failed to open database. File not found or corrupted.");
				return false;
			}
		}
		catch (const core::errors::AeadError&) {
			term_->show_error("Failed to open database. Wrong password or corrupted file.");
			return false;
		}
		catch (const core::errors::DeserialisationError& e) {
			term_->show_error(std::string("Database integrity error: ") + e.what());
			return false;
		}
		catch (const std::exception& e) {
			term_->show_error(std::string("Error: ") + e.what());
			return false;
		}
		term_->show_success(
			"Database loaded. " +
			std::to_string(db_->password_record_count()) + " password record(s), " +
			std::to_string(db_->note_record_count()) + " note record(s) found.");
		out_db_path = path;
		out_master_pw = pw;
		return true;
	}
}