#pragma once
#include "../../security/secure_string.hpp"
#include "../../interface/interface_crypto_service.hpp"
#include <cstddef>

namespace app::utils {
	[[nodiscard]] security::SecureString generate_random_password(domain::interfaces::ICryptoService& crypto, std::size_t min_len, std::size_t max_len);
}