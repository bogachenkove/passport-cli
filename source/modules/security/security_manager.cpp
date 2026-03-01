#include "security_manager.hpp"

namespace security {
	SecurityManager::SecurityManager() {
#ifdef _WIN32
		impl_ = std::make_unique<WindowsSecureMemory>();
#else
		impl_ = std::make_unique<UnixSecureMemory>();
#endif
	}
	SecurityManager& SecurityManager::instance() {
		static SecurityManager instance;
		return instance;
	}
	std::unique_ptr<void, std::function<void(void*)>> SecurityManager::create_secure_buffer(std::size_t size) {
		void* ptr = impl_->allocate(size);
		if (!impl_->lock(ptr, size)) {
		}
		return std::unique_ptr<void, std::function<void(void*)>>(ptr,[this, size](void* p) {
				impl_->unlock(p, size);
				impl_->deallocate(p, size);
			}
		);
	}
}