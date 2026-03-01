#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "../security/secure_string.hpp"

namespace filesystem::storage::binary_serializer {
	void write_field(std::vector<std::uint8_t>& out, const std::string& value);
	void write_field(std::vector<std::uint8_t>& out, const std::vector<std::uint8_t>& value);
	void write_field(std::vector<std::uint8_t>& out, const security::SecureString& value);
	[[nodiscard]] std::string read_string_field(const std::vector<std::uint8_t>& buf, std::size_t& offset);
	[[nodiscard]] std::vector<std::uint8_t> read_bytes_field(const std::vector<std::uint8_t>& buf, std::size_t& offset);
	[[nodiscard]] security::SecureString read_secure_string_field(const std::vector<std::uint8_t>& buf, std::size_t& offset);
}