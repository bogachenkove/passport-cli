#include "database.h"
#include "crypto.h"
#include "security.h"

#include <chrono>
#include <cstring>
#include <fstream>
#include <stdexcept>

// ── Byte-order helpers (big-endian on disk) ──────────────────────────────────

namespace {

void write_u32_be(uint8_t* dst, uint32_t v) {
    dst[0] = static_cast<uint8_t>((v >> 24) & 0xFF);
    dst[1] = static_cast<uint8_t>((v >> 16) & 0xFF);
    dst[2] = static_cast<uint8_t>((v >>  8) & 0xFF);
    dst[3] = static_cast<uint8_t>((v >>  0) & 0xFF);
}

uint32_t read_u32_be(const uint8_t* src) {
    return (static_cast<uint32_t>(src[0]) << 24)
         | (static_cast<uint32_t>(src[1]) << 16)
         | (static_cast<uint32_t>(src[2]) <<  8)
         | (static_cast<uint32_t>(src[3]) <<  0);
}

void write_u64_be(uint8_t* dst, uint64_t v) {
    for (int i = 7; i >= 0; --i) {
        dst[i] = static_cast<uint8_t>(v & 0xFF);
        v >>= 8;
    }
}

uint64_t read_u64_be(const uint8_t* src) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) {
        v = (v << 8) | static_cast<uint64_t>(src[i]);
    }
    return v;
}

uint64_t unix_timestamp_now() {
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

}  // anonymous namespace

// ── Binary serialisation (length-prefixed fields) ────────────────────────────

std::vector<uint8_t> PasswordDatabase::serialize_records() const {
    std::vector<uint8_t> buf;

    for (const auto& r : records_) {
        security::write_field(buf, r.login);
        security::write_field(buf, r.password);
        security::write_field(buf, r.url);
        security::write_field(buf, r.note);
    }

    return buf;
}

void PasswordDatabase::deserialize_records(
    const std::vector<uint8_t>& plaintext)
{
    records_.clear();

    if (plaintext.empty()) {
        return;
    }

    std::size_t offset = 0;

    while (offset < plaintext.size()) {
        PasswordRecord rec;
        rec.login    = security::read_string_field(plaintext, offset);
        rec.password = security::read_string_field(plaintext, offset);
        rec.url      = security::read_string_field(plaintext, offset);
        rec.note     = security::read_string_field(plaintext, offset);
        records_.push_back(std::move(rec));
    }
}

// ── File I/O ─────────────────────────────────────────────────────────────────

bool PasswordDatabase::load_from_file(
    const std::string& file_path,
    const std::string& master_password)
{
    // Validate file size before reading.
    security::validate_file_size(file_path);

    std::ifstream in(file_path, std::ios::binary);
    if (!in.is_open()) {
        return false;
    }

    const std::vector<uint8_t> blob(
        (std::istreambuf_iterator<char>(in)),
         std::istreambuf_iterator<char>());
    in.close();

    // Double-check the in-memory blob size as well.
    security::validate_blob_size(blob.size());

    // Minimum: header (64) + payload_length (4) + at least AEAD tag (16)
    constexpr std::size_t kMinFileSize =
        kHeaderAdSize + 4 + crypto::kAeadTagBytes;
    if (blob.size() < kMinFileSize) {
        throw std::runtime_error{"Database file is too small or truncated."};
    }

    std::size_t off = 0;

    // ── Magic ────────────────────────────────────────────────────────────
    if (std::memcmp(blob.data() + off, kFileMagic, 4) != 0) {
        throw std::runtime_error{"Not a valid password database file."};
    }
    off += 4;

    // ── Salt (16 bytes for Argon2id) ─────────────────────────────────────
    std::vector<uint8_t> salt(blob.begin() + off,
                              blob.begin() + off + crypto::kSaltBytes);
    off += crypto::kSaltBytes;

    // ── Nonce (24 bytes for XChaCha20-Poly1305) ──────────────────────────
    std::vector<uint8_t> nonce(blob.begin() + off,
                               blob.begin() + off + crypto::kAeadNonceBytes);
    off += crypto::kAeadNonceBytes;

    // ── Timestamps ───────────────────────────────────────────────────────
    const uint64_t stored_ts_created  = read_u64_be(blob.data() + off); off += 8;
    const uint64_t stored_ts_modified = read_u64_be(blob.data() + off); off += 8;

    // ── Record count (informational, skip) ───────────────────────────────
    off += 4;

    // Sanity: we must now be at kHeaderAdSize.
    // magic(4) + salt(16) + nonce(24) + ts(8+8) + cnt(4) = 64

    // ── Payload length ───────────────────────────────────────────────────
    const uint32_t payload_len = read_u32_be(blob.data() + off);
    off += 4;

    if (off + payload_len > blob.size()) {
        throw std::runtime_error{"Payload extends beyond end of file."};
    }

    // ── Derive key (Argon2id) ────────────────────────────────────────────
    const auto derived_key = crypto::derive_key(master_password, salt);

    // Associated data = entire header (first kHeaderAdSize bytes).
    const std::vector<uint8_t> ad(blob.begin(), blob.begin() + kHeaderAdSize);

    // ── Encrypted payload ────────────────────────────────────────────────
    const std::vector<uint8_t> encrypted(blob.begin() + off,
                                         blob.begin() + off + payload_len);

    // Decrypt (throws crypto::AeadError on bad password / tamper).
    const auto decrypted = crypto::aead_decrypt(encrypted, ad, nonce, derived_key);

    deserialize_records(decrypted);

    ts_created_  = stored_ts_created;
    ts_modified_ = stored_ts_modified;

    return true;
}

