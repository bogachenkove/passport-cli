#pragma once

#include <cstdint>
#include <string>

namespace domain::models {

	struct TransportCardRecord {
		std::uint64_t date = 0;           // автоматически
		std::string   card_number;        // 8-16 цифр (обязательно)
		std::string   barcode;            // 6-13 цифр (обязательно)
		std::string   expiry;             // "MM/YY", опционально
		std::string   holder;             // 4-30 ASCII, опционально
		std::string   cvv;                // 3-4 цифры, опционально
		std::string   note;               // 5-30 ASCII, опционально
	};

} // namespace domain::models