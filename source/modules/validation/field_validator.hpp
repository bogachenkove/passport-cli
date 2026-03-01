#pragma once
#include "../security/secure_string.hpp"
#include <string>
#include <vector>
#include <string_view>

namespace domain::validation {
	std::string ensure_url_protocol(std::string_view url);
	bool is_field_empty(std::string_view value);
	bool is_ascii_field_valid(std::string_view value, std::size_t min_len, std::size_t max_len, bool optional);
	bool is_valid_url(std::string_view url);
	bool is_single_char_valid(std::string_view input, char expected1, char expected2);
	bool is_single_char_valid_with_cancel(std::string_view input, char expected1, char expected2);
	[[nodiscard]] bool is_digits_only(std::string_view value);
	[[nodiscard]] bool is_valid_expiry(std::string_view value);
	[[nodiscard]] bool is_letters_and_spaces(std::string_view value);
	[[nodiscard]] bool is_luhn_valid(std::string_view card_number);
	[[nodiscard]] bool is_mnemonic_valid(const std::vector<security::SecureString>& words, const std::vector<std::string>& wordlist);
}