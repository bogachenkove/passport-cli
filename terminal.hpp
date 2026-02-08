#pragma once

#include "database.hpp"

#include <cstddef>
#include <string>

namespace terminal {

	inline constexpr std::size_t kMinPasswordLength = 12;
	inline constexpr std::size_t kMaxPasswordLength = 128;

	inline constexpr std::size_t kLoginMinLength = 3;
	inline constexpr std::size_t kLoginMaxLength = 100;
	inline constexpr std::size_t kRecordPwMinLength = 12;
	inline constexpr std::size_t kRecordPwMaxLength = 128;
	inline constexpr std::size_t kUrlMinLength = 5;
	inline constexpr std::size_t kUrlMaxLength = 1000;
	inline constexpr std::size_t kNoteMinLength = 5;
	inline constexpr std::size_t kNoteMaxLength = 1000;

	void clear_screen();
	void show_welcome_banner();
	void show_message(const std::string& msg);
	void show_error(const std::string& msg);
	void show_success(const std::string& msg);
	void display_records(const PassportDatabase& db);

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
	[[nodiscard]] PasswordRecord prompt_for_new_record();

}