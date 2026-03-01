#pragma once
#include <cstdint>
#include <string>
#include "../security/secure_string.hpp"

namespace domain::models {
	struct NoteRecord {
		std::uint64_t date = 0;
		security::SecureString title;
		security::SecureString note;
	};
}