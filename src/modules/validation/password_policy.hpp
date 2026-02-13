#pragma once

#include <string>
#include <cstddef>
#include "../../core/constants.hpp"

namespace domain::validation {
	inline bool is_master_password_length_valid(const std::string& pw) {
		return pw.size() >= core::constants::kPasswordMinLength_MasterPassword &&
			pw.size() <= core::constants::kPasswordMaxLength_MasterPassword;
	}
}