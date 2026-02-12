#include "validation.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <regex>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/stat.h>
#endif

namespace validation {

/**
 *
 * bool file_exists(const std::string& path) {
 * #ifdef _WIN32
 *		DWORD dwAttrib = GetFileAttributesA(path.c_str());
 *		return (dwAttrib != INVALID_FILE_ATTRIBUTES);
 * #else
 *		struct stat info {};
 *		return stat(path.c_str(), &info) == 0;
 * #endif
 *	}
 *
 **/
	
	bool is_directory(const std::string& path) {
#ifdef _WIN32
		DWORD dwAttrib = GetFileAttributesA(path.c_str());
		return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
			(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
		struct stat info {};
		if (stat(path.c_str(), &info) != 0) {
			return false;
		}
		return S_ISDIR(info.st_mode);
#endif
	}

	bool is_regular_file(const std::string& path) {
#ifdef _WIN32
		DWORD dwAttrib = GetFileAttributesA(path.c_str());
		return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
			!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
		struct stat info {};
		if (stat(path.c_str(), &info) != 0) {
			return false;
		}
		return S_ISREG(info.st_mode);
#endif
	}

	bool is_field_empty(const std::string& value) {
		return value.empty() ||
			std::all_of(value.begin(), value.end(), [](unsigned char c) {
			return std::isspace(c);
				});
	}

	bool is_master_password_length_valid(const std::string& pw) {
		return pw.size() >= kPasswordMinLength_MasterPassword &&
			pw.size() <= kPasswordMaxLength_MasterPassword;
	}

	bool is_ascii_field_valid(const std::string& value,
		std::size_t min_len,
		std::size_t max_len,
		bool optional) {
		if (optional && value.empty()) {
			return true;
		}
		if (!optional && value.empty()) {
			return false;
		}

		if (value.size() < min_len || value.size() > max_len) {
			return false;
		}

		for (unsigned char ch : value) {
			if (ch < 0x20 || ch > 0x7E) {
				return false;
			}
		}

		return true;
	}

	bool is_single_char_valid(const std::string& input,
		char expected1,
		char expected2) {
		if (input.length() != 1) {
			return false;
		}
		char c = std::tolower(static_cast<unsigned char>(input[0]));
		return c == expected1 || c == expected2;
	}

	bool is_single_char_valid_with_cancel(const std::string& input,
		char expected1,
		char expected2) {
		if (input.length() != 1) {
			return false;
		}
		char c = std::tolower(static_cast<unsigned char>(input[0]));
		return c == expected1 || c == expected2 || c == 'q';
	}
	bool is_valid_url(const std::string& url) {
		if (url.empty()) return true;
		try {
			std::regex url_regex("^(https?://)?([a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,6}(/.*)?$");
			return std::regex_match(url, url_regex);
		}
		catch (const std::regex_error&) {
			return false;
		}
	}

	std::string ensure_url_protocol(const std::string& url) {
		if (url.empty()) return url;
		if (url.find("http://") == 0 || url.find("https://") == 0) {
			return url;
		}
		return "http://" + url;
	}
}