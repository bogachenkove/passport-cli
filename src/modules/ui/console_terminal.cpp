#include "console_terminal.hpp"
#include "../core/platform.hpp"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <limits>

namespace ui {

	void ConsoleTerminal::show_message(const std::string& msg) {
		std::cout << "  " << msg << '\n';
	}

	void ConsoleTerminal::show_error(const std::string& msg) {
		std::cout << "  [!] " << msg << '\n';
	}

	void ConsoleTerminal::show_success(const std::string& msg) {
		std::cout << "  [+] " << msg << '\n';
	}

	void ConsoleTerminal::clear_screen() {
		core::platform::clear_screen();
	}

	std::string ConsoleTerminal::prompt_input(const std::string& prompt) {
		std::cout << prompt;
		std::string line;
		std::getline(std::cin, line);
		return line;
	}

	std::string ConsoleTerminal::prompt_password(const std::string& prompt) {
		std::cout << prompt;
		return core::platform::read_password_masked();
	}

	void ConsoleTerminal::wait_for_enter() {
		std::cout << "\nPress Enter to continue...";
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	std::string ConsoleTerminal::format_datetime(std::uint64_t ts) const {
		if (ts == 0) return "N/A";
		std::time_t t = static_cast<std::time_t>(ts);
		std::tm tm_buf{};
#ifdef _WIN32
		localtime_s(&tm_buf, &t);
#else
		localtime_r(&t, &tm_buf);
#endif
		char buf[17];
		std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &tm_buf);
		return buf;
	}

} // namespace ui