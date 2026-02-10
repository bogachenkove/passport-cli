#pragma once

#include "database.hpp"

#include <cstddef>
#include <string>
#include <variant>

namespace terminal {
    inline constexpr std::size_t kPasswordMinLength_MasterPassword = 12;
    inline constexpr std::size_t kPasswordMaxLength_MasterPassword = 128;
    inline constexpr std::size_t kLoginMinLength_Password = 3;
    inline constexpr std::size_t kLoginMaxLength_Password = 100;
    inline constexpr std::size_t kPasswordMinLength_Password = 8;
    inline constexpr std::size_t kPasswordMaxLength_Password = 128;
    inline constexpr std::size_t kUrlMinLength_Password = 4;
    inline constexpr std::size_t kUrlMaxLength_Password = 100;
    inline constexpr std::size_t kNoteMinLength_Password = 2;
    inline constexpr std::size_t kNoteMaxLength_Password = 100;
    inline constexpr std::size_t kTitleMinLength_Note = 3;
    inline constexpr std::size_t kTitleMaxLength_Note = 12;
    inline constexpr std::size_t kNoteMinLength_Note = 5;
    inline constexpr std::size_t kNoteMaxLength_Note = 500;
    enum class RecordKind { Password, Note };
    using AnyRecord = std::variant<PasswordRecord, NoteRecord>;
    void clear_screen();
    void show_welcome_banner();
    void show_message(const std::string& msg);
    void show_error(const std::string& msg);
    void show_success(const std::string& msg);
    void display_password_records(const PassportDatabase& db);
    void display_note_records(const PassportDatabase& db);
    [[nodiscard]] std::string prompt_for_input(const std::string& prompt);
    [[nodiscard]] std::string prompt_for_password(const std::string& prompt);
    [[nodiscard]] bool prompt_for_overwrite(const std::string& path);
    void wait_for_enter();
    [[nodiscard]] bool is_field_empty(const std::string& value);
    [[nodiscard]] bool is_master_password_length_valid(const std::string& pw);
    [[nodiscard]] bool file_exists(const std::string& path);
    [[nodiscard]] bool is_ascii_field_valid(const std::string& value,
        std::size_t min_len,
        std::size_t max_len);
    [[nodiscard]] int show_startup_menu();
    [[nodiscard]] int show_main_menu();
    [[nodiscard]] RecordKind prompt_record_kind_for_add();
    [[nodiscard]] RecordKind prompt_record_kind_for_list();
    [[nodiscard]] RecordKind prompt_record_kind_for_remove();
    [[nodiscard]] PasswordRecord prompt_for_new_password_record();
    [[nodiscard]] NoteRecord prompt_for_new_note_record();

}