#include "password_generator.hpp"
#include "../../interface/interface_crypto_service.hpp"
#include "../../core/constants.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>
#include <string>

namespace app::utils {
	std::string generate_random_password(domain::interfaces::ICryptoService& crypto, std::size_t min_len, std::size_t max_len) {
		std::size_t len = min_len + crypto.random_uniform(static_cast<std::uint32_t>(max_len - min_len + 1));
		std::vector<char> chars;
		chars.reserve(len);
		auto rand_char = [&](const std::string& charset) -> char {
			return charset[crypto.random_uniform(static_cast<std::uint32_t>(charset.size()))];
			};
		chars.push_back(rand_char(core::constants::kLowercaseChars));
		chars.push_back(rand_char(core::constants::kUppercaseChars));
		chars.push_back(rand_char(core::constants::kDigitChars));
		chars.push_back(rand_char(core::constants::kSpecialChars));
		for (std::size_t i = 4; i < len; ++i) {
			chars.push_back(rand_char(core::constants::kAllowedPasswordChars));
		}
		for (std::size_t i = len; i > 1; --i) {
			std::size_t j = crypto.random_uniform(static_cast<std::uint32_t>(i));
			std::swap(chars[i - 1], chars[j]);
		}
		return std::string(chars.begin(), chars.end());
	}
}