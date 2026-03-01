#pragma once
#include <cstddef>
#include <string>
#include <vector>
#include <string_view>

namespace app::utils {
	[[nodiscard]] std::vector<std::size_t> parse_record_numbers(std::string_view input, std::size_t max_records);
}