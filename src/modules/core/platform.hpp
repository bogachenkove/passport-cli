#pragma once

#include <filesystem>
#include <string>
#include <iostream>

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
	inline void clear_screen() {
		#ifdef _WIN32
				std::system("cls");
		#else
				std::system("clear");
		#endif
	}
	inline bool is_directory(const std::string& path) {
		#ifdef _WIN32
				DWORD dwAttrib = GetFileAttributesA(path.c_str());
				return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
					(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
		#else
				struct stat info {};
				if (stat(path.c_str(), &info) != 0)
					return false;
				return S_ISDIR(info.st_mode);
		#endif
	}
	inline bool is_regular_file(const std::string& path) {
		#ifdef _WIN32
				DWORD dwAttrib = GetFileAttributesA(path.c_str());
				return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
					!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
		#else
				struct stat info {};
				if (stat(path.c_str(), &info) != 0)
					return false;
				return S_ISREG(info.st_mode);
		#endif
	}
	inline bool path_is_absolute(const std::string& path) {
		try {
			return fs::path(path).is_absolute();
		}
		catch (...) {
			return false;
		}
	}
	inline std::string absolute_path(const std::string& path) {
		try {
			return fs::absolute(fs::path(path)).string();
		}
		catch (...) {
			return path;
		}
	}
	inline char path_separator() {
		#ifdef _WIN32
				return '\\';
		#else
				return '/';
		#endif
	}
	inline std::string read_password_masked() {
		std::string pw;
		#ifdef _WIN32
				while (true) {
					int ch = _getch();
					if (ch == '\r' || ch == '\n') break;
					if (ch == '\b' || ch == 127) {
						if (!pw.empty()) {
							pw.pop_back();
							std::cout << "\b \b";
						}
					}
					else if (ch >= 32 && ch <= 126) {
						pw.push_back(static_cast<char>(ch));
						std::cout << '*';
					}
				}
				std::cout << '\n';
		#else
				struct termios orig, hidden;
				tcgetattr(STDIN_FILENO, &orig);
				hidden = orig;
				hidden.c_lflag &= ~ECHO;
				hidden.c_lflag |= ICANON;
				tcsetattr(STDIN_FILENO, TCSANOW, &hidden);
				std::getline(std::cin, pw);
				tcsetattr(STDIN_FILENO, TCSANOW, &orig);
				std::cout << '\n';
		#endif
		return pw;
	}
}