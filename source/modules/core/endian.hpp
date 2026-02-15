#pragma once
#include <cstdint>
#include <vector>

namespace core::endian {
	inline void write_u32_be(uint8_t* dst, uint32_t value) noexcept {
		dst[0] = static_cast<uint8_t>((value >> 24) & 0xFF);
		dst[1] = static_cast<uint8_t>((value >> 16) & 0xFF);
		dst[2] = static_cast<uint8_t>((value >> 8) & 0xFF);
		dst[3] = static_cast<uint8_t>((value >> 0) & 0xFF);
	}
	inline uint32_t read_u32_be(const uint8_t* src) noexcept {
		return (static_cast<uint32_t>(src[0]) << 24) |
			(static_cast<uint32_t>(src[1]) << 16) |
			(static_cast<uint32_t>(src[2]) << 8) |
			(static_cast<uint32_t>(src[3]) << 0);
	}
	inline void append_u32_be(std::vector<uint8_t>& out, uint32_t value) {
		uint8_t buf[4];
		write_u32_be(buf, value);
		out.insert(out.end(), buf, buf + 4);
	}
	inline void write_u64_be(uint8_t* dst, uint64_t value) noexcept {
		for (int i = 7; i >= 0; --i) {
			dst[i] = static_cast<uint8_t>(value & 0xFF);
			value >>= 8;
		}
	}
	inline uint64_t read_u64_be(const uint8_t* src) noexcept {
		uint64_t value = 0;
		for (int i = 0; i < 8; ++i) {
			value = (value << 8) | static_cast<uint64_t>(src[i]);
		}
		return value;
	}
	inline void append_u64_be(std::vector<uint8_t>& out, uint64_t value) {
		uint8_t buf[8];
		write_u64_be(buf, value);
		out.insert(out.end(), buf, buf + 8);
	}
}