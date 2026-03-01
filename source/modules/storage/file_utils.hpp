#pragma once
#include "../interface/interface_crypto_service.hpp"
#include <cstddef>
#include <string>
#include <string_view>

namespace filesystem::storage {
	[[nodiscard]] std::string generate_unique_db_filename(domain::interfaces::ICryptoService& crypto);
	[[nodiscard]] std::string normalise_path(std::string_view raw);
	[[nodiscard]] std::string normalise_db_path(std::string_view raw, domain::interfaces::ICryptoService& crypto);
	bool check_file_access(std::string_view path, bool for_write);
	void validate_file_size(std::string_view path);
	void validate_blob_size(std::size_t size);
	bool is_directory(std::string_view path);
	bool is_regular_file(std::string_view path);
}