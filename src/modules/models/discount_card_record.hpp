#pragma once
#include <cstdint>
#include <string>

namespace domain::models {
	struct DiscountCardRecord {
		std::uint64_t date = 0;
		std::string card_number;
		std::string barcode;
		std::string cvv;
		std::string store_name;
		std::string note;
	};
}