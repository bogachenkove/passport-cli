#include "create_database_cmd.hpp"
#include "../../core/constants.hpp"
#include "../../validation/field_validator.hpp"
#include "../../storage/file_utils.hpp"
#include "../../interface/interface_terminal.hpp"
#include "../../interface/interface_database.hpp"
#include <cctype>
#include "../../validation/password_policy.hpp"
#include "../../core/errors.hpp"

namespace app::commands {

	CreateDatabaseCommand::CreateDatabaseCommand(
		std::shared_ptr<domain::interfaces::ITerminal> terminal,
		std::shared_ptr<domain::interfaces::IDatabase> database,
		std::shared_ptr<domain::interfaces::ICryptoService> crypto)
		: term_(std::move(terminal))
		, db_(std::move(database))
		, crypto_(std::move(crypto))
	{
	}

bool CreateDatabaseCommand::prompt_overwrite(const std::string& path) {
	term_->show_message("File already exists: " + path);
	auto resp = term_->prompt_input("  Do you want to overwrite it? (y/N): ");
	if (resp.empty()) return false;
	return std::tolower(static_cast<unsigned char>(resp[0])) == 'y';
}

bool CreateDatabaseCommand::execute(std::string& out_db_path,
									std::string& out_master_pw) {
	auto raw = term_->prompt_input("  Database path: ");
	if (domain::validation::is_field_empty(raw)) {
		term_->show_error("File path cannot be empty.");
		return false;
	}

	std::string path = filesystem::storage::normalise_db_path(raw, *crypto_);
	if (path.empty()) {
		term_->show_error("Invalid file path.");
		return false;
	}

	if (filesystem::storage::is_regular_file(path)) {
		if (!prompt_overwrite(path)) {
			term_->show_message("Database creation cancelled.");
			return false;
		}
		term_->show_message("Overwriting existing file...");
	}

	if (!filesystem::storage::check_file_access(path, true)) {
		term_->show_error("Cannot write to \"" + path + "\". Check write permissions.");
		return false;
	}

	term_->show_message("Saving to: " + path);

	std::string pw = term_->prompt_password("  Set master password (min 12 chars): ");
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

	auto confirm = term_->prompt_password("  Confirm master password: ");
	if (pw != confirm) {
		term_->show_error("Passwords do not match.");
		return false;
	}

	try {
		if (!db_->save_to_file(path, pw)) {
			term_->show_error("Cannot write to \"" + path + "\". Check the path and permissions.");
			return false;
		}
	} catch (const core::errors::CryptoError& e) {
		term_->show_error(std::string("Encryption failed: ") + e.what());
		return false;
	}

	term_->show_success("New database created successfully.");
	out_db_path = path;
	out_master_pw = pw;
	return true;
}

} // namespace app::commands