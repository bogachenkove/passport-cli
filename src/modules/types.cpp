#include "types.hpp"
#include "crypto.hpp"
#include "validation/validation_security.hpp"

#include <cstring>
#include <sodium.h>

namespace types {
	TypeSystem::TypeSystem(const std::vector<uint8_t>& master_seed,
		const std::vector<uint8_t>& context_salt)
		: master_seed_(master_seed), context_salt_(context_salt) {
		validation_security::validate_seed_sizes(master_seed_, context_salt_);
	}

	void TypeSystem::generate_seeds() {
		master_seed_ = crypto::random_bytes(validation_security::kMasterSeedBytes);
		context_salt_ = crypto::random_bytes(validation_security::kContextSaltBytes);
	}

	RecordType TypeSystem::derive_type(const std::string& context_string) const {
		if (!is_initialised()) {
			throw validation_security::TypeSystemError{
				"TypeSystem is not initialised — seeds are missing."
			};
		}

		std::vector<uint8_t> input;
		input.reserve(master_seed_.size()
			+ context_salt_.size()
			+ context_string.size());
		input.insert(input.end(), master_seed_.begin(), master_seed_.end());
		input.insert(input.end(), context_salt_.begin(), context_salt_.end());
		input.insert(input.end(), context_string.begin(), context_string.end());

		RecordType result{};
		if (crypto_generichash(
			result.data(), result.size(),
			input.data(),
			static_cast<unsigned long long>(input.size()),
			nullptr, 0) != 0) {
			throw validation_security::TypeSystemError{
				"BLAKE2b hash computation failed."
			};
		}
		return result;
	}

	RecordType TypeSystem::password_type() const {
		return derive_type(validation_security::kContextPassword);
	}

	RecordType TypeSystem::note_type() const {
		return derive_type(validation_security::kContextNote);
	}

	const std::vector<uint8_t>& TypeSystem::master_seed() const noexcept {
		return master_seed_;
	}

	const std::vector<uint8_t>& TypeSystem::context_salt() const noexcept {
		return context_salt_;
	}

	bool TypeSystem::is_initialised() const noexcept {
		return master_seed_.size() == validation_security::kMasterSeedBytes
			&& context_salt_.size() == validation_security::kContextSaltBytes;
	}
}