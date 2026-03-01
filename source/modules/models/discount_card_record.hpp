#pragma once
#include <cstdint>
#include <string>
#include "../security/secure_string.hpp"

namespace domain::models {
	struct DiscountCardRecord {
		std::uint64_t date = 0;
		security::SecureString card_number;
		security::SecureString barcode;
		security::SecureString cvv;
		security::SecureString store_name;
		security::SecureString note;
	};
}