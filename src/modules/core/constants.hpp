#pragma once

#include <cstddef>
#include <cstdint>
#include <sodium.h>

namespace core::constants {
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
	inline constexpr std::uint8_t kFileMagic[4] = {'P', 'A', 'S', 'S'};
	inline constexpr std::size_t  kHeaderAdSize = 64;
	inline constexpr std::size_t kPasswordMinLength_MasterPassword = 12;
	inline constexpr std::size_t kPasswordMaxLength_MasterPassword = 128;
	inline constexpr std::size_t kLoginMinLength_Password = 3;
	inline constexpr std::size_t kLoginMaxLength_Password = 100;
	inline constexpr std::size_t kPasswordMinLength_Password = 8;
	inline constexpr std::size_t kPasswordMaxLength_Password = 128;
	inline constexpr std::size_t kUrlMinLength_Password = 11;
	inline constexpr std::size_t kUrlMaxLength_Password = 100;
	inline constexpr std::size_t kNoteMinLength_Password = 2;
	inline constexpr std::size_t kNoteMaxLength_Password = 100;
	inline constexpr std::size_t kTitleMinLength_Note = 3;
	inline constexpr std::size_t kTitleMaxLength_Note = 12;
	inline constexpr std::size_t kNoteMinLength_Note = 5;
	inline constexpr std::size_t kNoteMaxLength_Note = 500;
}