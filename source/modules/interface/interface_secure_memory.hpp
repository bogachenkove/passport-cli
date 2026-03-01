#pragma once
#include <cstddef>
#include <cstdint>

namespace security {
	class ISecureMemory {
	public:
		virtual ~ISecureMemory() = default;
		virtual void* allocate(std::size_t size) = 0;
		virtual void deallocate(void* ptr, std::size_t size) noexcept = 0;
		virtual bool lock(void* ptr, std::size_t size) noexcept = 0;
		virtual void unlock(void* ptr, std::size_t size) noexcept = 0;
		virtual void memzero(void* ptr, std::size_t size) noexcept = 0;
		virtual bool memcmp_const(const void* a, const void* b, std::size_t size) noexcept = 0;
	};
}