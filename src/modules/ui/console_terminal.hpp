#pragma once

#include "../interface/interface_terminal.hpp"
#include <string>

namespace ui {

    class ConsoleTerminal final : public domain::interfaces::ITerminal {
    public:
        void show_message(const std::string& msg) override;
        void show_error(const std::string& msg) override;
        void show_success(const std::string& msg) override;
        void clear_screen() override;
        [[nodiscard]] std::string prompt_input(const std::string& prompt) override;
        [[nodiscard]] std::string prompt_password(const std::string& prompt) override;
        void wait_for_enter() override;
        [[nodiscard]] std::string format_datetime(std::uint64_t timestamp) const override;
    };

} // namespace ui