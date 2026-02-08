#include "terminal.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#ifdef _WIN32
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <conio.h>
	#include <windows.h>
#else
	#include <termios.h>
	#include <unistd.h>
#endif

namespace terminal {

void clear_screen() {
#ifdef _WIN32
	std::system("cls");
#else
	std::system("clear");
#endif
}

void show_message(const std::string& msg) {
	std::cout << "  " << msg << '\n';
}

void show_error(const std::string& msg) {
	std::cout << "  [!] " << msg << '\n';
}

void show_success(const std::string& msg) {
	std::cout << "  [+] " << msg << '\n';
}

void show_welcome_banner() {
	std::cout
		<< '\n'
		<< "  ======================================\n"
		<< "        Secure Password Manager\n"
		<< "  ======================================\n"
		<< '\n';
}

void display_records(const PassportDatabase& db) {
	const auto& all = db.records();

	if (all.empty()) {
		show_message("\n  The database is empty. No records to display.");
		return;
	}

	std::size_t w_login = 5;   
	std::size_t w_pass  = 8;   
	std::size_t w_url   = 3;   
	std::size_t w_note  = 4;   

	for (const auto& r : all) {
		if (r.login.size()    > w_login) { w_login = r.login.size(); }
		if (r.password.size() > w_pass)  { w_pass  = r.password.size(); }
		if (r.url.size()      > w_url)   { w_url   = r.url.size(); }
		if (r.note.size()     > w_note)  { w_note  = r.note.size(); }
	}

	std::cout << "\n  "
			  << std::left
			  << std::setw(5)             << "#"
			  << std::setw(w_login + 3)   << "Login"
			  << std::setw(w_pass  + 3)   << "Password"
			  << std::setw(w_url   + 3)   << "URL"
			  << std::setw(w_note  + 3)   << "Note"
			  << '\n';

	const auto total_w = 5 + (w_login + 3) + (w_pass + 3)
						   + (w_url + 3) + (w_note + 3);
	std::cout << "  " << std::string(total_w, '-') << '\n';

	for (std::size_t i = 0; i < all.size(); ++i) {
		const auto& r = all[i];
		std::cout << "  "
				  << std::left
				  << std::setw(5)             << (i + 1)
				  << std::setw(w_login + 3)   << r.login
				  << std::setw(w_pass  + 3)   << r.password
				  << std::setw(w_url   + 3)   << r.url
				  << std::setw(w_note  + 3)   << r.note
				  << '\n';
	}

	std::cout << "\n  Total records: " << all.size() << '\n';
}

std::string prompt_for_input(const std::string& prompt) {
	std::cout << prompt;
	std::string line;
	std::getline(std::cin, line);
	return line;
}

std::string prompt_for_password(const std::string& prompt) {
	std::cout << prompt;
	std::string pw;

#ifdef _WIN32
	while (true) {
		int ch = _getch();
		if (ch == '\r' || ch == '\n') {
			if (ch == '\r' && _kbhit()) {
				(void)_getch();
			}
			break;
		}
		if (ch == '\b' || ch == 127) {
			if (!pw.empty()) {
				pw.pop_back();
				std::cout << "\b \b";
			}
		} else if (ch >= 32 && ch <= 126) {
			pw.push_back(static_cast<char>(ch));
			std::cout << '*';
		}
	}
	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
#else
	struct termios orig{};
	struct termios hidden{};

	tcgetattr(STDIN_FILENO, &orig);
	hidden = orig;
	hidden.c_lflag &= ~static_cast<tcflag_t>(ECHO);
	hidden.c_lflag |= ICANON;
	tcsetattr(STDIN_FILENO, TCSANOW, &hidden);

	std::getline(std::cin, pw);

	tcsetattr(STDIN_FILENO, TCSANOW, &orig);
#endif

	std::cout << '\n';
	return pw;
}

void wait_for_enter() {
	std::cout << "\nPress Enter to continue...";
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

bool is_field_empty(const std::string& value) {
	return value.empty()
		|| std::all_of(value.begin(), value.end(),
			   [](unsigned char c) { return std::isspace(c); });
}

bool is_master_password_length_valid(const std::string& pw) {
	return pw.size() >= kMinPasswordLength
		&& pw.size() <= kMaxPasswordLength;
}

bool is_ascii_field_valid(const std::string& value,
						  std::size_t min_len,
						  std::size_t max_len)
{
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

bool file_exists(const std::string& path) {
#ifdef _WIN32
	DWORD dwAttrib = GetFileAttributesA(path.c_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
	struct stat info {};
	return stat(path.c_str(), &info) == 0 && S_ISREG(info.st_mode);
#endif
}

bool prompt_for_overwrite(const std::string& path) {
	std::cout << "\n  File already exists: " << path << "\n";
	std::cout << "  Do you want to overwrite it? (y/N): ";

	std::string response;
	std::getline(std::cin, response);

	if (response.empty()) {
		return false;
	}

	char first_char = std::tolower(static_cast<unsigned char>(response[0]));
	return (first_char == 'y');
}

int show_startup_menu() {
	std::cout
		<< "  What would you like to do?\n\n"
		<< "    [C]reate a new password database\n"
		<< "    [O]pen an existing database\n"
		<< "    [Q]uit\n\n";

	const auto choice = prompt_for_input("  Your choice: ");

	if (choice.empty()) { return -1; }

	const char key = static_cast<char>(
		std::tolower(static_cast<unsigned char>(choice[0])));

	if (key == 'c') { return 1; }
	if (key == 'o') { return 2; }
	if (key == 'q') { return 0; }
	return -1;
}

int show_main_menu() {
	std::cout
		<< "\n  --- Main Menu ---\n\n"
		<< "    [L]ist all records\n"
		<< "    [A]dd a new record\n"
		<< "    [R]emove a record\n"
		<< "    [S]ave database\n"
		<< "    Save and [E]xit\n"
		<< "    [Q]uit without saving\n\n";

	const auto choice = prompt_for_input("  Your choice: ");

	if (choice.empty()) { return -1; }

	const char key = static_cast<char>(
		std::tolower(static_cast<unsigned char>(choice[0])));

	if (key == 'l') { return 1; }
	if (key == 'a') { return 2; }
	if (key == 'r') { return 3; }
	if (key == 's') { return 4; }
	if (key == 'e') { return 5; }
	if (key == 'q') { return 0; }
	return -1;
}

PasswordRecord prompt_for_new_record() {
	PasswordRecord rec;

	std::cout << "\n  --- Add New Record ---\n\n";

	while (true) {
		rec.login = prompt_for_input("  Login:    ");
		if (is_ascii_field_valid(rec.login, kLoginMinLength, kLoginMaxLength)) {
			break;
		}
		show_error(
			"Login must be " + std::to_string(kLoginMinLength) + "-"
			+ std::to_string(kLoginMaxLength)
			+ " printable ASCII characters.");
	}

	while (true) {
		rec.password = prompt_for_password("  Password: ");
		if (is_ascii_field_valid(rec.password, kRecordPwMinLength, kRecordPwMaxLength)) {
			break;
		}
		show_error(
			"Password must be " + std::to_string(kRecordPwMinLength) + "-"
			+ std::to_string(kRecordPwMaxLength)
			+ " printable ASCII characters.");
	}

	while (true) {
		rec.url = prompt_for_input("  URL:      ");
		if (is_ascii_field_valid(rec.url, kUrlMinLength, kUrlMaxLength)) {
			break;
		}
		show_error(
			"URL must be " + std::to_string(kUrlMinLength) + "-"
			+ std::to_string(kUrlMaxLength)
			+ " printable ASCII characters.");
	}

	while (true) {
		rec.note = prompt_for_input("  Note:     ");
		if (is_ascii_field_valid(rec.note, kNoteMinLength, kNoteMaxLength)) {
			break;
		}
		show_error(
			"Note must be " + std::to_string(kNoteMinLength) + "-"
			+ std::to_string(kNoteMaxLength)
			+ " printable ASCII characters.");
	}

	return rec;
}

}