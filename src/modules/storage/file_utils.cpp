#include "file_utils.hpp"
#include "../core/constants.hpp"
#include "../core/errors.hpp"
#include "../core/platform.hpp"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <filesystem>

namespace filesystem::storage {
	std::string generate_unique_db_filename(
		domain::interfaces::ICryptoService& crypto) {
		constexpr std::size_t kEntropyBytes = 16;
		auto random = crypto.random_bytes(kEntropyBytes);
		std::ostringstream oss;
		oss << "passport_";
		oss << std::hex << std::setfill('0');
		for (auto byte : random) {
			oss << std::setw(2) << static_cast<unsigned>(byte);
		}
		oss << ".db";
		return oss.str();
	}
	std::string normalise_path(const std::string& raw) {
		std::string path = raw;
		while (!path.empty() && (path.back() == ' ' || path.back() == '\t')) {
			path.pop_back();
		}
		if (path.empty()) {
			return path;
		}
		if (!core::platform::path_is_absolute(path)) {
			path = core::platform::absolute_path(path);
		}
		return path;
	}
	std::string normalise_db_path(
		const std::string& raw,
		domain::interfaces::ICryptoService& crypto) {
		std::string path = normalise_path(raw);
		if (path.empty()) {
			return path;
		}
		if (core::platform::is_directory(path)) {
			char sep = core::platform::path_separator();
			if (!path.empty() && path.back() != sep) {
				path += sep;
			}
			path += generate_unique_db_filename(crypto);
		}
		return path;
	}
	bool check_file_access(const std::string& path, bool for_write) {
		if (for_write) {
			std::ofstream ofs(path, std::ios::binary | std::ios::app);
			if (!ofs.is_open()) return false;
			ofs.close();
		}
		else {
			std::ifstream ifs(path, std::ios::binary);
			if (!ifs.is_open()) return false;
			ifs.close();
		}
		return true;
	}
	void validate_file_size(const std::string& path) {
		std::error_code ec;
		auto size = std::filesystem::file_size(std::filesystem::path(path), ec);
		if (ec) {
			throw core::errors::FileAccessError{
			  "Cannot stat file \"" + path + "\": " + ec.message()
			};
		}
		if (static_cast<std::size_t>(size) > core::constants::kMaxFileSize) {
			throw core::errors::PolicyViolation{
			  "Database file exceeds the maximum allowed size of 100 MiB (" +
			  std::to_string(size) + " bytes)."
			};
		}
	}
	void validate_blob_size(std::size_t size) {
		if (size > core::constants::kMaxFileSize) {
			throw core::errors::PolicyViolation{
			  "Data blob exceeds the maximum allowed size of 100 MiB (" +
			  std::to_string(size) + " bytes)."
			};
		}
	}
	bool is_directory(const std::string& path) {
		return core::platform::is_directory(path);
	}
	bool is_regular_file(const std::string& path) {
		return core::platform::is_regular_file(path);
	}
}