#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <array>
#include "../core/constants.hpp"
#include "../core/errors.hpp"

namespace validation_security {
	void validate_file_size(const std::string& path);
	void validate_blob_size(std::size_t size);
	void validate_seed_sizes(const std::vector<uint8_t>& master_seed,
		const std::vector<uint8_t>& context_salt);
	bool record_type_equal(const std::array<uint8_t, 32>& a,
		const std::array<uint8_t, 32>& b);
	void validate_salt_size(const std::vector<uint8_t>& salt);
	void validate_ciphertext_size(const std::vector<uint8_t>& ciphertext_with_tag);
	void validate_key_size(const std::vector<uint8_t>& key);
	void validate_nonce_size(const std::vector<uint8_t>& nonce);
}