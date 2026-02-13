#pragma once

#include <string>

namespace domain::validation {

	// Проверка на пустоту (строка, содержащая только пробельные символы)
	bool is_field_empty(const std::string& value);

	// Валидация ASCII‑полей (печатные символы 0x20‑0x7E)
	bool is_ascii_field_valid(const std::string& value,
		std::size_t min_len,
		std::size_t max_len,
		bool optional);

	// Валидация URL (с протоколом http/https)
	bool is_valid_url(const std::string& url);

	// Добавление протокола http://, если его нет
	std::string ensure_url_protocol(const std::string& url);

	// Проверка, что строка состоит из одного символа и равна одному из ожидаемых
	bool is_single_char_valid(const std::string& input, char expected1, char expected2);

	// То же, но с разрешённым 'q' для отмены
	bool is_single_char_valid_with_cancel(const std::string& input,
		char expected1,
		char expected2);

} // namespace domain::validation