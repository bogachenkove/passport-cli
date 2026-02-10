#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <sodium.h>

namespace crypto {
	inline constexpr std::size_t kAeadNonceBytes = crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;
	inline constexpr std::size_t kAeadTagBytes = crypto_aead_xchacha20poly1305_ietf_ABYTES;
	inline constexpr std::size_t kDerivedKeyLength = crypto_aead_xchacha20poly1305_ietf_KEYBYTES;
	inline constexpr std::size_t kSaltBytes = crypto_pwhash_SALTBYTES;
	inline constexpr unsigned long long kOpsLimit = crypto_pwhash_OPSLIMIT_INTERACTIVE;
	inline constexpr std::size_t kMemLimit = crypto_pwhash_MEMLIMIT_INTERACTIVE;
	inline constexpr int kAlgorithm = crypto_pwhash_ALG_ARGON2ID13;
	struct InitialisationError : std::runtime_error {
		using std::runtime_error::runtime_error;
	};
	struct AeadError : std::runtime_error {
		using std::runtime_error::runtime_error;
	};
	struct KeyDerivationError : std::runtime_error {
		using std::runtime_error::runtime_error;
	};
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