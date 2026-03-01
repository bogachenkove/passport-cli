#pragma once
#include "../core/constants.hpp"
#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>

namespace domain::validation {
    inline bool is_master_password_length_valid(std::string_view pw) {
        return pw.size() >= core::constants::kPasswordMinLength_MasterPassword &&
            pw.size() <= core::constants::kPasswordMaxLength_MasterPassword;
    }
    inline bool is_password_characters_valid(std::string_view pw) {
        return std::all_of(pw.begin(), pw.end(), [](unsigned char ch) {
            return core::constants::kAllowedPasswordChars.find(ch) != std::string::npos;
            }
        );
    }
    inline bool is_master_password_complex(std::string_view pw) {
        bool has_lower = false;
        bool has_upper = false;
        bool has_digit = false;
        bool has_special = false;
        for (unsigned char ch : pw) {
            if (std::islower(ch)) {
                has_lower = true;
            }
            else if (std::isupper(ch)) {
                has_upper = true;
            }
            else if (std::isdigit(ch)) {
                has_digit = true;
            }
            else {
                has_special = true;
            }
        }
        return has_lower && has_upper && has_digit && has_special;
    }
}