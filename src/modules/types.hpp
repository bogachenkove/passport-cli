#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace types {
	using RecordType = std::array<uint8_t, 32>;
	class TypeSystem {
	public:
		TypeSystem() = default;
		TypeSystem(const std::vector<uint8_t>& master_seed,
			const std::vector<uint8_t>& context_salt);
		void generate_seeds();

		[[nodiscard]]
		RecordType derive_type(const std::string& context_string) const;

		[[nodiscard]] RecordType password_type() const;
		[[nodiscard]] RecordType note_type()     const;

		[[nodiscard]] const std::vector<uint8_t>& master_seed()  const noexcept;
		[[nodiscard]] const std::vector<uint8_t>& context_salt() const noexcept;
		[[nodiscard]] bool is_initialised() const noexcept;

	private:
		std::vector<uint8_t> master_seed_;
		std::vector<uint8_t> context_salt_;
	};
}