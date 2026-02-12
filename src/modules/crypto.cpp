#include "crypto.hpp"

namespace crypto {
	void initialise() {
		if (sodium_init() < 0) {
			throw validation_security::InitialisationError{
				"Failed to initialise libsodium. Cannot continue safely."
			};
		}
	}

	std::vector<uint8_t> derive_key(
		const std::string& password,
		const std::vector<uint8_t>& salt)
	{
		validation_security::validate_salt_size(salt);

		std::vector<uint8_t> key(validation_security::kDerivedKeyLength);
		if (crypto_pwhash(
			key.data(),
			validation_security::kDerivedKeyLength,
			password.data(),
			password.size(),
			salt.data(),
			validation_security::kOpsLimit,
			validation_security::kMemLimit,
			validation_security::kAlgorithm) != 0)
		{
			throw validation_security::KeyDerivationError{
				"Argon2id key derivation failed (out of memory?)."
			};
		}
		return key;
	}

	std::vector<uint8_t> aead_encrypt(
		const std::vector<uint8_t>& plaintext,
		const std::vector<uint8_t>& associated_data,
		const std::vector<uint8_t>& nonce,
		const std::vector<uint8_t>& key)
	{
		validation_security::validate_nonce_size(nonce);
		validation_security::validate_key_size(key);

		std::vector<uint8_t> ciphertext(plaintext.size() + validation_security::kAeadTagBytes);
		unsigned long long   ciphertext_len = 0;
		if (crypto_aead_xchacha20poly1305_ietf_encrypt(
			ciphertext.data(),
			&ciphertext_len,
			plaintext.data(),
			static_cast<unsigned long long>(plaintext.size()),
			associated_data.data(),
			static_cast<unsigned long long>(associated_data.size()),
			nullptr,
			nonce.data(),
			key.data()) != 0)
		{
			throw validation_security::AeadError{ "AEAD encryption failed." };
		}
		ciphertext.resize(static_cast<std::size_t>(ciphertext_len));
		return ciphertext;
	}
	std::vector<uint8_t> aead_decrypt(
		const std::vector<uint8_t>& ciphertext_with_tag,
		const std::vector<uint8_t>& associated_data,
		const std::vector<uint8_t>& nonce,
		const std::vector<uint8_t>& key)
	{
		validation_security::validate_ciphertext_size(ciphertext_with_tag);
		validation_security::validate_nonce_size(nonce);
		validation_security::validate_key_size(key);

		std::vector<uint8_t> plaintext(ciphertext_with_tag.size() - validation_security::kAeadTagBytes);
		unsigned long long   plaintext_len = 0;
		if (crypto_aead_xchacha20poly1305_ietf_decrypt(
			plaintext.data(),
			&plaintext_len,
			nullptr,
			ciphertext_with_tag.data(),
			static_cast<unsigned long long>(ciphertext_with_tag.size()),
			associated_data.data(),
			static_cast<unsigned long long>(associated_data.size()),
			nonce.data(),
			key.data()) != 0)
		{
			throw validation_security::AeadError{
				"AEAD authentication failed — data may have been tampered with."
			};
		}
		plaintext.resize(static_cast<std::size_t>(plaintext_len));
		return plaintext;
	}

	std::vector<uint8_t> random_bytes(std::size_t count) {
		std::vector<uint8_t> buf(count);
		randombytes_buf(buf.data(), count);
		return buf;
	}
}