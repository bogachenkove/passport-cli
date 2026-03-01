#include "input_parser.hpp"
#include <algorithm>
#include <charconv>
#include <cctype>
#include <vector>

namespace app::utils {
	std::vector<std::size_t> parse_record_numbers(std::string_view input, std::size_t max_records) {
		std::vector<std::size_t> indices;
		const char* begin = input.data();
		const char* end = input.data() + input.size();
		while (begin < end) {
			while (begin < end && std::isspace(static_cast<unsigned char>(*begin))) {
				++begin;
			}
			if (begin == end) break;
			const char* token_start = begin;
			while (begin < end && !std::isspace(static_cast<unsigned char>(*begin))) {
				++begin;
			}
			const char* token_end = begin;
			std::size_t num;
			auto [ptr, ec] = std::from_chars(token_start, token_end, num);
			if (ec != std::errc()) {
				return {
				};
			}
			if (num == 0) {
				return {
				};
			}
			if (num < 1 || num > max_records) {
				return {
				};
			}
			indices.push_back(num - 1);
		}
		std::sort(indices.begin(), indices.end());
		indices.erase(std::unique(indices.begin(), indices.end()), indices.end());
		std::reverse(indices.begin(), indices.end());
		return indices;
	}
}