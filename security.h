#pragma once

/// @file security.h
/// @brief Security policies and binary serialisation helpers.
///
/// Provides:
///   - File size validation (max 100 MiB).
///   - Length-prefixed binary field encoding / decoding
///     (4-byte big-endian length followed by raw data).

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace security {

// ── Constants ────────────────────────────────────────────────────────────────

/// Maximum allowed database file size (100 MiB).
inline constexpr std::size_t kMaxFileSize = 100ULL * 1024 * 1024;

// ── Exceptions ───────────────────────────────────────────────────────────────

/// Thrown when a security policy is violated (file too large, etc.).
struct PolicyViolation : std::runtime_error {
    using std::runtime_error::runtime_error;
};

/// Thrown when binary deserialisation encounters malformed data.
struct DeserialisationError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// ── File size validation ─────────────────────────────────────────────────────

/// Check that file at @p path does not exceed kMaxFileSize.
/// @throws PolicyViolation if the file is too large.
/// @throws std::runtime_error if the file cannot be stat'd.
void validate_file_size(const std::string& path);

/// Check that a byte count does not exceed kMaxFileSize.
/// @throws PolicyViolation if @p size > kMaxFileSize.
void validate_blob_size(std::size_t size);

// ── Binary field encoding ────────────────────────────────────────────────────
//
// Wire format per field:
//   [length : 4 bytes, big-endian] [data : <length> bytes]

/// Append a length-prefixed field to @p out.
void write_field(std::vector<uint8_t>& out, const std::string& value);

/// Append a length-prefixed field (raw bytes) to @p out.
void write_field(std::vector<uint8_t>& out, const std::vector<uint8_t>& value);

/// Read a length-prefixed string field starting at @p offset.
/// Advances @p offset past the consumed bytes.
/// @throws DeserialisationError on truncation / overflow.
[[nodiscard]]
std::string read_string_field(const std::vector<uint8_t>& buf,
                              std::size_t& offset);

/// Read a length-prefixed raw-bytes field starting at @p offset.
/// Advances @p offset past the consumed bytes.
/// @throws DeserialisationError on truncation / overflow.
[[nodiscard]]
std::vector<uint8_t> read_bytes_field(const std::vector<uint8_t>& buf,
                                      std::size_t& offset);

}  // namespace security
