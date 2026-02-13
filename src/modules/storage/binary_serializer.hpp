#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace filesystem::storage::binary_serializer {

	// Запись строки с 4‑байтовым префиксом длины (big‑endian)
	void write_field(std::vector<std::uint8_t>& out, const std::string& value);

	// Запись бинарных данных с префиксом длины
	void write_field(std::vector<std::uint8_t>& out, const std::vector<std::uint8_t>& value);

	// Чтение строки с префиксом длины
	[[nodiscard]]
	std::string read_string_field(const std::vector<std::uint8_t>& buf,
		std::size_t& offset);

	// Чтение бинарного поля с префиксом длины
	[[nodiscard]]
	std::vector<std::uint8_t> read_bytes_field(const std::vector<std::uint8_t>& buf,
		std::size_t& offset);

} // namespace filesystem::storage::binary_serializer