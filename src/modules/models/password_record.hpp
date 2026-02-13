#pragma once

#include <cstdint>
#include <string>

namespace domain::models {

    struct PasswordRecord {
        std::uint64_t date = 0;
        std::string   login;
        std::string   password;
        std::string   url;
        std::string   note;
    };

} // namespace domain::models