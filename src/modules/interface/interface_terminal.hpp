#pragma once

#include <cstdint>
#include <string>

namespace domain::interfaces {
	class ITerminal {
	public:
		virtual ~ITerminal() = default;
		virtual void show_message(const std::string& msg) = 0;
		virtual void show_error(const std::string& msg) = 0;
		virtual void show_success(const std::string& msg) = 0;
		virtual void clear_screen() = 0;
		[[nodiscard]] virtual std::string prompt_input(const std::string& prompt) = 0;
		[[nodiscard]] virtual std::string prompt_password(const std::string& prompt) = 0;
		virtual void wait_for_enter() = 0;
		[[nodiscard]] virtual std::string format_datetime(std::uint64_t timestamp) const = 0;
	};
}