#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace domain::interfaces {

class ICryptoService {
public:
    virtual ~ICryptoService() = default;

    // Инициализация криптографической библиотеки (libsodium)
    virtual void initialise() = 0;

    // Вывод ключа из пароля и соли (Argon2id)
    [[nodiscard]]
    virtual std::vector<std::uint8_t> derive_key(
        const std::string& password,
        const std::vector<std::uint8_t>& salt) = 0;

    // AEAD-шифрование (XChaCha20-Poly1305)
    [[nodiscard]]
    virtual std::vector<std::uint8_t> aead_encrypt(
        const std::vector<std::uint8_t>& plaintext,
        const std::vector<std::uint8_t>& associated_data,
        const std::vector<std::uint8_t>& nonce,
        const std::vector<std::uint8_t>& key) = 0;

    // AEAD-расшифровка
    [[nodiscard]]
    virtual std::vector<std::uint8_t> aead_decrypt(
        const std::vector<std::uint8_t>& ciphertext_with_tag,
        const std::vector<std::uint8_t>& associated_data,
        const std::vector<std::uint8_t>& nonce,
        const std::vector<std::uint8_t>& key) = 0;

    // Криптографически стойкие случайные байты
    [[nodiscard]]
    virtual std::vector<std::uint8_t> random_bytes(std::size_t count) = 0;
};

} // namespace domain::interfaces