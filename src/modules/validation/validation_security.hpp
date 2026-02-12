#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <array>
#include <sodium.h>

namespace validation_security {
	inline constexpr const char* kContextPassword = { "PASSPORT::PASSWORD//DATABASE" };
	inline constexpr const char* kContextNote = { "PASSPORT::NOTE//DATABASE" };
	inline constexpr std::size_t kMaxFileSize = 100ULL * 1024 * 1024;
	inline constexpr std::size_t kMasterSeedBytes = 32;
	inline constexpr std::size_t kContextSaltBytes = 16;
	inline constexpr std::size_t kAeadNonceBytes = crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;
	inline constexpr std::size_t kAeadTagBytes = crypto_aead_xchacha20poly1305_ietf_ABYTES;
	inline constexpr std::size_t kDerivedKeyLength = crypto_aead_xchacha20poly1305_ietf_KEYBYTES;
	inline constexpr std::size_t kSaltBytes = crypto_pwhash_SALTBYTES;
	inline constexpr unsigned long long kOpsLimit = crypto_pwhash_OPSLIMIT_INTERACTIVE;
	inline constexpr std::size_t kMemLimit = crypto_pwhash_MEMLIMIT_INTERACTIVE;
	inline constexpr int kAlgorithm = crypto_pwhash_ALG_ARGON2ID13;
	struct PolicyViolation : std::runtime_error {
		using std::runtime_error::runtime_error;
	};

	struct DeserialisationError : std::runtime_error {
		using std::runtime_error::runtime_error;
	};

	struct TypeSystemError : std::runtime_error {
		using std::runtime_error::runtime_error;
	};

	struct InitialisationError : std::runtime_error {
		using std::runtime_error::runtime_error;
	};

	struct AeadError : std::runtime_error {
		using std::runtime_error::runtime_error;
	};

	struct KeyDerivationError : std::runtime_error {
		using std::runtime_error::runtime_error;
	};
	void validate_file_size(const std::string& path);
	void validate_blob_size(std::size_t size);
	bool check_file_access(const std::string& path, bool for_write);
	void validate_seed_sizes(const std::vector<uint8_t>& master_seed,
		const std::vector<uint8_t>& context_salt);
	bool record_type_equal(const std::array<uint8_t, 32>& a,
		const std::array<uint8_t, 32>& b);
	void validate_salt_size(const std::vector<uint8_t>& salt);
	void validate_ciphertext_size(const std::vector<uint8_t>& ciphertext_with_tag);
	void validate_key_size(const std::vector<uint8_t>& key);
	void validate_nonce_size(const std::vector<uint8_t>& nonce);
}