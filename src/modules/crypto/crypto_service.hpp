#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "../interface/interface_crypto_service.hpp"

namespace security::crypto {
	class SodiumCryptoService final : public domain::interfaces::ICryptoService {
	public:
		void initialise() override;
		[[nodiscard]] std::vector<std::uint8_t> derive_key(const std::string& password, const std::vector<std::uint8_t>& salt) override;
		[[nodiscard]] std::vector<std::uint8_t> aead_encrypt(const std::vector<std::uint8_t>& plaintext, const std::vector<std::uint8_t>& associated_data, const std::vector<std::uint8_t>& nonce, const std::vector<std::uint8_t>& key) override;
		[[nodiscard]] std::vector<std::uint8_t> aead_decrypt(const std::vector<std::uint8_t>& ciphertext_with_tag, const std::vector<std::uint8_t>& associated_data, const std::vector<std::uint8_t>& nonce, const std::vector<std::uint8_t>& key) override;
		[[nodiscard]] std::vector<std::uint8_t> random_bytes(std::size_t count) override;
		[[nodiscard]] std::uint32_t random_uniform(std::uint32_t upper_bound) override;
	};
}