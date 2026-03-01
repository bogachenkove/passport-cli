#pragma once
#include "../interface/interface_secure_memory.hpp"

namespace security {
#ifdef _WIN32
	class WindowsSecureMemory final : public ISecureMemory {
	public:
		void* allocate(std::size_t size) override;
		void deallocate(void* ptr, std::size_t size) noexcept override;
		bool lock(void* ptr, std::size_t size) noexcept override;
		void unlock(void* ptr, std::size_t size) noexcept override;
		void memzero(void* ptr, std::size_t size) noexcept override;
		bool memcmp_const(const void* a, const void* b, std::size_t size) noexcept override;
	};
#else
	class UnixSecureMemory final : public ISecureMemory {
	public:
		void* allocate(std::size_t size) override;
		void deallocate(void* ptr, std::size_t size) noexcept override;
		bool lock(void* ptr, std::size_t size) noexcept override;
		void unlock(void* ptr, std::size_t size) noexcept override;
		void memzero(void* ptr, std::size_t size) noexcept override;
		bool memcmp_const(const void* a, const void* b, std::size_t size) noexcept override;
	};
#endif
}