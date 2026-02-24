#pragma once

#include <cstdint>
#include <string>

namespace domain::models {

	struct TransportCardRecord {
		std::uint64_t date = 0;
		std::string   card_number;
		std::string   barcode;
		std::string   expiry;
		std::string   holder;
		std::string   cvv;
		std::string   note;
	};

}