#include "type_system.hpp"
#include "../core/constants.hpp"
#include "../core/errors.hpp"
#include <string>
#include <sodium.h>

namespace security::crypto {
	TypeSystem::TypeSystem(const std::vector<std::uint8_t>& master_seed, const std::vector<std::uint8_t>& context_salt) : master_seed_(master_seed), context_salt_(context_salt) {
		if (master_seed.size() != core::constants::kMasterSeedBytes ||
			context_salt.size() != core::constants::kContextSaltBytes) {
			throw core::errors::TypeSystemError{
			  "Seed sizes mismatch: expected " + std::to_string(core::constants::kMasterSeedBytes) + " and " + std::to_string(core::constants::kContextSaltBytes) + " bytes."
			};
		}
	}
	void TypeSystem::generate_seeds(domain::interfaces::ICryptoService& crypto) {
		master_seed_ = crypto.random_bytes(core::constants::kMasterSeedBytes);
		context_salt_ = crypto.random_bytes(core::constants::kContextSaltBytes);
	}
	domain::models::RecordType
		TypeSystem::derive_type(const std::string& context_string) const {
		if (!is_initialised()) {
			throw core::errors::TypeSystemError{
			  "TypeSystem is not initialised — seeds are missing."
			};
		}
		std::vector<std::uint8_t> input;
		input.reserve(master_seed_.size() + context_salt_.size() + context_string.size());
		input.insert(input.end(), master_seed_.begin(), master_seed_.end());
		input.insert(input.end(), context_salt_.begin(), context_salt_.end());
		input.insert(input.end(), context_string.begin(), context_string.end());
		domain::models::RecordType result{
		};
		if (crypto_generichash(
			result.data(), result.size(),
			input.data(),
			static_cast<unsigned long long>(input.size()),
			nullptr, 0) != 0) {
			throw core::errors::TypeSystemError{
			  "BLAKE2b hash computation failed."
			};
		}
		return result;
	}
	domain::models::RecordType TypeSystem::password_type() const {
		return derive_type(domain::models::kContextPassword);
	}
	domain::models::RecordType TypeSystem::note_type() const {
		return derive_type(domain::models::kContextNote);
	}
	domain::models::RecordType TypeSystem::bankcard_type() const {
		return derive_type(domain::models::kContextBankCard);
	}
	domain::models::RecordType TypeSystem::discountcard_type() const {
		return derive_type(domain::models::kContextDiscountCard);
	}
	domain::models::RecordType TypeSystem::transportcard_type() const {
		return derive_type(domain::models::kContextTransportCard);
	}
	domain::models::RecordType TypeSystem::mnemonicphrase_type() const {
		return derive_type(domain::models::kContextMnemonicPhrase);
	}
	const std::vector<std::uint8_t>& TypeSystem::master_seed() const noexcept {
		return master_seed_;
	}
	const std::vector<std::uint8_t>& TypeSystem::context_salt() const noexcept {
		return context_salt_;
	}
	bool TypeSystem::is_initialised() const noexcept {
		return master_seed_.size() == core::constants::kMasterSeedBytes && context_salt_.size() == core::constants::kContextSaltBytes;
	}
}