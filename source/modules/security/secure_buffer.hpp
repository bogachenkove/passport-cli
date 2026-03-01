#pragma once
#include "security_manager.hpp"
#include <cstddef>
#include <stdexcept>
#include <cstring>

namespace security {
	template <typename T>
	class SecureBuffer {
	public:
		explicit SecureBuffer(std::size_t count = 0) : size_(count * sizeof(T)), ptr_(nullptr) {
			if (count == 0) return;
			auto& mem = SecurityManager::instance().memory();
			void* raw = mem.allocate(size_);
			if (!raw) throw std::bad_alloc();
			mem.lock(raw, size_);
			mem.memzero(raw, size_);
			ptr_ = raw;
		}
		~SecureBuffer() {
			reset();
		}
		SecureBuffer(const SecureBuffer&) = delete;
		SecureBuffer& operator=(const SecureBuffer&) = delete;
		SecureBuffer(SecureBuffer&& other) noexcept
			: size_(other.size_), ptr_(other.ptr_) {
			other.size_ = 0;
			other.ptr_ = nullptr;
		}
		SecureBuffer& operator=(SecureBuffer&& other) noexcept {
			if (this != &other) {
				reset();
				size_ = other.size_;
				ptr_ = other.ptr_;
				other.size_ = 0;
				other.ptr_ = nullptr;
			}
			return *this;
		}
		T* data() noexcept {
			return static_cast<T*>(ptr_);
		}
		const T* data() const noexcept {
			return static_cast<const T*>(ptr_);
		}
		std::size_t size() const noexcept {
			return size_;
		}
		std::size_t count() const noexcept {
			return size_ / sizeof(T);
		}
		bool empty() const noexcept {
			return size_ == 0;
		}
		T& operator[](std::size_t idx) {
			if (idx >= count()) throw std::out_of_range("SecureBuffer index out of range");
			return data()[idx];
		}
		const T& operator[](std::size_t idx) const {
			if (idx >= count()) throw std::out_of_range("SecureBuffer index out of range");
			return data()[idx];
		}
		void resize(std::size_t new_count) {
			std::size_t new_size = new_count * sizeof(T);
			if (new_size == size_) return;
			SecureBuffer tmp(new_count);
			if (size_ > 0 && new_size > 0) {
				std::size_t copy_bytes = (size_ < new_size) ? size_ : new_size;
				std::memcpy(tmp.data(), data(), copy_bytes);
			}
			*this = std::move(tmp);
		}
		void clear() noexcept {
			if (ptr_) {
				auto& mem = SecurityManager::instance().memory();
				mem.memzero(ptr_, size_);
			}
		}
	private:
		std::size_t size_;
		void* ptr_;
		void reset() noexcept {
			if (ptr_) {
				auto& mem = SecurityManager::instance().memory();
				mem.unlock(ptr_, size_);
				mem.deallocate(ptr_, size_);
				ptr_ = nullptr;
				size_ = 0;
			}
		}
	};
	using SecureByteBuffer = SecureBuffer<std::uint8_t>;
	using SecureCharBuffer = SecureBuffer<char>;
}