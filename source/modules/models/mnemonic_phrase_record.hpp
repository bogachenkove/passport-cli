#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace domain::models {
	struct MnemonicPhraseRecord {
		std::uint64_t date = 0;
		std::uint64_t value = 0;
		std::vector<std::string> mnemonic;
		std::string passphrase;
		std::string language;
		std::uint32_t iteration = 2048;
		std::string note;
	};
}