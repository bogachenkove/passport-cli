#pragma once
#include "../security/secure_string.hpp"
#include "../security/secure_buffer.hpp"
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
	inline security::SecureString read_input() {
		constexpr std::size_t MAX_INPUT = 1024;
		security::SecureBuffer<char> buffer(MAX_INPUT);
		char* data = buffer.data();
		std::size_t pos = 0;
#ifdef _WIN32
		while (true) {
			int ch = _getch();
			if (ch == '\r' || ch == '\n') {
				break;
			}
			if (ch == '\b' || ch == 127) {
				if (pos > 0) {
					--pos;
					std::cout << "\b \b";
				}
			}
			else if (ch >= 32 && ch <= 126) {
				if (pos < MAX_INPUT - 1) {
					data[pos++] = static_cast<char>(ch);
					std::cout << static_cast<char>(ch);
				}
			}
		}
		std::cout << '\n';
#else
		struct termios orig, raw;
		tcgetattr(STDIN_FILENO, &orig);
		raw = orig;
		raw.c_lflag &= ~ICANON;
		raw.c_lflag |= ECHO;
		tcsetattr(STDIN_FILENO, TCSANOW, &raw);
		while (true) {
			int ch = getchar();
			if (ch == '\n') {
				break;
			}
			if (ch == 127 || ch == '\b') {
				if (pos > 0) {
					--pos;
					std::cout << "\b \b";
				}
			}
			else if (ch >= 32 && ch <= 126) {
				if (pos < MAX_INPUT - 1) {
					data[pos++] = static_cast<char>(ch);
				}
			}
		}
		std::cout << '\n';
		tcsetattr(STDIN_FILENO, TCSANOW, &orig);
#endif
		return security::SecureString(std::string_view(data, pos));
	}
	inline security::SecureString read_password_masked() {
		constexpr std::size_t MAX_PASSWORD = 256;
		security::SecureBuffer<char> buffer(MAX_PASSWORD);
		char* data = buffer.data();
		std::size_t pos = 0;
#ifdef _WIN32
		while (true) {
			int ch = _getch();
			if (ch == '\r' || ch == '\n') {
				break;
			}
			if (ch == '\b' || ch == 127) {
				if (pos > 0) {
					--pos;
					std::cout << "\b \b";
				}
			}
			else if (ch >= 32 && ch <= 126) {
				if (pos < MAX_PASSWORD - 1) {
					data[pos++] = static_cast<char>(ch);
					std::cout << '*';
				}
			}
		}
		std::cout << '\n';
#else
		struct termios orig, raw;
		tcgetattr(STDIN_FILENO, &orig);
		raw = orig;
		raw.c_lflag &= ~(ECHO | ICANON);
		tcsetattr(STDIN_FILENO, TCSANOW, &raw);
		while (true) {
			int ch = getchar();
			if (ch == '\n' || ch == '\r') {
				break;
			}
			if (ch == 127 || ch == '\b') {
				if (pos > 0) {
					--pos;
					std::cout << "\b \b";
				}
			}
			else if (ch >= 32 && ch <= 126) {
				if (pos < MAX_PASSWORD - 1) {
					data[pos++] = static_cast<char>(ch);
					std::cout << '*';
				}
			}
		}
		std::cout << '\n';
		tcsetattr(STDIN_FILENO, TCSANOW, &orig);
#endif
		return security::SecureString(std::string_view(data, pos));
	}
}