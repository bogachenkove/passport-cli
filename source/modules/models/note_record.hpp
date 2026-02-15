#pragma once
#include <cstdint>
#include <string>

namespace domain::models {
	struct NoteRecord {
		std::uint64_t date = 0;
		std::string title;
		std::string note;
	};
}