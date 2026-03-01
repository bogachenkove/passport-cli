#pragma once
#include "secure_buffer.hpp"
#include <string_view>
#include <cstring>

namespace security {
	class SecureString {
	public:
		SecureString() = default;
		explicit SecureString(std::string_view str) {
			assign(str);
		}
		SecureString(const SecureString&) = delete;
		SecureString& operator=(const SecureString&) = delete;
		SecureString(SecureString&&) noexcept = default;
		SecureString& operator=(SecureString&&) noexcept = default;
		void assign(std::string_view str) {
			buf_ = SecureCharBuffer(str.size() + 1);
			std::memcpy(buf_.data(), str.data(), str.size());
			buf_.data()[str.size()] = '\0';
		}
		const char* c_str() const noexcept {
			return buf_.empty() ? "" : buf_.data();
		}
		std::size_t size() const noexcept {
			return buf_.empty() ? 0 : buf_.size() - 1;
		}
		bool empty() const noexcept {
			return size() == 0;
		}
		bool constant_time_equals(const SecureString& other) const noexcept {
			if (size() != other.size()) return false;
			auto& mem = SecurityManager::instance().memory();
			return mem.memcmp_const(c_str(), other.c_str(), size());
		}
		void clear() noexcept {
			buf_.clear();
		}
	private:
		SecureCharBuffer buf_;
	};
}