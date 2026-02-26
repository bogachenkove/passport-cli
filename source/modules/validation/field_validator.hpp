#pragma once
#include <string>

namespace domain::validation {
	std::string ensure_url_protocol(const std::string& url);
	bool is_field_empty(const std::string& value);
	bool is_ascii_field_valid(const std::string& value, std::size_t min_len, std::size_t max_len, bool optional);
	bool is_valid_url(const std::string& url);
	bool is_single_char_valid(const std::string& input, char expected1, char expected2);
	bool is_single_char_valid_with_cancel(const std::string& input, char expected1, char expected2);
	[[nodiscard]] bool is_digits_only(const std::string& value);
	[[nodiscard]] bool is_valid_expiry(const std::string& value);
	[[nodiscard]] bool is_letters_and_spaces(const std::string& value);
	[[nodiscard]] bool is_luhn_valid(const std::string& card_number);
}