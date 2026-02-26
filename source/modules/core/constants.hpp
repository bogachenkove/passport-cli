#pragma once
#include <cstddef>
#include <cstdint>
#include <string> // may have to be removed
#include <array>
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
	inline constexpr std::uint8_t kFileMagic[4] = {'P','S','R','T'};
	inline constexpr std::uint8_t kFileMagicNull = 0x00;
	inline constexpr std::size_t  kHeaderAdSize = 65;
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
	inline constexpr std::size_t kCardNumberMinLength_BankCard = 13;
	inline constexpr std::size_t kCardNumberMaxLength_BankCard = 19;
	inline constexpr std::size_t kCVVMinLength_BankCard = 3;
	inline constexpr std::size_t kCVVMaxLength_BankCard = 4;
	inline constexpr std::size_t kCardHolderNameMinLength_BankCard = 4;
	inline constexpr std::size_t kCardHolderNameMaxLength_BankCard = 30;
	inline constexpr std::size_t kNoteMinLength_BankCard = 5;
	inline constexpr std::size_t kNoteMaxLength_BankCard = 30;
	inline constexpr std::size_t kCardNumberMinLength_DiscountCard = 8;
	inline constexpr std::size_t kCardNumberMaxLength_DiscountCard = 16;
	inline constexpr std::size_t kBarCodeMinLength_DiscountCard = 6;
	inline constexpr std::size_t kBarCodeMaxLength_DiscountCard = 13;
	inline constexpr std::size_t kCVVMinLength_DiscountCard = 3;
	inline constexpr std::size_t kCVVMaxLength_DiscountCard = 4;
	inline constexpr std::size_t kStoreNameMinLength_DiscountCard = 4;
	inline constexpr std::size_t kStoreNameMaxLength_DiscountCard = 30;
	inline constexpr std::size_t kNoteMinLength_DiscountCard = 5;
	inline constexpr std::size_t kNoteMaxLength_DiscountCard = 30;
	inline constexpr std::size_t kCardNumberMinLength_TransportCard = 8;
	inline constexpr std::size_t kCardNumberMaxLength_TransportCard = 16;
	inline constexpr std::size_t kBarCodeMinLength_TransportCard = 6;
	inline constexpr std::size_t kBarCodeMaxLength_TransportCard = 13;
	inline constexpr std::size_t kCVVMinLength_TransportCard = 3;
	inline constexpr std::size_t kCVVMaxLength_TransportCard = 4;
	inline constexpr std::size_t kHolderMinLength_TransportCard = 4;
	inline constexpr std::size_t kHolderMaxLength_TransportCard = 30;
	inline constexpr std::size_t kNoteMinLength_TransportCard = 5;
	inline constexpr std::size_t kNoteMaxLength_TransportCard = 30;
	inline constexpr std::array<std::size_t, 5> kValidMnemonicWordCounts = { 12, 15, 18, 21, 24 };
	inline constexpr std::size_t kPassphraseMinLength_Mnemonic = 1;
	inline constexpr std::size_t kPassphraseMaxLength_Mnemonic = 100;
	inline constexpr std::size_t kNoteMinLength_Mnemonic = 5;
	inline constexpr std::size_t kNoteMaxLength_Mnemonic = 30;
	inline constexpr std::uint32_t kIterationMin_Mnemonic = 2048;
	inline constexpr std::uint32_t kIterationMax_Mnemonic = 1000000;
	inline constexpr std::size_t kSSIDMinLength_WiFi = 3;
	inline constexpr std::size_t kSSIDMaxLength_WiFi = 30;
	inline constexpr std::size_t kPasswordMinLength_WiFi = 8;
	inline constexpr std::size_t kPasswordMaxLength_WiFi = 128;
	inline constexpr std::size_t kNoteMinLength_WiFi = 5;
	inline constexpr std::size_t kNoteMaxLength_WiFi = 30;
	inline constexpr const char* kLowercaseChars = "abcdefghijklmnopqrstuvwxyz";
	inline constexpr const char* kUppercaseChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	inline constexpr const char* kDigitChars = "0123456789";
	inline constexpr const char* kSpecialChars = "!@#$%^&*()_+-=[]{}|;:,.<>?";
	inline const std::string kAllowedPasswordChars = std::string(kLowercaseChars) + std::string(kUppercaseChars) + std::string(kDigitChars) + std::string(kSpecialChars);
}