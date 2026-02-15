#pragma once
#include <cstdint>
#include <string>

namespace domain::models {
	struct BankCardRecord {
		std::uint64_t date = 0;
		std::string card_number;
		std::string expiry_date;
		std::string cvv;
		std::string cardholder_name;
		std::string note;
	};
}