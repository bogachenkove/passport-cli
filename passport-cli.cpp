#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>

#include "crypto.h"
#include "database.h"
#include "terminal.h"

// ── Helpers ──────────────────────────────────────────────────────────────────

namespace {

const std::string kDefaultDbFilename = "passwords.db";

bool is_directory(const std::string& path) {
    struct stat info{};
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
}

std::string normalise_db_path(const std::string& raw) {
    // Trim trailing whitespace
    std::string path = raw;
    while (!path.empty() && (path.back() == ' ' || path.back() == '\t')) {
        path.pop_back();
    }

    if (path.empty()) {
        return path;
    }

    if (is_directory(path)) {
        const char last = path.back();
        const bool has_sep = (last == '/' || last == '\\');
        if (!has_sep) {
#ifdef _WIN32
            path += '\\';
#else
            path += '/';
#endif
        }
        path += kDefaultDbFilename;
    }

    return path;
}

// ── Startup workflows ────────────────────────────────────────────────────────

bool create_new_database(
    PasswordDatabase& db,
    std::string& db_path,
    std::string& master_pw)
{
    const auto raw = terminal::prompt_for_input(
        "  Database file path (file or folder): ");
    if (terminal::is_field_empty(raw)) {
        terminal::show_error("File path cannot be empty.");
        return false;
    }

    db_path = normalise_db_path(raw);
    terminal::show_message("Saving to: " + db_path);

    master_pw = terminal::prompt_for_password(
        "  Set master password (min 12 chars): ");
    if (terminal::is_field_empty(master_pw)) {
        terminal::show_error("Master password cannot be empty.");
        return false;
    }
    if (!terminal::is_master_password_length_valid(master_pw)) {
        terminal::show_error(
            "Master password must be between "
            + std::to_string(terminal::kMinPasswordLength) + " and "
            + std::to_string(terminal::kMaxPasswordLength) + " characters.");
        return false;
    }

    const auto confirm =
        terminal::prompt_for_password("  Confirm master password: ");
    if (master_pw != confirm) {
        terminal::show_error("Passwords do not match.");
        return false;
    }

    try {
        if (!db.save_to_file(db_path, master_pw)) {
            terminal::show_error(
                "Cannot write to \"" + db_path
                + "\". Check the path and permissions.");
            return false;
        }
    } catch (const std::exception& e) {
        terminal::show_error(std::string("Encryption failed: ") + e.what());
        return false;
    }

    terminal::show_success("New database created successfully.");
    return true;
}

bool open_existing_database(
    PasswordDatabase& db,
    std::string& db_path,
    std::string& master_pw)
{
    const auto raw = terminal::prompt_for_input(
        "  Database file path (file or folder): ");
    if (terminal::is_field_empty(raw)) {
        terminal::show_error("File path cannot be empty.");
        return false;
    }

    db_path = normalise_db_path(raw);
    terminal::show_message("Opening: " + db_path);

    master_pw = terminal::prompt_for_password("  Master password: ");
    if (terminal::is_field_empty(master_pw)) {
        terminal::show_error("Master password cannot be empty.");
        return false;
    }
    if (!terminal::is_master_password_length_valid(master_pw)) {
        terminal::show_error(
            "Master password must be between "
            + std::to_string(terminal::kMinPasswordLength) + " and "
            + std::to_string(terminal::kMaxPasswordLength) + " characters.");
        return false;
    }

    try {
        if (!db.load_from_file(db_path, master_pw)) {
            terminal::show_error(
                "Failed to open database. File not found.");
            return false;
        }
    } catch (const crypto::AeadError&) {
        terminal::show_error(
            "Failed to open database. Wrong password or corrupted file.");
        return false;
    } catch (const std::exception& e) {
        terminal::show_error(std::string("Error: ") + e.what());
        return false;
    }

    terminal::show_success(
        "Database loaded. "
        + std::to_string(db.record_count()) + " record(s) found.");
    return true;
}

// ── Record removal ───────────────────────────────────────────────────────────

void handle_remove_record(PasswordDatabase& db) {
    if (db.record_count() == 0) {
        terminal::show_message("The database is empty. Nothing to remove.");
        return;
    }

    terminal::display_records(db);

    const auto input = terminal::prompt_for_input(
        "\n  Enter record number to remove (0 to cancel): ");

    std::size_t idx = 0;
    try {
        idx = std::stoul(input);
    } catch (...) {
        terminal::show_error("Invalid input. Expected a number.");
        return;
    }

    if (idx == 0) {
        terminal::show_message("Removal cancelled.");
        return;
    }

    if (db.remove_record(idx - 1)) {
        terminal::show_success(
            "Record #" + std::to_string(idx) + " removed.");
    } else {
        terminal::show_error("Invalid record number. No changes made.");
    }
}

// ── Main menu loop ───────────────────────────────────────────────────────────

void run_main_menu(
    PasswordDatabase& db,
    const std::string& db_path,
    const std::string& master_pw)
{
    bool running = true;

    while (running) {
        const int choice = terminal::show_main_menu();

        switch (choice) {
        case 1:
            terminal::display_records(db);
            break;

        case 2: {
            auto rec = terminal::prompt_for_new_record();
            db.add_record(rec);
            terminal::show_success("Record added successfully.");
            break;
        }

        case 3:
            handle_remove_record(db);
            break;

        case 4:
            if (db.save_to_file(db_path, master_pw)) {
                terminal::show_success("Database saved.");
            } else {
                terminal::show_error("Failed to save database.");
            }
            break;

        case 5:
            if (db.save_to_file(db_path, master_pw)) {
                terminal::show_success("Database saved. Goodbye!");
            } else {
                terminal::show_error("Failed to save database.");
            }
            running = false;
            break;

        case 0:
            terminal::show_message("Exiting without saving. Goodbye!");
            running = false;
            break;

        default:
            terminal::show_error(
                "Invalid option. Please choose from the menu.");
            break;
        }
    }
}

}  // anonymous namespace

// ── Entry point ──────────────────────────────────────────────────────────────

int main() {
    try {
        crypto::initialise();

        terminal::clear_screen();
        terminal::show_welcome_banner();

        PasswordDatabase db;
        std::string      db_path;
        std::string      master_pw;
        bool             ready = false;

        while (!ready) {
            const int choice = terminal::show_startup_menu();

            switch (choice) {
            case 1:
                ready = create_new_database(db, db_path, master_pw);
                break;
            case 2:
                ready = open_existing_database(db, db_path, master_pw);
                break;
            case 0:
                terminal::show_message("Goodbye!");
                return 0;
            default:
                terminal::show_error(
                    "Invalid option. Please enter 0, 1, or 2.");
                break;
            }

            if (!ready) {
                std::cout << '\n';
            }
        }

        run_main_menu(db, db_path, master_pw);

    } catch (const std::exception& e) {
        std::cerr << "\n  [FATAL] " << e.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "\n  [FATAL] An unknown error occurred.\n";
        return 1;
    }

    return 0;
}
