#pragma once
#include <cstdint>
#include <string>
#include "../security/secure_string.hpp"

namespace domain::models {
	struct KeyRecord {
		std::uint64_t date = 0;
		security::SecureString chain;
		security::SecureString symbol;
		security::SecureString publickey;
		security::SecureString privatekey;
		security::SecureString note;
	};
}