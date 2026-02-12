#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace security {
	void write_field(std::vector<uint8_t>& out, const std::string& value);
	void write_field(std::vector<uint8_t>& out, const std::vector<uint8_t>& value);

	[[nodiscard]]
	std::string read_string_field(const std::vector<uint8_t>& buf,
		std::size_t& offset);

	[[nodiscard]]
	std::vector<uint8_t> read_bytes_field(const std::vector<uint8_t>& buf,
		std::size_t& offset);
}