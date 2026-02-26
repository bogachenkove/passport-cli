#pragma once
#include "../interface/interface_crypto_service.hpp"
#include "../models/record_types.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace security::crypto {
	class TypeSystem {
	public:
		TypeSystem() = default;
		TypeSystem(const std::vector<std::uint8_t>& master_seed, const std::vector<std::uint8_t>& context_salt);
		void generate_seeds(domain::interfaces::ICryptoService& crypto);
		[[nodiscard]] domain::models::RecordType derive_type(const std::string& context_string) const;
		[[nodiscard]] domain::models::RecordType password_type() const;
		[[nodiscard]] domain::models::RecordType note_type() const;
		[[nodiscard]] domain::models::RecordType bankcard_type() const;
		[[nodiscard]] domain::models::RecordType discountcard_type() const;
		[[nodiscard]] domain::models::RecordType transportcard_type() const;
		[[nodiscard]] domain::models::RecordType mnemonic_type() const;
		[[nodiscard]] domain::models::RecordType wifi_type() const;
		[[nodiscard]] const std::vector<std::uint8_t>& master_seed() const noexcept;
		[[nodiscard]] const std::vector<std::uint8_t>& context_salt() const noexcept;
		[[nodiscard]] bool is_initialised() const noexcept;
	private:
		std::vector<std::uint8_t> master_seed_;
		std::vector<std::uint8_t> context_salt_;
	};
}