bool PasswordDatabase::save_to_file(
    const std::string& file_path,
    const std::string& master_password)
{
    auto salt  = crypto::random_bytes(crypto::kSaltBytes);
    auto nonce = crypto::random_bytes(crypto::kAeadNonceBytes);

    const auto derived_key = crypto::derive_key(master_password, salt);

    const uint64_t now = unix_timestamp_now();
    if (ts_created_ == 0) {
        ts_created_ = now;
    }
    ts_modified_ = now;

    // ── Build header (= associated data) ─────────────────────────────────
    std::vector<uint8_t> header;
    header.reserve(kHeaderAdSize);

    // Magic
    header.insert(header.end(), kFileMagic, kFileMagic + 4);

    // Salt
    header.insert(header.end(), salt.begin(), salt.end());

    // Nonce
    header.insert(header.end(), nonce.begin(), nonce.end());

    // Timestamps
    uint8_t ts_buf[8];
    write_u64_be(ts_buf, ts_created_);
    header.insert(header.end(), ts_buf, ts_buf + 8);

    write_u64_be(ts_buf, ts_modified_);
    header.insert(header.end(), ts_buf, ts_buf + 8);

    // Record count
    uint8_t cnt_buf[4];
    write_u32_be(cnt_buf, static_cast<uint32_t>(records_.size()));
    header.insert(header.end(), cnt_buf, cnt_buf + 4);

    // ── Encrypt ──────────────────────────────────────────────────────────
    const auto plaintext      = serialize_records();
    const auto ciphertext_tag = crypto::aead_encrypt(
        plaintext, header, nonce, derived_key);

    // ── Assemble output ──────────────────────────────────────────────────
    std::vector<uint8_t> out;
    out.reserve(kHeaderAdSize + 4 + ciphertext_tag.size());

    out.insert(out.end(), header.begin(), header.end());

    uint8_t plen_buf[4];
    write_u32_be(plen_buf, static_cast<uint32_t>(ciphertext_tag.size()));
    out.insert(out.end(), plen_buf, plen_buf + 4);

    out.insert(out.end(), ciphertext_tag.begin(), ciphertext_tag.end());

    // ── Validate output size before writing ──────────────────────────────
    security::validate_blob_size(out.size());

    // ── Write ────────────────────────────────────────────────────────────
    std::ofstream ofs(file_path, std::ios::binary | std::ios::trunc);
    if (!ofs.is_open()) {
        return false;
    }
    ofs.write(reinterpret_cast<const char*>(out.data()),
              static_cast<std::streamsize>(out.size()));
    return ofs.good();
}

// ── CRUD ─────────────────────────────────────────────────────────────────────

void PasswordDatabase::add_record(const PasswordRecord& record) {
    records_.push_back(record);
}

bool PasswordDatabase::remove_record(std::size_t index) {
    if (index >= records_.size()) {
        return false;
    }
    records_.erase(records_.begin() + static_cast<std::ptrdiff_t>(index));
    return true;
}

const std::vector<PasswordRecord>& PasswordDatabase::records() const noexcept {
    return records_;
}

std::size_t PasswordDatabase::record_count() const noexcept {
    return records_.size();
}

uint64_t PasswordDatabase::timestamp_created() const noexcept {
    return ts_created_;
}

uint64_t PasswordDatabase::timestamp_modified() const noexcept {
    return ts_modified_;
}
