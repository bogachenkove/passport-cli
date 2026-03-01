#pragma once
#include <cstdint>
#include <string>
#include "../interface/interface_terminal.hpp"
#include "../security/secure_string.hpp"

namespace ui {
	class ConsoleTerminal final : public domain::interfaces::ITerminal {
	public:
		void show_message(const std::string& msg) override;
		void show_error(const std::string& msg) override;
		void show_success(const std::string& msg) override;
		void clear_screen() override;
		[[nodiscard]] security::SecureString prompt_input(const std::string& prompt) override;
		[[nodiscard]] security::SecureString prompt_password(const std::string& prompt) override;
		void wait_for_enter() override;
		[[nodiscard]] std::string format_datetime(std::uint64_t timestamp) const override;
	};
}