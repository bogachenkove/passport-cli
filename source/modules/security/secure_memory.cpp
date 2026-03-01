#include "secure_memory.hpp"
#include <sodium.h>
#include <stdexcept>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#endif

namespace security {
#ifdef _WIN32
	void* WindowsSecureMemory::allocate(std::size_t size) {
		void* ptr = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (!ptr) throw std::bad_alloc();
		return ptr;
	}
	void WindowsSecureMemory::deallocate(void* ptr, std::size_t size) noexcept {
		if (!ptr) return;
		memzero(ptr, size);
		VirtualFree(ptr, 0, MEM_RELEASE);
	}
	bool WindowsSecureMemory::lock(void* ptr, std::size_t size) noexcept {
		return VirtualLock(ptr, size) != 0;
	}
	void WindowsSecureMemory::unlock(void* ptr, std::size_t size) noexcept {
		VirtualUnlock(ptr, size);
	}
	void WindowsSecureMemory::memzero(void* ptr, std::size_t size) noexcept {
		if (ptr && size) sodium_memzero(ptr, size);
	}
	bool WindowsSecureMemory::memcmp_const(const void* a, const void* b, std::size_t size) noexcept {
		return sodium_memcmp(a, b, size) == 0;
	}
#else
	void* UnixSecureMemory::allocate(std::size_t size) {
		void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (ptr == MAP_FAILED) throw std::bad_alloc();
		return ptr;
	}
	void UnixSecureMemory::deallocate(void* ptr, std::size_t size) noexcept {
		if (!ptr) return;
		memzero(ptr, size);
		munmap(ptr, size);
	}
	bool UnixSecureMemory::lock(void* ptr, std::size_t size) noexcept {
		return mlock(ptr, size) == 0;
	}
	void UnixSecureMemory::unlock(void* ptr, std::size_t size) noexcept {
		munlock(ptr, size);
	}
	void UnixSecureMemory::memzero(void* ptr, std::size_t size) noexcept {
		if (ptr && size) sodium_memzero(ptr, size);
	}
	bool UnixSecureMemory::memcmp_const(const void* a, const void* b, std::size_t size) noexcept {
		return sodium_memcmp(a, b, size) == 0;
	}
#endif
}