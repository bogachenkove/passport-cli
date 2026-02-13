#include "file_database.hpp"
#include "binary_serializer.hpp"
#include "file_utils.hpp"
#include "../core/constants.hpp"
#include "../core/endian.hpp"
#include "../core/errors.hpp"
#include <chrono>
#include <cstring>
#include <fstream>

namespace security::storage {

	FileDatabase::FileDatabase(std::shared_ptr<domain::interfaces::ICryptoService> crypto)
		: crypto_(std::move(crypto))
	{
	}

	std::uint64_t FileDatabase::unix_timestamp_now() {
		using namespace std::chrono;
		return static_cast<std::uint64_t>(
			duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
	}

	std::vector<std::uint8_t> FileDatabase::serialize_records() const {
		std::vector<std::uint8_t> buf;
		const auto& seed = type_system_.master_seed();
		const auto& salt = type_system_.context_salt();
		buf.insert(buf.end(), seed.begin(), seed.end());
		buf.insert(buf.end(), salt.begin(), salt.end());

		const auto pw_type = type_system_.password_type();
		const auto note_type = type_system_.note_type();

		for (const auto& r : password_records_) {
			buf.insert(buf.end(), pw_type.begin(), pw_type.end());
			core::endian::append_u64_be(buf, r.date);
			filesystem::storage::binary_serializer::write_field(buf, r.login);
			filesystem::storage::binary_serializer::write_field(buf, r.password);
			filesystem::storage::binary_serializer::write_field(buf, r.url);
			filesystem::storage::binary_serializer::write_field(buf, r.note);
		}

		for (const auto& r : note_records_) {
			buf.insert(buf.end(), note_type.begin(), note_type.end());
			core::endian::append_u64_be(buf, r.date);
			filesystem::storage::binary_serializer::write_field(buf, r.title);
			filesystem::storage::binary_serializer::write_field(buf, r.note);
		}
		return buf;
	}

	void FileDatabase::deserialize_records(const std::vector<std::uint8_t>& plaintext) {
		password_records_.clear();
		note_records_.clear();

		if (plaintext.empty()) {
			type_system_.generate_seeds(*crypto_);
			return;
		}

		constexpr std::size_t kSeedBlock = core::constants::kMasterSeedBytes +
			core::constants::kContextSaltBytes;
		if (plaintext.size() < kSeedBlock) {
			throw core::errors::DeserialisationError{
				"Encrypted payload too short to contain type-system seeds."
			};
		}

		std::size_t offset = 0;
		std::vector<std::uint8_t> master_seed(
			plaintext.begin() + offset,
			plaintext.begin() + offset + core::constants::kMasterSeedBytes);
		offset += core::constants::kMasterSeedBytes;
		std::vector<std::uint8_t> context_salt(
			plaintext.begin() + offset,
			plaintext.begin() + offset + core::constants::kContextSaltBytes);
		offset += core::constants::kContextSaltBytes;

		type_system_ = security::crypto::TypeSystem(master_seed, context_salt);

		const auto pw_type = type_system_.password_type();
		const auto note_type = type_system_.note_type();

		while (offset < plaintext.size()) {
			if (offset + 32 > plaintext.size()) {
				throw core::errors::DeserialisationError{
					"Truncated record: not enough bytes for type identifier."
				};
			}
			domain::models::RecordType tag;
			std::memcpy(tag.data(), plaintext.data() + offset, tag.size());
			offset += tag.size();

			if (sodium_memcmp(tag.data(), pw_type.data(), tag.size()) == 0) {
				if (offset + 8 > plaintext.size()) {
					throw core::errors::DeserialisationError{
						"Truncated PasswordRecord: not enough bytes for date field."
					};
				}
				domain::models::PasswordRecord rec;
				rec.date = core::endian::read_u64_be(plaintext.data() + offset);
				offset += 8;

				rec.login = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.password = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.url = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.note = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);

				password_records_.push_back(std::move(rec));
			}
			else if (sodium_memcmp(tag.data(), note_type.data(), tag.size()) == 0) {
				if (offset + 8 > plaintext.size()) {
					throw core::errors::DeserialisationError{
						"Truncated NoteRecord: not enough bytes for date field."
					};
				}
				domain::models::NoteRecord rec;
				rec.date = core::endian::read_u64_be(plaintext.data() + offset);
				offset += 8;

				rec.title = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.note = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);

				note_records_.push_back(std::move(rec));
			}
			else {
				throw core::errors::DeserialisationError{
					"Unknown record type identifier Ч database may be corrupted or "
					"created by an incompatible version."
				};
			}
		}
	}

	bool FileDatabase::load_from_file(const std::string& file_path,
		const std::string& master_password) {
		if (!filesystem::storage::check_file_access(file_path, false)) {
			throw core::errors::FileAccessError{
				"Cannot read from file. Check permissions: " + file_path
			};
		}
		filesystem::storage::validate_file_size(file_path);

		std::ifstream in(file_path, std::ios::binary);
		if (!in.is_open()) return false;

		std::vector<std::uint8_t> blob(
			(std::istreambuf_iterator<char>(in)),
			std::istreambuf_iterator<char>());
		in.close();

		filesystem::storage::validate_blob_size(blob.size());

		constexpr std::size_t kMinFileSize = core::constants::kHeaderAdSize + 4 +
			core::constants::kAeadTagBytes;
		if (blob.size() < kMinFileSize) {
			throw core::errors::DeserialisationError{
				"Database file is too small or truncated."
			};
		}

		std::size_t off = 0;
		if (std::memcmp(blob.data() + off, core::constants::kFileMagic, 4) != 0) {
			throw core::errors::DeserialisationError{
				"Not a valid password database file."
			};
		}
		off += 4;

		std::vector<std::uint8_t> salt(blob.begin() + off,
			blob.begin() + off + core::constants::kSaltBytes);
		off += core::constants::kSaltBytes;

		std::vector<std::uint8_t> nonce(blob.begin() + off,
			blob.begin() + off + core::constants::kAeadNonceBytes);
		off += core::constants::kAeadNonceBytes;

		std::uint64_t stored_created = core::endian::read_u64_be(blob.data() + off);
		off += 8;
		std::uint64_t stored_modified = core::endian::read_u64_be(blob.data() + off);
		off += 8;
		off += 4; // пропускаем счЄтчик записей (не используетс€)

		std::uint32_t payload_len = core::endian::read_u32_be(blob.data() + off);
		off += 4;

		if (off + payload_len > blob.size()) {
			throw core::errors::DeserialisationError{
				"Payload extends beyond end of file."
			};
		}

		auto derived_key = crypto_->derive_key(master_password, salt);
		std::vector<std::uint8_t> ad(blob.begin(), blob.begin() + core::constants::kHeaderAdSize);
		std::vector<std::uint8_t> encrypted(blob.begin() + off,
			blob.begin() + off + payload_len);

		auto decrypted = crypto_->aead_decrypt(encrypted, ad, nonce, derived_key);
		deserialize_records(decrypted);

		ts_created_ = stored_created;
		ts_modified_ = stored_modified;
		return true;
	}

	bool FileDatabase::save_to_file(const std::string& file_path,
		const std::string& master_password) {
		if (!filesystem::storage::check_file_access(file_path, true)) {
			throw core::errors::FileAccessError{
				"Cannot write to file. Check permissions: " + file_path
			};
		}

		if (!type_system_.is_initialised()) {
			type_system_.generate_seeds(*crypto_);
		}

		auto salt = crypto_->random_bytes(core::constants::kSaltBytes);
		auto nonce = crypto_->random_bytes(core::constants::kAeadNonceBytes);
		auto derived_key = crypto_->derive_key(master_password, salt);

		std::uint64_t now = unix_timestamp_now();
		if (ts_created_ == 0) ts_created_ = now;
		ts_modified_ = now;

		std::vector<std::uint8_t> header;
		header.reserve(core::constants::kHeaderAdSize);
		header.insert(header.end(), core::constants::kFileMagic,
			core::constants::kFileMagic + 4);
		header.insert(header.end(), salt.begin(), salt.end());
		header.insert(header.end(), nonce.begin(), nonce.end());

		core::endian::append_u64_be(header, ts_created_);
		core::endian::append_u64_be(header, ts_modified_);
		core::endian::append_u32_be(header, static_cast<std::uint32_t>(record_count()));

		auto plaintext = serialize_records();
		auto ciphertext_tag = crypto_->aead_encrypt(plaintext, header, nonce, derived_key);

		std::vector<std::uint8_t> out;
		out.reserve(header.size() + 4 + ciphertext_tag.size());
		out.insert(out.end(), header.begin(), header.end());
		core::endian::append_u32_be(out, static_cast<std::uint32_t>(ciphertext_tag.size()));
		out.insert(out.end(), ciphertext_tag.begin(), ciphertext_tag.end());

		filesystem::storage::validate_blob_size(out.size());

		std::ofstream ofs(file_path, std::ios::binary | std::ios::trunc);
		if (!ofs.is_open()) return false;
		ofs.write(reinterpret_cast<const char*>(out.data()),
			static_cast<std::streamsize>(out.size()));
		return ofs.good();
	}

	void FileDatabase::add_password_record(domain::models::PasswordRecord record) {
		if (record.date == 0) record.date = unix_timestamp_now();
		password_records_.push_back(std::move(record));
	}

	bool FileDatabase::remove_password_record(std::size_t index) {
		if (index >= password_records_.size()) return false;
		password_records_.erase(password_records_.begin() +
			static_cast<std::ptrdiff_t>(index));
		return true;
	}

	const std::vector<domain::models::PasswordRecord>&
		FileDatabase::password_records() const noexcept {
		return password_records_;
	}

	std::size_t FileDatabase::password_record_count() const noexcept {
		return password_records_.size();
	}

	void FileDatabase::add_note_record(domain::models::NoteRecord record) {
		if (record.date == 0) record.date = unix_timestamp_now();
		note_records_.push_back(std::move(record));
	}

	bool FileDatabase::remove_note_record(std::size_t index) {
		if (index >= note_records_.size()) return false;
		note_records_.erase(note_records_.begin() +
			static_cast<std::ptrdiff_t>(index));
		return true;
	}

	const std::vector<domain::models::NoteRecord>&
		FileDatabase::note_records() const noexcept {
		return note_records_;
	}

	std::size_t FileDatabase::note_record_count() const noexcept {
		return note_records_.size();
	}

	std::size_t FileDatabase::record_count() const noexcept {
		return password_records_.size() + note_records_.size();
	}

	std::uint64_t FileDatabase::timestamp_created() const noexcept {
		return ts_created_;
	}

	std::uint64_t FileDatabase::timestamp_modified() const noexcept {
		return ts_modified_;
	}

} // namespace security::storage