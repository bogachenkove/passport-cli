#include "field_validator.hpp"
#include <cctype>
#include <algorithm>
#include <regex>
#include <string>

namespace domain::validation {
	bool is_field_empty(const std::string& value) {
		return value.empty() ||
			std::all_of(value.begin(), value.end(),
				[](unsigned char c) {
					return std::isspace(c);
				}
			);
	}
	bool is_ascii_field_valid(const std::string& value,
		std::size_t min_len,
		std::size_t max_len,
		bool optional) {
		if (optional && value.empty()) return true;
		if (!optional && value.empty()) return false;
		if (value.size() < min_len || value.size() > max_len) return false;
		for (unsigned char ch : value) {
			if (ch < 0x20 || ch > 0x7E) return false;
		}
		return true;
	}
	bool is_valid_url(const std::string& url) {
		if (url.empty()) return true;
		try {
			std::regex url_regex(R"(^(https?://)?([a-zA-Z0-9][-a-zA-Z0-9]*\.)+[a-zA-Z]{2,6}(/[-a-zA-Z0-9._~:/?#[\]@!$&'()*+,;=]*)?$)");
			return std::regex_match(url, url_regex);
		}
		catch (const std::regex_error&) {
			return false;
		}
	}
	std::string ensure_url_protocol(const std::string& url) {
		if (url.empty()) return url;
		if (url.starts_with("http://") || url.starts_with("https://"))
			return url;
		return "http://" + url;
	}
	bool is_single_char_valid(const std::string& input, char expected1, char expected2) {
		if (input.length() != 1) return false;
		char c = std::tolower(static_cast<unsigned char>(input[0]));
		return c == expected1 || c == expected2;
	}
	bool is_single_char_valid_with_cancel(const std::string& input,
		char expected1,
		char expected2) {
		if (input.length() != 1) return false;
		char c = std::tolower(static_cast<unsigned char>(input[0]));
		return c == expected1 || c == expected2 || c == 'q';
	}
	bool is_digits_only(const std::string& value) {
		return !value.empty() && std::all_of(value.begin(), value.end(), ::isdigit);
	}
	bool is_valid_expiry(const std::string& value) {
		if (value.size() != 5) return false;
		if (value[2] != '/') return false;
		if (!std::isdigit(value[0]) || !std::isdigit(value[1]) ||
			!std::isdigit(value[3]) || !std::isdigit(value[4])) return false;
		int month = (value[0] - '0') * 10 + (value[1] - '0');
		return month >= 1 && month <= 12;
	}
	bool is_letters_and_spaces(const std::string& value) {
		return !value.empty() && std::all_of(value.begin(), value.end(), [](unsigned char c) {
			return std::isalpha(c) || std::isspace(c);
			}
		);
	}
}