#pragma once

#include <stdexcept>
#include <string>

namespace core::errors {
	struct PassportError : std::runtime_error {
		using std::runtime_error::runtime_error;
	};
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
	struct DeserialisationError : PassportError {
		using PassportError::PassportError;
	};
	struct TypeSystemError : PassportError {
		using PassportError::PassportError;
	};
	struct PolicyViolation : PassportError {
		using PassportError::PassportError;
	};
	struct FileAccessError : PassportError {
		using PassportError::PassportError;
	};
	struct DomainError : PassportError {
		using PassportError::PassportError;
	};
}