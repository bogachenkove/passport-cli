#pragma once

/// @file crypto.h
/// @brief Cryptographic primitives — thin wrappers around libsodium.
///
/// All custom implementations (SHA-256, HMAC, PBKDF2) have been removed.
/// Key derivation now uses Argon2id via crypto_pwhash().
/// AEAD uses XChaCha20-Poly1305 (24-byte nonce) to eliminate nonce-reuse risk.

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <sodium.h>

namespace crypto {

// ── Constants ────────────────────────────────────────────────────────────────

/// AEAD nonce size for XChaCha20-Poly1305 (24 bytes).
inline constexpr std::size_t kAeadNonceBytes =
    crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;

/// AEAD authentication tag overhead (16 bytes).
inline constexpr std::size_t kAeadTagBytes =
    crypto_aead_xchacha20poly1305_ietf_ABYTES;

/// Derived key length used for AEAD encryption (32 bytes).
inline constexpr std::size_t kDerivedKeyLength =
    crypto_aead_xchacha20poly1305_ietf_KEYBYTES;

/// Salt length for Argon2id key derivation (16 bytes, libsodium requirement).
inline constexpr std::size_t kSaltBytes = crypto_pwhash_SALTBYTES;

// ── Argon2id parameters (INTERACTIVE profile) ────────────────────────────────

inline constexpr unsigned long long kOpsLimit =
    crypto_pwhash_OPSLIMIT_INTERACTIVE;          // 2

inline constexpr std::size_t kMemLimit =
    crypto_pwhash_MEMLIMIT_INTERACTIVE;           // 64 MiB

inline constexpr int kAlgorithm =
    crypto_pwhash_ALG_ARGON2ID13;

// ── Exceptions ───────────────────────────────────────────────────────────────

/// Thrown when libsodium cannot be initialised.
struct InitialisationError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

/// Thrown when an AEAD encryption or decryption operation fails.
struct AeadError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

/// Thrown when key derivation fails.
struct KeyDerivationError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// ── Public API ───────────────────────────────────────────────────────────────

/// Initialise libsodium.  Must be called once before any other function.
/// @throws InitialisationError on failure.
void initialise();

/// Derive an encryption key from a master password and salt using Argon2id.
///
/// @param password    Master password (UTF-8).
/// @param salt        Random salt (must be exactly kSaltBytes).
/// @returns           Derived key of kDerivedKeyLength bytes.
/// @throws KeyDerivationError on failure.
[[nodiscard]]
std::vector<uint8_t> derive_key(
    const std::string&          password,
    const std::vector<uint8_t>& salt);

/// Encrypt plaintext with XChaCha20-Poly1305 AEAD.
///
/// @returns ciphertext || tag  (plaintext.size() + kAeadTagBytes bytes).
/// @throws AeadError on failure.
[[nodiscard]]
std::vector<uint8_t> aead_encrypt(
    const std::vector<uint8_t>& plaintext,
    const std::vector<uint8_t>& associated_data,
    const std::vector<uint8_t>& nonce,
    const std::vector<uint8_t>& key);

/// Decrypt and authenticate ciphertext produced by aead_encrypt().
///
/// @returns recovered plaintext.
/// @throws AeadError if authentication fails (tampered / wrong key).
[[nodiscard]]
std::vector<uint8_t> aead_decrypt(
    const std::vector<uint8_t>& ciphertext_with_tag,
    const std::vector<uint8_t>& associated_data,
    const std::vector<uint8_t>& nonce,
    const std::vector<uint8_t>& key);

/// Generate cryptographically secure random bytes.
[[nodiscard]]
std::vector<uint8_t> random_bytes(std::size_t count);

}  // namespace crypto
