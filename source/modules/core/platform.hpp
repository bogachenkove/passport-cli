#pragma once
#include "../security/secure_string.hpp"
#include "../security/secure_buffer.hpp"
#include "../validation/unicode.hpp"
#include <filesystem>
#include <string>
#include <iostream>
#include <string_view>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

namespace fs = std::filesystem;

namespace core::platform {
	inline void init_console_utf8() {
#ifdef _WIN32
		SetConsoleOutputCP(CP_UTF8);
		SetConsoleCP(CP_UTF8);
#endif
		std::setlocale(LC_ALL, "C.UTF-8");
	}
	inline void clear_screen() {
#ifdef _WIN32
		std::system("cls");
#else
		std::system("clear");
#endif
	}
	inline bool is_directory(std::string_view path) {
#ifdef _WIN32
		std::string tmp(path);
		DWORD dwAttrib = GetFileAttributesA(tmp.c_str());
		return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
			(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
		std::string tmp(path);
		struct stat info {
		};
		if (stat(tmp.c_str(), &info) != 0)
			return false;
		return S_ISDIR(info.st_mode);
#endif
	}
	inline bool is_regular_file(std::string_view path) {
#ifdef _WIN32
		std::string tmp(path);
		DWORD dwAttrib = GetFileAttributesA(tmp.c_str());
		return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
			!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
		std::string tmp(path);
		struct stat info {
		};
		if (stat(tmp.c_str(), &info) != 0)
			return false;
		return S_ISREG(info.st_mode);
#endif
	}
	inline bool path_is_absolute(std::string_view path) {
		try {
			std::string tmp(path);
			return fs::path(tmp).is_absolute();
		}
		catch (...) {
			return false;
		}
	}
	inline std::string absolute_path(std::string_view path) {
		try {
			std::string tmp(path);
			return fs::absolute(fs::path(tmp)).string();
		}
		catch (...) {
			return std::string(path);
		}
	}
	inline char path_separator() {
#ifdef _WIN32
		return '\\';
#else
		return '/';
#endif
	}
	inline bool wide_to_utf8(const std::vector<wchar_t>& wchars, security::SecureBuffer<char>& buffer) {
		if (wchars.empty()) {
			return true;
		}
		UErrorCode status = U_ZERO_ERROR;
		icu::UnicodeString ustr(wchars.data(), static_cast<int32_t>(wchars.size()), status);
		if (U_FAILURE(status)) {
			return false;
		}
		int32_t utf8_len = ustr.extract(0, ustr.length(), nullptr, "utf-8");
		if (utf8_len <= 0) {
			return false;
		}
		if (buffer.size() < static_cast<std::size_t>(utf8_len)) {
			return false;
		}
		char* dest = buffer.data();
		int32_t written = ustr.extract(0, ustr.length(), dest, "utf-8");
		if (written != utf8_len) {
			return false;
		}
		return true;
	}
	inline security::SecureString read_input() {
		std::vector<wchar_t> wchars;
		constexpr std::size_t MAX_INPUT = 1024;
#ifdef _WIN32
		while (true) {
			wint_t ch = _getwch();
			if (ch == L'\r' || ch == L'\n') {
				break;
			}
			if (ch == L'\b' || ch == 127) {
				if (!wchars.empty()) {
					wchars.pop_back();
					std::wcout << L"\b \b";
				}
			}
			else if (ch >= 32 && ch <= 0x10FFFF) {
				if (wchars.size() < MAX_INPUT - 1) {
					wchars.push_back(static_cast<wchar_t>(ch));
					_putwch(ch);
				}
			}
		}
		std::wcout << L'\n';
#else
		while (true) {
			wint_t ch = getwchar();
			if (ch == L'\n') {
				break;
			}
			if (ch == L'\b' || ch == 127) {
				if (!wchars.empty()) {
					wchars.pop_back();
					std::wcout << L"\b \b";
				}
			}
			else if (ch >= 32 && ch <= 0x10FFFF) {
				if (wchars.size() < MAX_INPUT - 1) {
					wchars.push_back(static_cast<wchar_t>(ch));
					putwchar(ch);
				}
			}
		}
		std::wcout << L'\n';
#endif
		if (wchars.empty()) {
			return security::SecureString();
		}
		icu::UnicodeString ustr;
#if defined(_WIN32)
		ustr.setTo(reinterpret_cast<const char16_t*>(wchars.data()), static_cast<int32_t>(wchars.size()));
#else
		ustr.setTo(static_cast<const UChar32*>(wchars.data()), static_cast<int32_t>(wchars.size()));
#endif
		int32_t utf8_len = ustr.extract(0, ustr.length(), nullptr, "utf-8");
		if (utf8_len <= 0) {
			return security::SecureString();
		}
		security::SecureBuffer<char> buffer(utf8_len);
		char* dest = buffer.data();
		int32_t written = ustr.extract(0, ustr.length(), dest, "utf-8");
		if (written != utf8_len) {
			return security::SecureString();
		}
		return security::SecureString(std::string_view(dest, written));
	}
	inline security::SecureString read_password_masked() {
		std::vector<wchar_t> wchars;
		constexpr std::size_t MAX_PASSWORD = 256;
#ifdef _WIN32
		while (true) {
			wint_t ch = _getwch();
			if (ch == L'\r' || ch == L'\n') {
				break;
			}
			if (ch == L'\b' || ch == 127) {
				if (!wchars.empty()) {
					wchars.pop_back();
					std::wcout << L"\b \b";
				}
			}
			else if (ch >= 32 && ch <= 0x10FFFF) {
				if (wchars.size() < MAX_PASSWORD - 1) {
					wchars.push_back(static_cast<wchar_t>(ch));
					std::wcout << L'*';
				}
			}
		}
		std::wcout << L'\n';
#else
		while (true) {
			wint_t ch = getwchar();
			if (ch == L'\n') {
				break;
			}
			if (ch == L'\b' || ch == 127) {
				if (!wchars.empty()) {
					wchars.pop_back();
					std::wcout << L"\b \b";
				}
			}
			else if (ch >= 32 && ch <= 0x10FFFF) {
				if (wchars.size() < MAX_PASSWORD - 1) {
					wchars.push_back(static_cast<wchar_t>(ch));
					std::wcout << L'*';
				}
			}
		}
		std::wcout << L'\n';
#endif
		if (wchars.empty()) {
			return security::SecureString();
		}
		icu::UnicodeString ustr;
#if defined(_WIN32)
		ustr.setTo(reinterpret_cast<const char16_t*>(wchars.data()), static_cast<int32_t>(wchars.size()));
#else
		ustr.setTo(static_cast<const UChar32*>(wchars.data()), static_cast<int32_t>(wchars.size()));
#endif
		int32_t utf8_len = ustr.extract(0, ustr.length(), nullptr, "utf-8");
		if (utf8_len <= 0) {
			return security::SecureString();
		}
		security::SecureBuffer<char> buffer(utf8_len);
		char* dest = buffer.data();
		int32_t written = ustr.extract(0, ustr.length(), dest, "utf-8");
		if (written != utf8_len) {
			return security::SecureString();
		}
		return security::SecureString(std::string_view(dest, written));
	}
}