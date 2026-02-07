#include "security.h"

#include <cstring>
#include <sys/stat.h>

namespace security {

// ── Byte-order helpers ───────────────────────────────────────────────────────

namespace {

void put_u32_be(std::vector<uint8_t>& out, uint32_t v) {
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >>  8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >>  0) & 0xFF));
}

uint32_t get_u32_be(const uint8_t* src) {
    return (static_cast<uint32_t>(src[0]) << 24)
         | (static_cast<uint32_t>(src[1]) << 16)
         | (static_cast<uint32_t>(src[2]) <<  8)
         | (static_cast<uint32_t>(src[3]) <<  0);
}

}  // anonymous namespace

// ── File size validation ─────────────────────────────────────────────────────

void validate_file_size(const std::string& path) {
    struct stat info{};
    if (stat(path.c_str(), &info) != 0) {
        throw std::runtime_error{
            "Cannot stat file \"" + path + "\"."};
    }

    const auto size = static_cast<std::size_t>(info.st_size);
    if (size > kMaxFileSize) {
        throw PolicyViolation{
            "Database file exceeds the maximum allowed size of 100 MiB ("
            + std::to_string(size) + " bytes)."};
    }
}

void validate_blob_size(std::size_t size) {
    if (size > kMaxFileSize) {
        throw PolicyViolation{
            "Data blob exceeds the maximum allowed size of 100 MiB ("
            + std::to_string(size) + " bytes)."};
    }
}

// ── Binary field encoding ────────────────────────────────────────────────────

void write_field(std::vector<uint8_t>& out, const std::string& value) {
    put_u32_be(out, static_cast<uint32_t>(value.size()));
    out.insert(out.end(), value.begin(), value.end());
}

void write_field(std::vector<uint8_t>& out, const std::vector<uint8_t>& value) {
    put_u32_be(out, static_cast<uint32_t>(value.size()));
    out.insert(out.end(), value.begin(), value.end());
}

std::string read_string_field(const std::vector<uint8_t>& buf,
                              std::size_t& offset)
{
    if (offset + 4 > buf.size()) {
        throw DeserialisationError{
            "Truncated field: not enough bytes for length prefix."};
    }

    const uint32_t len = get_u32_be(buf.data() + offset);
    offset += 4;

    if (offset + len > buf.size()) {
        throw DeserialisationError{
            "Truncated field: declared length " + std::to_string(len)
            + " exceeds available data."};
    }

    std::string result(reinterpret_cast<const char*>(buf.data() + offset), len);
    offset += len;
    return result;
}

std::vector<uint8_t> read_bytes_field(const std::vector<uint8_t>& buf,
                                      std::size_t& offset)
{
    if (offset + 4 > buf.size()) {
        throw DeserialisationError{
            "Truncated field: not enough bytes for length prefix."};
    }

    const uint32_t len = get_u32_be(buf.data() + offset);
    offset += 4;

    if (offset + len > buf.size()) {
        throw DeserialisationError{
            "Truncated field: declared length " + std::to_string(len)
            + " exceeds available data."};
    }

    std::vector<uint8_t> result(buf.begin() + offset,
                                buf.begin() + offset + len);
    offset += len;
    return result;
}

}  // namespace security
