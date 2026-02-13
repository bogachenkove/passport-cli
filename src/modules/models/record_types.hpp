#pragma once

#include <array>
#include <cstdint>

namespace domain::models {

	// 32‑байтовый идентификатор типа записи (BLAKE2b)
	using RecordType = std::array<std::uint8_t, 32>;

	// Контекстные строки для TypeSystem (чистые данные)
	inline constexpr const char* kContextPassword = { "PASSPORT::PASSWORD//DATABASE" };
	inline constexpr const char* kContextNote = { "PASSPORT::NOTE//DATABASE" };

} // namespace domain::models