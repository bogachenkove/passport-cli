#pragma once
#include "secure_memory.hpp"
#include "../interface/interface_secure_memory.hpp"
#include <memory>
#include <functional>

namespace security {
	class SecurityManager {
	public:
		static SecurityManager& instance();
		ISecureMemory& memory() noexcept {
			return *impl_;
		}
		std::unique_ptr<void, std::function<void(void*)>> create_secure_buffer(std::size_t size);
	private:
		SecurityManager();
		std::unique_ptr<ISecureMemory> impl_;
	};
}