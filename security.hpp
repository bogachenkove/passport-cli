#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace security {
    inline constexpr std::size_t kMaxFileSize = 100ULL * 1024 * 1024;
    struct PolicyViolation : std::runtime_error {
        using std::runtime_error::runtime_error;
    };
    struct DeserialisationError : std::runtime_error {
        using std::runtime_error::runtime_error;
    };
    void validate_file_size(const std::string& path);
    void validate_blob_size(std::size_t size);
    bool check_file_access(const std::string& path, bool for_write);
    void write_field(std::vector<uint8_t>& out, const std::string& value);
    void write_field(std::vector<uint8_t>& out, const std::vector<uint8_t>& value);
    [[nodiscard]]
    std::string read_string_field(const std::vector<uint8_t>& buf,
        std::size_t& offset);
    [[nodiscard]]
    std::vector<uint8_t> read_bytes_field(const std::vector<uint8_t>& buf,
        std::size_t& offset);
}