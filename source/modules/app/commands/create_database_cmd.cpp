#include "create_database_cmd.hpp"
#include "../../interface/interface_terminal.hpp"
#include "../../interface/interface_database.hpp"
#include "../../interface/interface_crypto_service.hpp"
#include "../../core/constants.hpp"
#include "../../core/errors.hpp"
#include "../../validation/field_validator.hpp"
#include "../../storage/file_utils.hpp"
#include "../../validation/password_policy.hpp"
#include "../../security/secure_string.hpp"
#include "../utils/password_generator.hpp"
#include <cctype>
#include <memory>

namespace app::commands {
	CreateDatabaseCommand::CreateDatabaseCommand(
		std::shared_ptr<domain::interfaces::ITerminal> terminal,
		std::shared_ptr<domain::interfaces::IDatabase> database,
		std::shared_ptr<domain::interfaces::ICryptoService> crypto) : term_(std::move(terminal)), db_(std::move(database)), crypto_(std::move(crypto)) {
	}
	bool CreateDatabaseCommand::prompt_overwrite(const std::string& path) {
		term_->show_message("File already exists: " + path);
		auto resp_secure = term_->prompt_input("  Do you want to overwrite it? (y/N): ");
		if (resp_secure.empty()) return false;
		char resp = std::tolower(static_cast<unsigned char>(resp_secure.c_str()[0]));
		return resp == 'y';
	}
	bool CreateDatabaseCommand::execute(std::string& out_db_path, security::SecureString& out_master_pw) {
		auto raw_secure = term_->prompt_input("  Database path: ");
		if (domain::validation::is_field_empty(raw_secure)) {
			term_->show_error("File path cannot be empty.");
			return false;
		}
		std::string path = filesystem::storage::normalise_db_path(raw_secure, *crypto_);
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
		term_->show_message("\nChoose password generation mode:");
		term_->show_message("  [M]anual - enter password manually");
		term_->show_message("  [A]uto   - generate a strong password automatically");
		char mode = 0;
		while (true) {
			auto mode_choice_secure = term_->prompt_input("  Your choice (M/A): ");
			if (mode_choice_secure.empty()) continue;
			mode = std::tolower(static_cast<unsigned char>(mode_choice_secure.c_str()[0]));
			if (mode == 'm' || mode == 'a') break;
			term_->show_error("Invalid option. Please press M or A.");
		}
		security::SecureString pw;
		if (mode == 'a') {
			pw = app::utils::generate_random_password(
				*crypto_,
				core::constants::kPasswordMinLength_MasterPassword,
				core::constants::kPasswordMaxLength_MasterPassword
			);
			term_->show_message("\nGenerated master password (please write it down):");
			term_->show_message("  " + std::string(pw.c_str(), pw.size()));
			term_->show_message("");
			auto confirm_input = term_->prompt_password("  Please re-enter the generated password to confirm: ");
			if (!pw.constant_time_equals(confirm_input)) {
				term_->show_error("Passwords do not match. Database creation cancelled.");
				return false;
			}
		}
		else {
			auto input = term_->prompt_password("  Set master password (min 12 chars): ");
			pw = std::move(input);
			if (pw.empty()) {
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
			if (!domain::validation::is_password_characters_valid(pw)) {
				term_->show_error(
					"Master password contains invalid characters. Allowed characters:\n"
					"  Lowercase: " + std::string(core::constants::kLowercaseChars) + "\n"
					"  Uppercase: " + std::string(core::constants::kUppercaseChars) + "\n"
					"  Digits: " + std::string(core::constants::kDigitChars) + "\n"
					"  Specials: " + std::string(core::constants::kSpecialChars));
				return false;
			}
			if (!domain::validation::is_master_password_complex(pw)) {
				term_->show_error(
					"Master password must contain at least one lowercase letter, "
					"one uppercase letter, one digit, and one special character. Required characters:\n"
					"  Lowercase: " + std::string(core::constants::kLowercaseChars) + "\n"
					"  Uppercase: " + std::string(core::constants::kUppercaseChars) + "\n"
					"  Digits: " + std::string(core::constants::kDigitChars) + "\n"
					"  Specials: " + std::string(core::constants::kSpecialChars));
				return false;
			}
			auto confirm_input = term_->prompt_password("  Confirm master password: ");
			if (!pw.constant_time_equals(confirm_input)) {
				term_->show_error("Passwords do not match.");
				return false;
			}
		}
		try {
			if (!db_->save_to_file(path, pw)) {
				term_->show_error("Cannot write to \"" + path + "\". Check the path and permissions.");
				return false;
			}
		}
		catch (const core::errors::CryptoError& e) {
			term_->show_error(std::string("Encryption failed: ") + e.what());
			return false;
		}
		term_->show_success("New database created successfully.");
		out_db_path = path;
		out_master_pw = std::move(pw);
		return true;
	}
}