#pragma once
#include "../interface/interface_crypto_service.hpp"
#include <cstddef>
#include <string>

namespace filesystem::storage {
	[[nodiscard]] std::string generate_unique_db_filename(domain::interfaces::ICryptoService& crypto);
	[[nodiscard]] std::string normalise_path(const std::string& raw);
	[[nodiscard]] std::string normalise_db_path(const std::string& raw, domain::interfaces::ICryptoService& crypto);
	bool check_file_access(const std::string& path, bool for_write);
	void validate_file_size(const std::string& path);
	void validate_blob_size(std::size_t size);
	bool is_directory(const std::string& path);
	bool is_regular_file(const std::string& path);
}