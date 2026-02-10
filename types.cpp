#include "types.hpp"
#include "crypto.hpp"

#include <cstring>
#include <sodium.h>

namespace types {
	TypeSystem::TypeSystem(const std::vector<uint8_t>& master_seed, const std::vector<uint8_t>& context_salt) : master_seed_(master_seed), context_salt_(context_salt)
	{
		if (master_seed_.size() != kMasterSeedBytes) {
			throw TypeSystemError{
				"master_seed must be exactly "
				+ std::to_string(kMasterSeedBytes) + " bytes." };
		}
		if (context_salt_.size() != kContextSaltBytes) {
			throw TypeSystemError{
				"context_salt must be exactly "
				+ std::to_string(kContextSaltBytes) + " bytes." };
		}
	}
	void TypeSystem::generate_seeds() {
		master_seed_ = crypto::random_bytes(kMasterSeedBytes);
		context_salt_ = crypto::random_bytes(kContextSaltBytes);
	}
	RecordType TypeSystem::derive_type(const std::string& context_string) const {
		if (!is_initialised()) {
			throw TypeSystemError{
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
			nullptr, 0) != 0)
		{
			throw TypeSystemError{
				"BLAKE2b hash computation failed."
			};
		}
		return result;
	}
	RecordType TypeSystem::password_type() const {
		return derive_type(kContextPassword);
	}
	RecordType TypeSystem::note_type() const {
		return derive_type(kContextNote);
	}
	const std::vector<uint8_t>& TypeSystem::master_seed() const noexcept {
		return master_seed_;
	}
	const std::vector<uint8_t>& TypeSystem::context_salt() const noexcept {
		return context_salt_;
	}
	bool TypeSystem::is_initialised() const noexcept {
		return master_seed_.size() == kMasterSeedBytes
			&& context_salt_.size() == kContextSaltBytes;
	}
	bool record_type_equal(const RecordType& a, const RecordType& b) {
		return sodium_memcmp(a.data(), b.data(), a.size()) == 0;
	}
}