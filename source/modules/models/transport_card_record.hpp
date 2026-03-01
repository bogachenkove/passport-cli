#pragma once
#include <cstdint>
#include <string>
#include "../security/secure_string.hpp"

namespace domain::models {
	struct TransportCardRecord {
		std::uint64_t date = 0;
		security::SecureString card_number;
		security::SecureString barcode;
		security::SecureString expiry;
		security::SecureString holder;
		security::SecureString cvv;
		security::SecureString note;
	};
}