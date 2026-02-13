#pragma once

#include <cstdint>
#include <string>

namespace domain::interfaces {

    class ITerminal {
    public:
        virtual ~ITerminal() = default;

        // Вывод сообщений разных уровней
        virtual void show_message(const std::string& msg) = 0;
        virtual void show_error(const std::string& msg) = 0;
        virtual void show_success(const std::string& msg) = 0;

        // Очистка экрана
        virtual void clear_screen() = 0;

        // Запрос строки от пользователя (видимый ввод)
        [[nodiscard]]
        virtual std::string prompt_input(const std::string& prompt) = 0;

        // Запрос пароля (ввод скрыт)
        [[nodiscard]]
        virtual std::string prompt_password(const std::string& prompt) = 0;

        // Ожидание нажатия Enter
        virtual void wait_for_enter() = 0;

        // Форматирование timestamp в строку "YYYY-MM-DD HH:MM"
        [[nodiscard]]
        virtual std::string format_datetime(std::uint64_t timestamp) const = 0;
    };

} // namespace domain::interfaces