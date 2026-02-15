#include "crypto_service.hpp"
#include "../core/constants.hpp"
#include "../core/errors.hpp"
#include <string>
#include <vector>
#include <cstdint>
#include <sodium.h>

namespace {
	void validate_salt_size(const std::vector<std::uint8_t>& salt) {
		if (salt.size() != core::constants::kSaltBytes) {
			throw core::errors::KeyDerivationError{
			  "Salt must be exactly " + std::to_string(core::constants::kSaltBytes) + " bytes."
			};
		}
	}
	void validate_ciphertext_size(const std::vector<std::uint8_t>& ciphertext_with_tag) {
		if (ciphertext_with_tag.size() < core::constants::kAeadTagBytes) {
			throw core::errors::AeadError{
			  "Ciphertext is too short to contain an authentication tag."
			};
		}
	}
	void validate_key_size(const std::vector<std::uint8_t>& key) {
		if (key.size() != core::constants::kDerivedKeyLength) {
			throw core::errors::AeadError{
			  "Key must be exactly " + std::to_string(core::constants::kDerivedKeyLength) + " bytes."
			};
		}
	}
	void validate_nonce_size(const std::vector<std::uint8_t>& nonce) {
		if (nonce.size() != core::constants::kAeadNonceBytes) {
			throw core::errors::AeadError{
			  "Nonce must be exactly " + std::to_string(core::constants::kAeadNonceBytes) + " bytes."
			};
		}
	}
}
namespace security::crypto {
	void SodiumCryptoService::initialise() {
		if (sodium_init() < 0) {
			throw core::errors::InitialisationError{
			  "Failed to initialise libsodium. Cannot continue safely."
			};
		}
	}
	std::vector<std::uint8_t> SodiumCryptoService::derive_key(
		const std::string& password,
		const std::vector<std::uint8_t>& salt) {
		validate_salt_size(salt);
		std::vector<std::uint8_t> key(core::constants::kDerivedKeyLength);
		if (crypto_pwhash(
			key.data(),
			core::constants::kDerivedKeyLength,
			password.data(),
			password.size(),
			salt.data(),
			core::constants::kOpsLimit,
			core::constants::kMemLimit,
			core::constants::kAlgorithm) != 0) {
			throw core::errors::KeyDerivationError{
			  "Argon2id key derivation failed (out of memory?)."
			};
		}
		return key;
	}
	std::vector<std::uint8_t> SodiumCryptoService::aead_encrypt(
		const std::vector<std::uint8_t>& plaintext,
		const std::vector<std::uint8_t>& associated_data,
		const std::vector<std::uint8_t>& nonce,
		const std::vector<std::uint8_t>& key) {
		validate_nonce_size(nonce);
		validate_key_size(key);
		std::vector<std::uint8_t> ciphertext(plaintext.size() + core::constants::kAeadTagBytes);
		unsigned long long ciphertext_len = 0;
		if (crypto_aead_xchacha20poly1305_ietf_encrypt(
			ciphertext.data(),
			&ciphertext_len,
			plaintext.data(),
			static_cast<unsigned long long>(plaintext.size()),
			associated_data.data(),
			static_cast<unsigned long long>(associated_data.size()),
			nullptr,
			nonce.data(),
			key.data()) != 0) {
			throw core::errors::AeadError{
			  "AEAD encryption failed."
			};
		}
		ciphertext.resize(static_cast<std::size_t>(ciphertext_len));
		return ciphertext;
	}
	std::vector<std::uint8_t> SodiumCryptoService::aead_decrypt(
		const std::vector<std::uint8_t>& ciphertext_with_tag,
		const std::vector<std::uint8_t>& associated_data,
		const std::vector<std::uint8_t>& nonce,
		const std::vector<std::uint8_t>& key) {
		validate_ciphertext_size(ciphertext_with_tag);
		validate_nonce_size(nonce);
		validate_key_size(key);
		std::vector<std::uint8_t> plaintext(ciphertext_with_tag.size() - core::constants::kAeadTagBytes);
		unsigned long long plaintext_len = 0;
		if (crypto_aead_xchacha20poly1305_ietf_decrypt(
			plaintext.data(),
			&plaintext_len,
			nullptr,
			ciphertext_with_tag.data(),
			static_cast<unsigned long long>(ciphertext_with_tag.size()),
			associated_data.data(),
			static_cast<unsigned long long>(associated_data.size()),
			nonce.data(),
			key.data()) != 0) {
			throw core::errors::AeadError{
			  "AEAD authentication failed — data may have been tampered with."
			};
		}
		plaintext.resize(static_cast<std::size_t>(plaintext_len));
		return plaintext;
	}
	std::vector<std::uint8_t> SodiumCryptoService::random_bytes(std::size_t count) {
		std::vector<std::uint8_t> buf(count);
		randombytes_buf(buf.data(), count);
		return buf;
	}
	std::uint32_t SodiumCryptoService::random_uniform(std::uint32_t upper_bound) {
		return randombytes_uniform(upper_bound);
	}
}