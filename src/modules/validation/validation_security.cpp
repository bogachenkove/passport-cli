#include "validation_security.hpp"

#include <cstring>
#include <fstream>
#include <sys/stat.h>
#include <sodium.h>

namespace validation_security {
	void validate_file_size(const std::string& path) {
		struct stat info {};
		if (stat(path.c_str(), &info) != 0) {
			throw std::runtime_error{
				"Cannot stat file \"" + path + "\"." };
		}
		const auto size = static_cast<std::size_t>(info.st_size);
		if (size > kMaxFileSize) {
			throw PolicyViolation{
				"Database file exceeds the maximum allowed size of 100 MiB ("
				+ std::to_string(size) + " bytes)." };
		}
	}

	void validate_blob_size(std::size_t size) {
		if (size > kMaxFileSize) {
			throw PolicyViolation{
				"Data blob exceeds the maximum allowed size of 100 MiB ("
				+ std::to_string(size) + " bytes)." };
		}
	}

	bool check_file_access(const std::string& path, bool for_write) {
		if (for_write) {
			std::ofstream ofs(path, std::ios::binary | std::ios::app);
			if (!ofs.is_open()) {
				return false;
			}
			ofs.close();
		}
		else {
			std::ifstream ifs(path, std::ios::binary);
			if (!ifs.is_open()) {
				return false;
			}
			ifs.close();
		}
		return true;
	}

	void validate_seed_sizes(const std::vector<uint8_t>& master_seed,
		const std::vector<uint8_t>& context_salt) {
		if (master_seed.size() != kMasterSeedBytes) {
			throw TypeSystemError{
				"master_seed must be exactly "
				+ std::to_string(kMasterSeedBytes) + " bytes." };
		}
		if (context_salt.size() != kContextSaltBytes) {
			throw TypeSystemError{
				"context_salt must be exactly "
				+ std::to_string(kContextSaltBytes) + " bytes." };
		}
	}

	bool record_type_equal(const std::array<uint8_t, 32>& a,
		const std::array<uint8_t, 32>& b) {
		return sodium_memcmp(a.data(), b.data(), a.size()) == 0;
	}

	void validate_salt_size(const std::vector<uint8_t>& salt) {
		if (salt.size() != kSaltBytes) {
			throw KeyDerivationError{
				"Salt must be exactly " + std::to_string(kSaltBytes) + " bytes." };
		}
	}

	void validate_ciphertext_size(const std::vector<uint8_t>& ciphertext_with_tag) {
		if (ciphertext_with_tag.size() < kAeadTagBytes) {
			throw AeadError{
				"Ciphertext is too short to contain an authentication tag." };
		}
	}

	void validate_key_size(const std::vector<uint8_t>& key) {
		if (key.size() != kDerivedKeyLength) {
			throw AeadError{
				"Key must be exactly " + std::to_string(kDerivedKeyLength) + " bytes." };
		}
	}

	void validate_nonce_size(const std::vector<uint8_t>& nonce) {
		if (nonce.size() != kAeadNonceBytes) {
			throw AeadError{
				"Nonce must be exactly " + std::to_string(kAeadNonceBytes) + " bytes." };
		}
	}
}