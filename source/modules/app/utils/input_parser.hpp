#pragma once
#include <cstddef>
#include <string>
#include <vector>

namespace app::utils {
	[[nodiscard]] std::vector<std::size_t> parse_record_numbers(const std::string& input, std::size_t max_records);
}