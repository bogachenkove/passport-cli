#pragma once

/// @file terminal.h
/// @brief Terminal UI helpers — prompts, menus, record display.

#include "database.h"

#include <cstddef>
#include <string>

namespace terminal {

// ── Constants ────────────────────────────────────────────────────────────────

inline constexpr std::size_t kMinPasswordLength = 12;
inline constexpr std::size_t kMaxPasswordLength = 128;

/// Per-field length limits  { min, max }.
inline constexpr std::size_t kLoginMinLength    =   3;
inline constexpr std::size_t kLoginMaxLength    = 100;
inline constexpr std::size_t kRecordPwMinLength =  12;
inline constexpr std::size_t kRecordPwMaxLength = 128;
inline constexpr std::size_t kUrlMinLength      =   5;
inline constexpr std::size_t kUrlMaxLength      = 1000;
inline constexpr std::size_t kNoteMinLength     =   5;
inline constexpr std::size_t kNoteMaxLength     = 1000;

// ── Screen / output ──────────────────────────────────────────────────────────

void clear_screen();
void show_welcome_banner();
void show_message(const std::string& msg);
void show_error(const std::string& msg);
void show_success(const std::string& msg);
void display_records(const PasswordDatabase& db);

// ── Input ────────────────────────────────────────────────────────────────────

/// Read a line of user input (visible).
[[nodiscard]] std::string prompt_for_input(const std::string& prompt);

/// Read a line of user input with echo disabled.
[[nodiscard]] std::string prompt_for_password(const std::string& prompt);

/// Block until user presses Enter.
void wait_for_enter();

// ── Validation ───────────────────────────────────────────────────────────────

[[nodiscard]] bool is_field_empty(const std::string& value);
[[nodiscard]] bool is_master_password_length_valid(const std::string& pw);

/// Universal ASCII field validator.
/// Returns true when every character is printable ASCII (0x20..0x7E)
/// and the string length is within [min_len, max_len].
[[nodiscard]] bool is_ascii_field_valid(const std::string& value,
                                        std::size_t min_len,
                                        std::size_t max_len);

// ── Menus ────────────────────────────────────────────────────────────────────

/// Show the startup menu and return the user's choice (0 / 1 / 2 / -1).
[[nodiscard]] int show_startup_menu();

/// Show the main menu and return the user's choice (0–5 / -1).
[[nodiscard]] int show_main_menu();

/// Interactively prompt the user for login, password, url, and note.
[[nodiscard]] PasswordRecord prompt_for_new_record();

}  // namespace terminal
