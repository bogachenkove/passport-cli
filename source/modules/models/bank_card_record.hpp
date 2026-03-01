#pragma once
#include <cstdint>
#include <string>
#include "../security/secure_string.hpp"

namespace domain::models {
	struct BankCardRecord {
		std::uint64_t date = 0;
		security::SecureString card_number;
		security::SecureString expiry_date;
		security::SecureString cvv;
		security::SecureString cardholder_name;
		security::SecureString note;
	};
}