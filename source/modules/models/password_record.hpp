#pragma once
#include <cstdint>
#include <string>
#include "../security/secure_string.hpp"

namespace domain::models {
	struct PasswordRecord {
		std::uint64_t date = 0;
		security::SecureString login;
		security::SecureString password;
		security::SecureString url;
		security::SecureString note;
	};
}