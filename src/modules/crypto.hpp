#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include "validation/validation_security.hpp"

namespace crypto {
	inline constexpr std::size_t kAeadNonceBytes = validation_security::kAeadNonceBytes;
	inline constexpr std::size_t kAeadTagBytes = validation_security::kAeadTagBytes;
	inline constexpr std::size_t kDerivedKeyLength = validation_security::kDerivedKeyLength;
	inline constexpr std::size_t kSaltBytes = validation_security::kSaltBytes;
	inline constexpr unsigned long long kOpsLimit = validation_security::kOpsLimit;
	inline constexpr std::size_t kMemLimit = validation_security::kMemLimit;
	inline constexpr int kAlgorithm = validation_security::kAlgorithm;

	void initialise();

	[[nodiscard]]
	std::vector<uint8_t> derive_key(
		const std::string& password,
		const std::vector<uint8_t>& salt);

	[[nodiscard]]
	std::vector<uint8_t> aead_encrypt(
		const std::vector<uint8_t>& plaintext,
		const std::vector<uint8_t>& associated_data,
		const std::vector<uint8_t>& nonce,
		const std::vector<uint8_t>& key);

	[[nodiscard]]
	std::vector<uint8_t> aead_decrypt(
		const std::vector<uint8_t>& ciphertext_with_tag,
		const std::vector<uint8_t>& associated_data,
		const std::vector<uint8_t>& nonce,
		const std::vector<uint8_t>& key);

	[[nodiscard]]
	std::vector<uint8_t> random_bytes(std::size_t count);
}