#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "../security/secure_string.hpp"

namespace domain::models {
	struct MnemonicRecord {
		std::uint64_t date = 0;
		std::uint64_t value = 0;
		std::vector<security::SecureString> mnemonic;
		security::SecureString passphrase;
		security::SecureString language;
		std::uint32_t iteration = 2048;
		security::SecureString note;
	};
}