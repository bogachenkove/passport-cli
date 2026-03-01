#include "input_parser.hpp"
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace app::utils {
	std::vector<std::size_t> parse_record_numbers(const std::string& input,
		std::size_t max_records) {
		std::vector<std::size_t> indices;
		std::istringstream iss(input);
		std::string token;
		while (iss >> token) {
			try {
				std::size_t num = std::stoul(token);
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
			catch (...) {
				return {
				};
			}
		}
		std::sort(indices.begin(), indices.end());
		indices.erase(std::unique(indices.begin(), indices.end()), indices.end());
		std::reverse(indices.begin(), indices.end());
		return indices;
	}
}