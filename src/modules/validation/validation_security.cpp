#include "validation_security.hpp"

#include <cstring>
#include <fstream>

namespace validation_security {
	void validate_file_size(const std::string& path) {
		struct stat info {};
		if (stat(path.c_str(), &info) != 0) {
			throw std::runtime_error{
				"Cannot stat file \"" + path + "\"." };
		}
		const auto size = static_cast<std::size_t>(info.st_size);
		if (size > core::constants::kMaxFileSize) {
			throw core::errors::PolicyViolation{
				"Database file exceeds the maximum allowed size of 100 MiB ("
				+ std::to_string(size) + " bytes)." };
		}
	}

	void validate_blob_size(std::size_t size) {
		if (size > core::constants::kMaxFileSize) {
			throw core::errors::PolicyViolation{
				"Data blob exceeds the maximum allowed size of 100 MiB ("
				+ std::to_string(size) + " bytes)." };
		}
	}

	void validate_seed_sizes(const std::vector<uint8_t>& master_seed,
		const std::vector<uint8_t>& context_salt) {
		if (master_seed.size() != core::constants::kMasterSeedBytes) {
			throw core::errors::TypeSystemError{
				"master_seed must be exactly "
				+ std::to_string(core::constants::kMasterSeedBytes) + " bytes." };
		}
		if (context_salt.size() != core::constants::kContextSaltBytes) {
			throw core::errors::TypeSystemError{
				"context_salt must be exactly "
				+ std::to_string(core::constants::kContextSaltBytes) + " bytes." };
		}
	}

	bool record_type_equal(const std::array<uint8_t, 32>& a,
		const std::array<uint8_t, 32>& b) {
		return sodium_memcmp(a.data(), b.data(), a.size()) == 0;
	}

	void validate_salt_size(const std::vector<uint8_t>& salt) {
		if (salt.size() != core::constants::kSaltBytes) {
			throw core::errors::KeyDerivationError{
				"Salt must be exactly " + std::to_string(core::constants::kSaltBytes) + " bytes." };
		}
	}

	void validate_ciphertext_size(const std::vector<uint8_t>& ciphertext_with_tag) {
		if (ciphertext_with_tag.size() < core::constants::kAeadTagBytes) {
			throw core::errors::AeadError{
				"Ciphertext is too short to contain an authentication tag." };
		}
	}

	void validate_key_size(const std::vector<uint8_t>& key) {
		if (key.size() != core::constants::kDerivedKeyLength) {
			throw core::errors::AeadError{
				"Key must be exactly " + std::to_string(core::constants::kDerivedKeyLength) + " bytes." };
		}
	}

	void validate_nonce_size(const std::vector<uint8_t>& nonce) {
		if (nonce.size() != core::constants::kAeadNonceBytes) {
			throw core::errors::AeadError{
				"Nonce must be exactly " + std::to_string(core::constants::kAeadNonceBytes) + " bytes." };
		}
	}
}