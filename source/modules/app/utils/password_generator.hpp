#pragma once
#include "../../interface/interface_crypto_service.hpp"
#include <cstddef>
#include <string>

namespace app::utils {
	[[nodiscard]] std::string generate_random_password(domain::interfaces::ICryptoService& crypto, std::size_t min_len, std::size_t max_len);
}