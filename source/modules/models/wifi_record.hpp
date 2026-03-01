#pragma once
#include <cstdint>
#include <string>
#include "../security/secure_string.hpp"

namespace domain::models {
	struct WiFiRecord {
		std::uint64_t date = 0;
		security::SecureString ssid;
		security::SecureString password;
		security::SecureString security;
		security::SecureString note;
	};
}