#pragma once
#include <cstdint>
#include <string>

namespace domain::models {
	struct WiFiRecord {
		std::uint64_t date = 0;
		std::string ssid;
		std::string password;
		std::string security;
		std::string note;
	};
}