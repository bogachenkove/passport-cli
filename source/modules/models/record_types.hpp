#pragma once
#include <array>
#include <cstdint>

namespace domain::models {
	using RecordType = std::array<std::uint8_t, 32>;
	inline constexpr const char* kContextPassword = {"PASSPORT::PASSWORD//DATABASE"};
	inline constexpr const char* kContextNote = {"PASSPORT::NOTE//DATABASE"};
	inline constexpr const char* kContextBankCard = {"PASSPORT::BANKCARD//DATABASE"};
	inline constexpr const char* kContextDiscountCard = {"PASSPORT::DISCOUNTCARD//DATABASE"};
	inline constexpr const char* kContextTransportCard = {"PASSPORT::TRANSPORTCARD//DATABASE"};
}