#include "binary_serializer.hpp"
#include "../core/endian.hpp"
#include "../core/errors.hpp"
#include "../security/secure_string.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace filesystem::storage::binary_serializer {
	void write_field(std::vector<std::uint8_t>& out, const std::string& value) {
		core::endian::append_u32_be(out, static_cast<std::uint32_t>(value.size()));
		out.insert(out.end(), value.begin(), value.end());
	}
	void write_field(std::vector<std::uint8_t>& out, const std::vector<std::uint8_t>& value) {
		core::endian::append_u32_be(out, static_cast<std::uint32_t>(value.size()));
		out.insert(out.end(), value.begin(), value.end());
	}
	void write_field(std::vector<std::uint8_t>& out, const security::SecureString& value) {
		std::uint32_t len = static_cast<std::uint32_t>(value.size());
		core::endian::append_u32_be(out, len);
		const char* data = value.c_str();
		out.insert(out.end(), reinterpret_cast<const std::uint8_t*>(data),
			reinterpret_cast<const std::uint8_t*>(data + len));
	}
	std::string read_string_field(const std::vector<std::uint8_t>& buf, std::size_t& offset) {
		if (offset + 4 > buf.size()) {
			throw core::errors::DeserialisationError{
			  "Truncated field: not enough bytes for length prefix."
			};
		}
		std::uint32_t len = core::endian::read_u32_be(buf.data() + offset);
		offset += 4;
		if (offset + len > buf.size()) {
			throw core::errors::DeserialisationError{
			  "Truncated field: declared length " + std::to_string(len) + " exceeds available data."
			};
		}
		std::string result(reinterpret_cast<const char*>(buf.data() + offset), len);
		offset += len;
		return result;
	}
	std::vector<std::uint8_t> read_bytes_field(const std::vector<std::uint8_t>& buf,
		std::size_t& offset) {
		if (offset + 4 > buf.size()) {
			throw core::errors::DeserialisationError{
			  "Truncated field: not enough bytes for length prefix."
			};
		}
		std::uint32_t len = core::endian::read_u32_be(buf.data() + offset);
		offset += 4;
		if (offset + len > buf.size()) {
			throw core::errors::DeserialisationError{
			  "Truncated field: declared length " + std::to_string(len) + " exceeds available data."
			};
		}
		std::vector<std::uint8_t> result(buf.begin() + offset, buf.begin() + offset + len);
		offset += len;
		return result;
	}
	security::SecureString read_secure_string_field(const std::vector<std::uint8_t>& buf, std::size_t& offset) {
		if (offset + 4 > buf.size()) {
			throw core::errors::DeserialisationError{
			  "Truncated field: not enough bytes for length prefix."
			};
		}
		std::uint32_t len = core::endian::read_u32_be(buf.data() + offset);
		offset += 4;
		if (offset + len > buf.size()) {
			throw core::errors::DeserialisationError{
			  "Truncated field: declared length " + std::to_string(len) + " exceeds available data."
			};
		}
		security::SecureString result(std::string_view(
			reinterpret_cast<const char*>(buf.data() + offset), len));
		offset += len;
		return result;
	}
}