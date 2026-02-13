#include "add_record_cmd.hpp"
#include "../../core/constants.hpp"
#include "../../validation/field_validator.hpp"
#include <cctype>

namespace app::commands {

AddRecordCommand::AddRecordCommand(
    std::shared_ptr<domain::interfaces::ITerminal> terminal,
    std::shared_ptr<domain::interfaces::IDatabase> database)
    : term_(std::move(terminal))
    , db_(std::move(database))
{}

domain::models::PasswordRecord AddRecordCommand::prompt_password_record() {
    domain::models::PasswordRecord rec;
    term_->show_message("\n  --- Add New Password Record (* = required) ---\n");

    // Login
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

    // Password
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

    // URL
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
        } else {
            term_->show_error(
                "URL must be in format: http(s)://example.com\n"
                "Domain can contain letters, numbers, and hyphens\n"
                "Extension must be 2-6 letters only.");
        }
    }

    // Note
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

    // Title
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

    // Note
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

void AddRecordCommand::execute() {
    term_->show_message("\nWhat type of record would you like to add?");
    term_->show_message("  [P]assword");
    term_->show_message("  [N]ote");
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
        } else if (key == 'n') {
            auto rec = prompt_note_record();
            db_->add_note_record(std::move(rec));
            term_->show_success("Note record added successfully.");
            return;
        } else if (key == 'q') {
            term_->show_message("Operation cancelled.");
            return;
        } else {
            term_->show_error("Invalid option. Please press P, N, or Q.");
        }
    }
}

} // namespace app::commands