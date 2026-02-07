#pragma once

/// @file database.h
/// @brief Encrypted password database — file format, serialisation, CRUD.
///
/// File layout (XChaCha20-Poly1305 + Argon2id):
///   [magic 4B]
///   [salt 16B]
///   [nonce 24B]
///   [ts_created 8B]
///   [ts_modified 8B]
///   [record_count 4B]
///   [payload_length 4B]
///   [encrypted_payload ...]
///
/// Header up to (and including) record_count is used as AEAD associated data.
///
/// Record payload (binary, per record):
///   [login_len 4B][login_data ...]
///   [password_len 4B][password_data ...]
///   [url_len 4B][url_data ...]
///   [note_len 4B][note_data ...]

#include <array>
#include <cstdint>
#include <string>
#include <vector>

// ── Password record ──────────────────────────────────────────────────────────

/// A single stored credential.
struct PasswordRecord {
    std::string login;
    std::string password;
    std::string url;
    std::string note;
};

// ── Database ─────────────────────────────────────────────────────────────────

/// Manages an in-memory collection of PasswordRecord objects and
/// persists them to an encrypted file.
class PasswordDatabase {
public:
    PasswordDatabase() = default;

    // ── File I/O ─────────────────────────────────────────────────────────

    /// Load and decrypt a database file.
    /// @throws std::runtime_error / security::PolicyViolation /
    ///         crypto::AeadError on format / crypto / policy errors.
    /// @returns false only if the file cannot be opened.
    bool load_from_file(const std::string& file_path,
                        const std::string& master_password);

    /// Encrypt and save the database.
    /// @throws std::runtime_error / security::PolicyViolation /
    ///         crypto::AeadError on crypto / policy errors.
    /// @returns false only if the file cannot be written.
    bool save_to_file(const std::string& file_path,
                      const std::string& master_password);

    // ── CRUD ─────────────────────────────────────────────────────────────

    void   add_record(const PasswordRecord& record);
    bool   remove_record(std::size_t index);

    [[nodiscard]] const std::vector<PasswordRecord>& records()      const noexcept;
    [[nodiscard]] std::size_t                         record_count() const noexcept;

    // ── Timestamps ───────────────────────────────────────────────────────

    [[nodiscard]] uint64_t timestamp_created()  const noexcept;
    [[nodiscard]] uint64_t timestamp_modified() const noexcept;

private:
    // ── Binary serialisation (length-prefixed fields) ────────────────────

    [[nodiscard]]
    std::vector<uint8_t> serialize_records() const;

    void deserialize_records(const std::vector<uint8_t>& plaintext);

    // ── File format constants ────────────────────────────────────────────

    static constexpr uint8_t kFileMagic[4] = {'P', 'W', 'D', 'B'};

    /// Header size (= associated-data region):
    ///   magic(4) + salt(16) + nonce(24)
    ///   + ts_created(8) + ts_modified(8) + record_count(4) = 64
    static constexpr std::size_t kHeaderAdSize = 64;

    // ── State ────────────────────────────────────────────────────────────

    std::vector<PasswordRecord> records_;
    mutable uint64_t ts_created_  = 0;
    mutable uint64_t ts_modified_ = 0;
};
