#pragma once
#include <cstdint>
#include <string>

namespace domain::models {
	struct KeyRecord {
		std::uint64_t date = 0;
		std::string chain;
		std::string symbol;
		std::string publickey;
		std::string privatekey;
		std::string note;
	};
}