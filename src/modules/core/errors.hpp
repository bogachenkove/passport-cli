#pragma once

#include <stdexcept>
#include <string>

namespace core::errors {

    // Базовый класс для всех исключений приложения
    struct PassportError : std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    // Ошибки криптографического слоя
    struct CryptoError : PassportError {
        using PassportError::PassportError;
    };

    struct AeadError : CryptoError {
        using CryptoError::CryptoError;
    };

    struct KeyDerivationError : CryptoError {
        using CryptoError::CryptoError;
    };

    struct InitialisationError : CryptoError {
        using CryptoError::CryptoError;
    };

    // Ошибки сериализации / десериализации
    struct DeserialisationError : PassportError {
        using PassportError::PassportError;
    };

    // Ошибки, связанные с типовой системой (BLAKE2b)
    struct TypeSystemError : PassportError {
        using PassportError::PassportError;
    };

    // Нарушение политик безопасности (размер файла, длина пароля и т.д.)
    struct PolicyViolation : PassportError {
        using PassportError::PassportError;
    };

    // Ошибки доступа к файлам (не включая политики)
    struct FileAccessError : PassportError {
        using PassportError::PassportError;
    };

    // Ошибки бизнес‑логики (неверный формат записи и т.п.)
    struct DomainError : PassportError {
        using PassportError::PassportError;
    };

} // namespace core::errors