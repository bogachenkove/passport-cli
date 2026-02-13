#include "remove_record_cmd.hpp"
#include "../../ui/record_formatter.hpp"
#include "../../app/utils/input_parser.hpp"

namespace app::commands {

RemoveRecordCommand::RemoveRecordCommand(
    std::shared_ptr<domain::interfaces::ITerminal> terminal,
    std::shared_ptr<domain::interfaces::IDatabase> database)
    : term_(std::move(terminal))
    , db_(std::move(database))
{}

void RemoveRecordCommand::remove_password_records() {
    if (db_->password_record_count() == 0) {
        term_->show_message("No password records to remove.");
        return;
    }

    ui::display_password_records(*db_, term_);
    auto input = term_->prompt_input(
        "\n  Enter record number(s) to remove (separated by spaces, 0 to cancel): ");

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
    } else {
        term_->show_error("No records were removed.");
    }
}

void RemoveRecordCommand::remove_note_records() {
    if (db_->note_record_count() == 0) {
        term_->show_message("No note records to remove.");
        return;
    }

    ui::display_note_records(*db_, term_);
    auto input = term_->prompt_input(
        "\n  Enter record number(s) to remove (separated by spaces, 0 to cancel): ");

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
    } else {
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
    term_->show_message("  [N]ote");
    term_->show_message("  [Q]uit to main menu\n");

    while (true) {
        auto choice = term_->prompt_input("  Your choice: ");
        if (choice.empty()) continue;
        char key = std::tolower(static_cast<unsigned char>(choice[0]));

        if (key == 'p') {
            remove_password_records();
            return;
        } else if (key == 'n') {
            remove_note_records();
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