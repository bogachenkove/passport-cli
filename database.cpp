#include "database.hpp"
#include "crypto.hpp"
#include "security.hpp"
#include "types.hpp"

#include <chrono>
#include <cstring>
#include <fstream>
#include <stdexcept>

namespace {
	void write_u32_be(uint8_t* dst, uint32_t v) {
		dst[0] = static_cast<uint8_t>((v >> 24) & 0xFF);
		dst[1] = static_cast<uint8_t>((v >> 16) & 0xFF);
		dst[2] = static_cast<uint8_t>((v >> 8) & 0xFF);
		dst[3] = static_cast<uint8_t>((v >> 0) & 0xFF);
	}
	uint32_t read_u32_be(const uint8_t* src) {
		return (static_cast<uint32_t>(src[0]) << 24)
			| (static_cast<uint32_t>(src[1]) << 16)
			| (static_cast<uint32_t>(src[2]) << 8)
			| (static_cast<uint32_t>(src[3]) << 0);
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
	void append_raw(std::vector<uint8_t>& out, const uint8_t* data, std::size_t len) {
		out.insert(out.end(), data, data + len);
	}
	void read_raw(const std::vector<uint8_t>& buf,
		std::size_t& offset,
		uint8_t* dst,
		std::size_t len)
	{
		if (offset + len > buf.size()) {
			throw security::DeserialisationError{
				"Truncated data: expected " + std::to_string(len)
				+ " bytes at offset " + std::to_string(offset) + "."
			};
		}
		std::memcpy(dst, buf.data() + offset, len);
		offset += len;
	}
} std::vector<uint8_t> PassportDatabase::serialize_records() const {
	std::vector<uint8_t> buf;
	const auto& seed = type_system_.master_seed();
	const auto& salt = type_system_.context_salt();
	append_raw(buf, seed.data(), seed.size());
	append_raw(buf, salt.data(), salt.size());
	const auto pw_type = type_system_.password_type();
	const auto note_type = type_system_.note_type();
	for (const auto& r : password_records_) {
		append_raw(buf, pw_type.data(), pw_type.size());
		uint8_t pw_date_buf[8] = {};
		write_u64_be(pw_date_buf, r.date);
		append_raw(buf, pw_date_buf, 8);
		security::write_field(buf, r.login);
		security::write_field(buf, r.password);
		security::write_field(buf, r.url);
		security::write_field(buf, r.note);
	}
	for (const auto& r : note_records_) {
		append_raw(buf, note_type.data(), note_type.size());
		uint8_t date_buf[8] = {};
		write_u64_be(date_buf, r.date);
		append_raw(buf, date_buf, 8);
		security::write_field(buf, r.title);
		security::write_field(buf, r.note);
	}
	return buf;
}
void PassportDatabase::deserialize_records(
	const std::vector<uint8_t>& plaintext)
{
	password_records_.clear();
	note_records_.clear();
	if (plaintext.empty()) {
		type_system_.generate_seeds();
		return;
	}
	constexpr std::size_t kSeedBlock = types::kMasterSeedBytes + types::kContextSaltBytes;
	if (plaintext.size() < kSeedBlock) {
		throw security::DeserialisationError{
			"Encrypted payload too short to contain type-system seeds."
		};
	}
	std::size_t offset = 0;
	std::vector<uint8_t> master_seed(types::kMasterSeedBytes);
	read_raw(plaintext, offset, master_seed.data(), types::kMasterSeedBytes);
	std::vector<uint8_t> context_salt(types::kContextSaltBytes);
	read_raw(plaintext, offset, context_salt.data(), types::kContextSaltBytes);
	type_system_ = types::TypeSystem(master_seed, context_salt);
	const auto pw_type = type_system_.password_type();
	const auto note_type = type_system_.note_type();
	while (offset < plaintext.size()) {
		if (offset + 32 > plaintext.size()) {
			throw security::DeserialisationError{
				"Truncated record: not enough bytes for type identifier."
			};
		}
		types::RecordType tag{};
		read_raw(plaintext, offset, tag.data(), tag.size());
		if (types::record_type_equal(tag, pw_type)) {
			if (offset + 8 > plaintext.size()) {
				throw security::DeserialisationError{
					"Truncated PasswordRecord: not enough bytes for date field."
				};
			}
			PasswordRecord rec;
			uint8_t pw_date_buf[8];
			read_raw(plaintext, offset, pw_date_buf, 8);
			rec.date = read_u64_be(pw_date_buf);
			rec.login = security::read_string_field(plaintext, offset);
			rec.password = security::read_string_field(plaintext, offset);
			rec.url = security::read_string_field(plaintext, offset);
			rec.note = security::read_string_field(plaintext, offset);
			password_records_.push_back(std::move(rec));
		}
		else if (types::record_type_equal(tag, note_type)) {
			if (offset + 8 > plaintext.size()) {
				throw security::DeserialisationError{
					"Truncated NoteRecord: not enough bytes for date field."
				};
			}
			NoteRecord rec;
			uint8_t date_buf[8];
			read_raw(plaintext, offset, date_buf, 8);
			rec.date = read_u64_be(date_buf);
			rec.title = security::read_string_field(plaintext, offset);
			rec.note = security::read_string_field(plaintext, offset);
			note_records_.push_back(std::move(rec));
		}
		else {
			throw security::DeserialisationError{
				"Unknown record type identifier — database may be "
				"corrupted or created by an incompatible version."
			};
		}
	}
}
bool PassportDatabase::load_from_file(
	const std::string& file_path,
	const std::string& master_password)
{
	if (!security::check_file_access(file_path, false)) {
		throw std::runtime_error{
			"Cannot read from file. Check permissions: " + file_path
		};
	}
	security::validate_file_size(file_path);
	std::ifstream in(file_path, std::ios::binary);
	if (!in.is_open()) {
		return false;
	}
	const std::vector<uint8_t> blob(
		(std::istreambuf_iterator<char>(in)),
		std::istreambuf_iterator<char>());
	in.close();
	security::validate_blob_size(blob.size());
	constexpr std::size_t kMinFileSize =
		kHeaderAdSize + 4 + crypto::kAeadTagBytes;
	if (blob.size() < kMinFileSize) {
		throw std::runtime_error{
			"Database file is too small or truncated."
		};
	}
	std::size_t off = 0;
	if (std::memcmp(blob.data() + off, kFileMagic, 4) != 0) {
		throw std::runtime_error{
			"Not a valid password database file."
		};
	}
	off += 4;
	std::vector<uint8_t> salt(blob.begin() + off,
		blob.begin() + off + crypto::kSaltBytes);
	off += crypto::kSaltBytes;
	std::vector<uint8_t> nonce(blob.begin() + off,
		blob.begin() + off + crypto::kAeadNonceBytes);
	off += crypto::kAeadNonceBytes;
	const uint64_t stored_ts_created = read_u64_be(blob.data() + off);
	off += 8;
	const uint64_t stored_ts_modified = read_u64_be(blob.data() + off);
	off += 8;
	off += 4;
	const uint32_t payload_len = read_u32_be(blob.data() + off);
	off += 4;
	if (off + payload_len > blob.size()) {
		throw std::runtime_error{
			"Payload extends beyond end of file."
		};
	}
	const auto derived_key = crypto::derive_key(master_password, salt);
	const std::vector<uint8_t> ad(blob.begin(), blob.begin() + kHeaderAdSize);
	const std::vector<uint8_t> encrypted(blob.begin() + off,
		blob.begin() + off + payload_len);
	const auto decrypted = crypto::aead_decrypt(encrypted, ad, nonce, derived_key);
	deserialize_records(decrypted);
	ts_created_ = stored_ts_created;
	ts_modified_ = stored_ts_modified;
	return true;
}
bool PassportDatabase::save_to_file(
	const std::string& file_path,
	const std::string& master_password)
{
	if (!security::check_file_access(file_path, true)) {
		throw std::runtime_error{
			"Cannot write to file. Check permissions: " + file_path
		};
	}
	if (!type_system_.is_initialised()) {
		type_system_.generate_seeds();
	}
	auto salt = crypto::random_bytes(crypto::kSaltBytes);
	auto nonce = crypto::random_bytes(crypto::kAeadNonceBytes);
	const auto derived_key = crypto::derive_key(master_password, salt);
	const uint64_t now = unix_timestamp_now();
	if (ts_created_ == 0) {
		ts_created_ = now;
	}
	ts_modified_ = now;
	std::vector<uint8_t> header;
	header.reserve(kHeaderAdSize);
	header.insert(header.end(), kFileMagic, kFileMagic + 4);
	header.insert(header.end(), salt.begin(), salt.end());
	header.insert(header.end(), nonce.begin(), nonce.end());
	uint8_t ts_buf[8];
	write_u64_be(ts_buf, ts_created_);
	header.insert(header.end(), ts_buf, ts_buf + 8);
	write_u64_be(ts_buf, ts_modified_);
	header.insert(header.end(), ts_buf, ts_buf + 8);
	uint8_t cnt_buf[4];
	write_u32_be(cnt_buf, static_cast<uint32_t>(record_count()));
	header.insert(header.end(), cnt_buf, cnt_buf + 4);
	const auto plaintext = serialize_records();
	const auto ciphertext_tag = crypto::aead_encrypt(plaintext, header, nonce, derived_key);
	std::vector<uint8_t> out;
	out.reserve(kHeaderAdSize + 4 + ciphertext_tag.size());
	out.insert(out.end(), header.begin(), header.end());
	uint8_t plen_buf[4];
	write_u32_be(plen_buf, static_cast<uint32_t>(ciphertext_tag.size()));
	out.insert(out.end(), plen_buf, plen_buf + 4);
	out.insert(out.end(), ciphertext_tag.begin(), ciphertext_tag.end());
	security::validate_blob_size(out.size());
	std::ofstream ofs(file_path, std::ios::binary | std::ios::trunc);
	if (!ofs.is_open()) {
		return false;
	}
	ofs.write(reinterpret_cast<const char*>(out.data()),
		static_cast<std::streamsize>(out.size()));
	return ofs.good();
}
void PassportDatabase::add_password_record(PasswordRecord record) {
	if (record.date == 0) {
		record.date = unix_timestamp_now();
	}
	password_records_.push_back(std::move(record));
}
bool PassportDatabase::remove_password_record(std::size_t index) {
	if (index >= password_records_.size()) {
		return false;
	}
	password_records_.erase(
		password_records_.begin() + static_cast<std::ptrdiff_t>(index));
	return true;
}
const std::vector<PasswordRecord>& PassportDatabase::password_records() const noexcept {
	return password_records_;
}

std::size_t PassportDatabase::password_record_count() const noexcept {
	return password_records_.size();
}
void PassportDatabase::add_note_record(NoteRecord record) {
	if (record.date == 0) {
		record.date = unix_timestamp_now();
	}
	note_records_.push_back(std::move(record));
}
bool PassportDatabase::remove_note_record(std::size_t index) {
	if (index >= note_records_.size()) {
		return false;
	}
	note_records_.erase(
		note_records_.begin() + static_cast<std::ptrdiff_t>(index));
	return true;
}
const std::vector<NoteRecord>& PassportDatabase::note_records() const noexcept {
	return note_records_;
}
std::size_t PassportDatabase::note_record_count() const noexcept {
	return note_records_.size();
}
std::size_t PassportDatabase::record_count() const noexcept {
	return password_records_.size() + note_records_.size();
}
uint64_t PassportDatabase::timestamp_created() const noexcept {
	return ts_created_;
}
uint64_t PassportDatabase::timestamp_modified() const noexcept {
	return ts_modified_;
}
const types::TypeSystem& PassportDatabase::type_system() const noexcept {
	return type_system_;
}