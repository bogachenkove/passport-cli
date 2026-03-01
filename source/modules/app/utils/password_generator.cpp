#include "password_generator.hpp"
#include "../../interface/interface_crypto_service.hpp"
#include "../../core/constants.hpp"
#include "../../security/secure_string.hpp"
#include <algorithm>
#include <cstdint>
#include <string_view>

namespace app::utils {
	security::SecureString generate_random_password(
		domain::interfaces::ICryptoService& crypto,
		std::size_t min_len,
		std::size_t max_len) {
		std::size_t len = min_len + crypto.random_uniform(
			static_cast<std::uint32_t>(max_len - min_len + 1));
		security::SecureBuffer<char> buffer(len);
		char* chars = buffer.data();
		auto rand_char = [&](const std::string& charset) -> char {
			return charset[crypto.random_uniform(
				static_cast<std::uint32_t>(charset.size()))];
			};
		chars[0] = rand_char(core::constants::kLowercaseChars);
		chars[1] = rand_char(core::constants::kUppercaseChars);
		chars[2] = rand_char(core::constants::kDigitChars);
		chars[3] = rand_char(core::constants::kSpecialChars);
		for (std::size_t i = 4; i < len; ++i) {
			chars[i] = rand_char(core::constants::kAllowedPasswordChars);
		}
		for (std::size_t i = len; i > 1; --i) {
			std::size_t j = crypto.random_uniform(static_cast<std::uint32_t>(i));
			std::swap(chars[i - 1], chars[j]);
		}
		security::SecureString result(std::string_view(chars, len));
		return result;
	}
}