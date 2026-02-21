#include "add_record_cmd.hpp"
#include "../../validation/field_validator.hpp"
#include "../../core/constants.hpp"
#include <cctype>
#include <chrono>
#include <memory>
#include <ctime>

namespace app::commands {
	AddRecordCommand::AddRecordCommand(
		std::shared_ptr<domain::interfaces::ITerminal> terminal,
		std::shared_ptr<domain::interfaces::IDatabase> database) : term_(std::move(terminal)), db_(std::move(database)) {
	}
	domain::models::PasswordRecord AddRecordCommand::prompt_password_record() {
		domain::models::PasswordRecord rec;
		term_->show_message("\n  --- Add New Password Record (* = required) ---\n");
		while (true) {
			rec.login = term_->prompt_input("  Login*:    ");
			if (domain::validation::is_ascii_field_valid(
				rec.login,
				core::constants::kLoginMinLength_Password,
				core::constants::kLoginMaxLength_Password,
				false)) break;
			term_->show_error(
				"Login is required and must be " +
				std::to_string(core::constants::kLoginMinLength_Password) + "-" +
				std::to_string(core::constants::kLoginMaxLength_Password) +
				" printable ASCII characters.");
		}
		while (true) {
			rec.password = term_->prompt_password("  Password*: ");
			if (domain::validation::is_ascii_field_valid(
				rec.password,
				core::constants::kPasswordMinLength_Password,
				core::constants::kPasswordMaxLength_Password,
				false)) break;
			term_->show_error(
				"Password is required and must be " +
				std::to_string(core::constants::kPasswordMinLength_Password) + "-" +
				std::to_string(core::constants::kPasswordMaxLength_Password) +
				" printable ASCII characters.");
		}
		while (true) {
			rec.url = term_->prompt_input("  URL:      (optional, leave empty for ---) ");
			if (domain::validation::is_field_empty(rec.url)) {
				rec.url.clear();
				break;
			}
			std::string with_proto = domain::validation::ensure_url_protocol(rec.url);
			if (with_proto.size() < core::constants::kUrlMinLength_Password ||
				with_proto.size() > core::constants::kUrlMaxLength_Password) {
				term_->show_error(
					"URL length must be between " +
					std::to_string(core::constants::kUrlMinLength_Password) +
					" and " +
					std::to_string(core::constants::kUrlMaxLength_Password) +
					" characters (including protocol).");
				continue;
			}
			if (domain::validation::is_valid_url(with_proto)) {
				rec.url = with_proto;
				break;
			}
			else {
				term_->show_error(
					"URL must be in format: http(s)://example.com\n"
					"Domain can contain letters, numbers, and hyphens\n"
					"Extension must be 2-6 letters only.");
			}
		}
		while (true) {
			rec.note = term_->prompt_input("  Note:     (optional, leave empty for ---) ");
			if (domain::validation::is_field_empty(rec.note)) {
				rec.note.clear();
				break;
			}
			if (domain::validation::is_ascii_field_valid(
				rec.note,
				core::constants::kNoteMinLength_Password,
				core::constants::kNoteMaxLength_Password,
				true)) break;
			term_->show_error(
				"If note is provided, it must be " +
				std::to_string(core::constants::kNoteMinLength_Password) + "-" +
				std::to_string(core::constants::kNoteMaxLength_Password) +
				" printable ASCII characters, or leave empty.");
		}
		return rec;
	}
	domain::models::NoteRecord AddRecordCommand::prompt_note_record() {
		domain::models::NoteRecord rec;
		term_->show_message("\n  --- Add New Note Record (* = required) ---\n");
		while (true) {
			rec.title = term_->prompt_input("  Title*:  ");
			if (domain::validation::is_ascii_field_valid(
				rec.title,
				core::constants::kTitleMinLength_Note,
				core::constants::kTitleMaxLength_Note,
				false)) break;
			term_->show_error(
				"Title is required and must be " +
				std::to_string(core::constants::kTitleMinLength_Note) + "-" +
				std::to_string(core::constants::kTitleMaxLength_Note) +
				" printable ASCII characters.");
		}
		while (true) {
			rec.note = term_->prompt_input("  Note:    (optional, leave empty for ---) ");
			if (domain::validation::is_field_empty(rec.note)) {
				rec.note.clear();
				break;
			}
			if (domain::validation::is_ascii_field_valid(
				rec.note,
				core::constants::kNoteMinLength_Note,
				core::constants::kNoteMaxLength_Note,
				true)) break;
			term_->show_error(
				"If note is provided, it must be " +
				std::to_string(core::constants::kNoteMinLength_Note) + "-" +
				std::to_string(core::constants::kNoteMaxLength_Note) +
				" printable ASCII characters, or leave empty.");
		}
		return rec;
	}
	domain::models::BankCardRecord AddRecordCommand::prompt_bankcard_record() {
		domain::models::BankCardRecord rec;
		term_->show_message("\n  --- Add New Bank Card Record (* = required) ---\n");
		while (true) {
			rec.card_number = term_->prompt_input("  Card Number* (digits only, 13-19): ");
			if (domain::validation::is_field_empty(rec.card_number)) {
				term_->show_error("Card number cannot be empty.");
				continue;
			}
			if (rec.card_number.size() < core::constants::kCardNumberMinLength_BankCard ||
				rec.card_number.size() > core::constants::kCardNumberMaxLength_BankCard) {
				term_->show_error(
					"Card number must be between " +
					std::to_string(core::constants::kCardNumberMinLength_BankCard) + " and " +
					std::to_string(core::constants::kCardNumberMaxLength_BankCard) + " digits.");
				continue;
			}
			if (!domain::validation::is_digits_only(rec.card_number)) {
				term_->show_error("Card number must contain only digits.");
				continue;
			}
			break;
		}
		while (true) {
			std::string month_str, year_str;
			int month, year;
			while (true) {
				month_str = term_->prompt_input("  Expiry Month* (MM): ");
				if (domain::validation::is_field_empty(month_str)) {
					term_->show_error("Month cannot be empty.");
					continue;
				}
				if (month_str.size() != 2 || !domain::validation::is_digits_only(month_str)) {
					term_->show_error("Month must be two digits (e.g., 12).");
					continue;
				}
				month = std::stoi(month_str);
				if (month < 1 || month > 12) {
					term_->show_error("Month must be between 01 and 12.");
					continue;
				}
				break;
			}
			auto now = std::chrono::system_clock::now();
			std::time_t t = std::chrono::system_clock::to_time_t(now);
			std::tm tm;
#ifdef _WIN32
			localtime_s(&tm, &t);
#else
			localtime_r(&t, &tm);
#endif
			int current_year = (tm.tm_year + 1900) % 100;
			int max_year = current_year + 10;
			while (true) {
				year_str = term_->prompt_input("  Expiry Year* (YY, current year to +10): ");
				if (domain::validation::is_field_empty(year_str)) {
					term_->show_error("Year cannot be empty.");
					continue;
				}
				if (year_str.size() != 2 || !domain::validation::is_digits_only(year_str)) {
					term_->show_error("Year must be two digits (e.g., 25).");
					continue;
				}
				year = std::stoi(year_str);
				if (year < current_year || year > max_year) {
					term_->show_error("Year must be between " +
						std::to_string(current_year) + " and " +
						std::to_string(max_year) + ".");
					continue;
				}
				break;
			}
			rec.expiry_date = month_str + "/" + year_str;
			if (!domain::validation::is_valid_expiry(rec.expiry_date)) {
				term_->show_error("Internal error: generated expiry is invalid. Please try again.");
				continue;
			}
			break;
		}
		while (true) {
			rec.cvv = term_->prompt_input("  CVV* (3-4 digits): ");
			if (domain::validation::is_field_empty(rec.cvv)) {
				term_->show_error("CVV cannot be empty.");
				continue;
			}
			if (rec.cvv.size() < core::constants::kCVVMinLength_BankCard ||
				rec.cvv.size() > core::constants::kCVVMaxLength_BankCard) {
				term_->show_error(
					"CVV must be between " +
					std::to_string(core::constants::kCVVMinLength_BankCard) + " and " +
					std::to_string(core::constants::kCVVMaxLength_BankCard) + " digits.");
				continue;
			}
			if (!domain::validation::is_digits_only(rec.cvv)) {
				term_->show_error("CVV must contain only digits.");
				continue;
			}
			break;
		}
		while (true) {
			rec.cardholder_name = term_->prompt_input("  Cardholder Name* (letters and spaces, 4-30): ");
			if (domain::validation::is_field_empty(rec.cardholder_name)) {
				term_->show_error("Cardholder name cannot be empty.");
				continue;
			}
			if (rec.cardholder_name.size() < core::constants::kCardHolderNameMinLength_BankCard ||
				rec.cardholder_name.size() > core::constants::kCardHolderNameMaxLength_BankCard) {
				term_->show_error(
					"Cardholder name must be between " +
					std::to_string(core::constants::kCardHolderNameMinLength_BankCard) + " and " +
					std::to_string(core::constants::kCardHolderNameMaxLength_BankCard) + " characters.");
				continue;
			}
			if (!domain::validation::is_letters_and_spaces(rec.cardholder_name)) {
				term_->show_error("Cardholder name can contain only letters and spaces.");
				continue;
			}
			break;
		}
		while (true) {
			rec.note = term_->prompt_input("  Note: (optional, 5-30 ASCII, leave empty for ---) ");
			if (domain::validation::is_field_empty(rec.note)) {
				rec.note.clear();
				break;
			}
			if (rec.note.size() < core::constants::kNoteMinLength_BankCard ||
				rec.note.size() > core::constants::kNoteMaxLength_BankCard) {
				term_->show_error(
					"Note must be between " +
					std::to_string(core::constants::kNoteMinLength_BankCard) + " and " +
					std::to_string(core::constants::kNoteMaxLength_BankCard) + " characters.");
				continue;
			}
			if (!domain::validation::is_ascii_field_valid(rec.note,
				core::constants::kNoteMinLength_BankCard,
				core::constants::kNoteMaxLength_BankCard, true)) {
				term_->show_error("Note must contain only printable ASCII characters.");
				continue;
			}
			break;
		}
		return rec;
	}
	domain::models::DiscountCardRecord AddRecordCommand::prompt_discountcard_record() {
		domain::models::DiscountCardRecord rec;
		term_->show_message("\n  --- Add New Discount Card Record (* = required) ---\n");
		while (true) {
			rec.card_number = term_->prompt_input("  Card Number* (digits only, 8-16): ");
			if (domain::validation::is_field_empty(rec.card_number)) {
				term_->show_error("Card number cannot be empty.");
				continue;
			}
			if (rec.card_number.size() < core::constants::kCardNumberMinLength_DiscountCard ||
				rec.card_number.size() > core::constants::kCardNumberMaxLength_DiscountCard) {
				term_->show_error(
					"Card number must be between " +
					std::to_string(core::constants::kCardNumberMinLength_DiscountCard) + " and " +
					std::to_string(core::constants::kCardNumberMaxLength_DiscountCard) + " digits.");
				continue;
			}
			if (!domain::validation::is_digits_only(rec.card_number)) {
				term_->show_error("Card number must contain only digits.");
				continue;
			}
			break;
		}
		while (true) {
			rec.barcode = term_->prompt_input("  Barcode* (digits only, 6-13): ");
			if (domain::validation::is_field_empty(rec.barcode)) {
				term_->show_error("Barcode cannot be empty.");
				continue;
			}
			if (rec.barcode.size() < core::constants::kBarCodeMinLength_DiscountCard ||
				rec.barcode.size() > core::constants::kBarCodeMaxLength_DiscountCard) {
				term_->show_error(
					"Barcode must be between " +
					std::to_string(core::constants::kBarCodeMinLength_DiscountCard) + " and " +
					std::to_string(core::constants::kBarCodeMaxLength_DiscountCard) + " digits.");
				continue;
			}
			if (!domain::validation::is_digits_only(rec.barcode)) {
				term_->show_error("Barcode must contain only digits.");
				continue;
			}
			break;
		}
		while (true) {
			rec.cvv = term_->prompt_input("  CVV (optional, 3-4 digits, leave empty for ---): ");
			if (domain::validation::is_field_empty(rec.cvv)) {
				rec.cvv.clear();
				break;
			}
			if (rec.cvv.size() < core::constants::kCVVMinLength_DiscountCard ||
				rec.cvv.size() > core::constants::kCVVMaxLength_DiscountCard) {
				term_->show_error(
					"CVV must be between " +
					std::to_string(core::constants::kCVVMinLength_DiscountCard) + " and " +
					std::to_string(core::constants::kCVVMaxLength_DiscountCard) + " digits.");
				continue;
			}
			if (!domain::validation::is_digits_only(rec.cvv)) {
				term_->show_error("CVV must contain only digits.");
				continue;
			}
			break;
		}
		while (true) {
			rec.store_name = term_->prompt_input("  Store Name* (4-30 ASCII characters): ");
			if (domain::validation::is_field_empty(rec.store_name)) {
				term_->show_error("Store name cannot be empty.");
				continue;
			}
			if (!domain::validation::is_ascii_field_valid(rec.store_name,
				core::constants::kStoreNameMinLength_DiscountCard,
				core::constants::kStoreNameMaxLength_DiscountCard, false)) {
				term_->show_error(
					"Store name must be between " +
					std::to_string(core::constants::kStoreNameMinLength_DiscountCard) + " and " +
					std::to_string(core::constants::kStoreNameMaxLength_DiscountCard) +
					" printable ASCII characters.");
				continue;
			}
			break;
		}
		while (true) {
			rec.note = term_->prompt_input("  Note (optional, 5-30 ASCII, leave empty for ---): ");
			if (domain::validation::is_field_empty(rec.note)) {
				rec.note.clear();
				break;
			}
			if (!domain::validation::is_ascii_field_valid(rec.note,
				core::constants::kNoteMinLength_DiscountCard,
				core::constants::kNoteMaxLength_DiscountCard, true)) {
				term_->show_error(
					"If note is provided, it must be between " +
					std::to_string(core::constants::kNoteMinLength_DiscountCard) + " and " +
					std::to_string(core::constants::kNoteMaxLength_DiscountCard) +
					" printable ASCII characters.");
				continue;
			}
			break;
		}
		return rec;
	}
	domain::models::TransportCardRecord AddRecordCommand::prompt_transportcard_record() {
		domain::models::TransportCardRecord rec;
		term_->show_message("\n  --- Add New Transport Card Record (* = required) ---\n");
		while (true) {
			rec.card_number = term_->prompt_input("  Card Number* (digits only, 8-16): ");
			if (domain::validation::is_field_empty(rec.card_number)) {
				term_->show_error("Card number cannot be empty.");
				continue;
			}
			if (rec.card_number.size() < core::constants::kCardNumberMinLength_TransportCard ||
				rec.card_number.size() > core::constants::kCardNumberMaxLength_TransportCard) {
				term_->show_error(
					"Card number must be between " +
					std::to_string(core::constants::kCardNumberMinLength_TransportCard) + " and " +
					std::to_string(core::constants::kCardNumberMaxLength_TransportCard) + " digits.");
				continue;
			}
			if (!domain::validation::is_digits_only(rec.card_number)) {
				term_->show_error("Card number must contain only digits.");
				continue;
			}
			break;
		}
		while (true) {
			rec.barcode = term_->prompt_input("  Barcode* (digits only, 6-13): ");
			if (domain::validation::is_field_empty(rec.barcode)) {
				term_->show_error("Barcode cannot be empty.");
				continue;
			}
			if (rec.barcode.size() < core::constants::kBarCodeMinLength_TransportCard ||
				rec.barcode.size() > core::constants::kBarCodeMaxLength_TransportCard) {
				term_->show_error(
					"Barcode must be between " +
					std::to_string(core::constants::kBarCodeMinLength_TransportCard) + " and " +
					std::to_string(core::constants::kBarCodeMaxLength_TransportCard) + " digits.");
				continue;
			}
			if (!domain::validation::is_digits_only(rec.barcode)) {
				term_->show_error("Barcode must contain only digits.");
				continue;
			}
			break;
		}
		while (true) {
			std::string month_str;
			month_str = term_->prompt_input("  Expiry Month (optional, MM, leave empty to skip): ");
			if (domain::validation::is_field_empty(month_str)) {
				rec.expiry.clear();
				break;
			}
			if (month_str.size() != 2 || !domain::validation::is_digits_only(month_str)) {
				term_->show_error("Month must be two digits (e.g., 12).");
				continue;
			}
			int month = std::stoi(month_str);
			if (month < 1 || month > 12) {
				term_->show_error("Month must be between 01 and 12.");
				continue;
			}
			auto now = std::chrono::system_clock::now();
			std::time_t t = std::chrono::system_clock::to_time_t(now);
			std::tm tm;
#ifdef _WIN32
			localtime_s(&tm, &t);
#else
			localtime_r(&t, &tm);
#endif
			int current_year = (tm.tm_year + 1900) % 100;
			int max_year = current_year + 10;
			std::string year_str;
			year_str = term_->prompt_input("  Expiry Year (YY, current year to +10): ");
			if (domain::validation::is_field_empty(year_str)) {
				term_->show_error("Year cannot be empty if month is entered.");
				continue;
			}
			if (year_str.size() != 2 || !domain::validation::is_digits_only(year_str)) {
				term_->show_error("Year must be two digits (e.g., 25).");
				continue;
			}
			int year = std::stoi(year_str);
			if (year < current_year || year > max_year) {
				term_->show_error("Year must be between " +
					std::to_string(current_year) + " and " +
					std::to_string(max_year) + ".");
				continue;
			}
			rec.expiry = month_str + "/" + year_str;
			if (!domain::validation::is_valid_expiry(rec.expiry)) {
				term_->show_error("Internal error: generated expiry is invalid. Please try again.");
				continue;
			}
			break;
		}
		while (true) {
			rec.holder = term_->prompt_input("  Holder (optional, 4-30 ASCII, leave empty for ---): ");
			if (domain::validation::is_field_empty(rec.holder)) {
				rec.holder.clear();
				break;
			}
			if (!domain::validation::is_ascii_field_valid(rec.holder,
				core::constants::kHolderMinLength_TransportCard,
				core::constants::kHolderMaxLength_TransportCard, true)) {
				term_->show_error(
					"Holder must be between " +
					std::to_string(core::constants::kHolderMinLength_TransportCard) + " and " +
					std::to_string(core::constants::kHolderMaxLength_TransportCard) +
					" printable ASCII characters.");
				continue;
			}
			break;
		}
		while (true) {
			rec.cvv = term_->prompt_input("  CVV (optional, 3-4 digits, leave empty for ---): ");
			if (domain::validation::is_field_empty(rec.cvv)) {
				rec.cvv.clear();
				break;
			}
			if (rec.cvv.size() < core::constants::kCVVMinLength_TransportCard ||
				rec.cvv.size() > core::constants::kCVVMaxLength_TransportCard) {
				term_->show_error(
					"CVV must be between " +
					std::to_string(core::constants::kCVVMinLength_TransportCard) + " and " +
					std::to_string(core::constants::kCVVMaxLength_TransportCard) + " digits.");
				continue;
			}
			if (!domain::validation::is_digits_only(rec.cvv)) {
				term_->show_error("CVV must contain only digits.");
				continue;
			}
			break;
		}
		while (true) {
			rec.note = term_->prompt_input("  Note (optional, 5-30 ASCII, leave empty for ---): ");
			if (domain::validation::is_field_empty(rec.note)) {
				rec.note.clear();
				break;
			}
			if (!domain::validation::is_ascii_field_valid(rec.note,
				core::constants::kNoteMinLength_TransportCard,
				core::constants::kNoteMaxLength_TransportCard, true)) {
				term_->show_error(
					"Note must be between " +
					std::to_string(core::constants::kNoteMinLength_TransportCard) + " and " +
					std::to_string(core::constants::kNoteMaxLength_TransportCard) +
					" printable ASCII characters.");
				continue;
			}
			break;
		}
		return rec;
	}
	void AddRecordCommand::execute() {
		term_->show_message("\nWhat type of record would you like to add?");
		term_->show_message("  [P]assword");
		term_->show_message("  [B]ank Card");
		term_->show_message("  [N]ote");
		term_->show_message("  [D]iscount Card");
		term_->show_message("  [T]ransport Card");
		term_->show_message("  [Q]uit to main menu\n");
		while (true) {
			auto choice = term_->prompt_input("  Your choice: ");
			if (choice.empty()) continue;
			char key = std::tolower(static_cast<unsigned char>(choice[0]));
			if (key == 'p') {
				auto rec = prompt_password_record();
				db_->add_password_record(std::move(rec));
				term_->show_success("Password record added successfully.");
				return;
			}
			else if (key == 'n') {
				auto rec = prompt_note_record();
				db_->add_note_record(std::move(rec));
				term_->show_success("Note record added successfully.");
				return;
			}
			else if (key == 'b') {
				auto rec = prompt_bankcard_record();
				db_->add_bankcard_record(std::move(rec));
				term_->show_success("Bank card record added successfully.");
				return;
			}
			else if (key == 'd') {
				auto rec = prompt_discountcard_record();
				db_->add_discountcard_record(std::move(rec));
				term_->show_success("Discount card record added successfully.");
				return;
			}
			else if (key == 't') {
				auto rec = prompt_transportcard_record();
				db_->add_transportcard_record(std::move(rec));
				term_->show_success("Transport card record added successfully.");
				return;
			}
			else if (key == 'q') {
				term_->show_message("Operation cancelled.");
				return;
			}
			else {
				term_->show_error("Invalid option. Please press P, B, N, D, T, or Q.");
			}
		}
	}
}