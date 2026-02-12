#pragma once

#include <cstddef>
#include <string>

namespace validation {
	inline constexpr std::size_t kPasswordMinLength_MasterPassword = 12;
	inline constexpr std::size_t kPasswordMaxLength_MasterPassword = 128;
	inline constexpr std::size_t kLoginMinLength_Password = 3;
	inline constexpr std::size_t kLoginMaxLength_Password = 100;
	inline constexpr std::size_t kPasswordMinLength_Password = 8;
	inline constexpr std::size_t kPasswordMaxLength_Password = 128;
	inline constexpr std::size_t kUrlMinLength_Password = 11;
	inline constexpr std::size_t kUrlMaxLength_Password = 100;
	inline constexpr std::size_t kNoteMinLength_Password = 2;
	inline constexpr std::size_t kNoteMaxLength_Password = 100;
	inline constexpr std::size_t kTitleMinLength_Note = 3;
	inline constexpr std::size_t kTitleMaxLength_Note = 12;
	inline constexpr std::size_t kNoteMinLength_Note = 5;
	inline constexpr std::size_t kNoteMaxLength_Note = 500;

	
	bool is_field_empty(const std::string& value);
	bool is_master_password_length_valid(const std::string& pw);
	bool is_ascii_field_valid(const std::string& value,
		std::size_t min_len,
		std::size_t max_len,
		bool optional);

	bool is_single_char_valid(const std::string& input,
		char expected1,
		char expected2);

	bool is_single_char_valid_with_cancel(const std::string& input,
		char expected1,
		char expected2);
	bool is_valid_url(const std::string& url);
	std::string ensure_url_protocol(const std::string& url);
	bool is_directory(const std::string& path);
	bool is_regular_file(const std::string& path);
	/** bool file_exists(const std::string& path); */
}