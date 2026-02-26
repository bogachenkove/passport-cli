#include "file_database.hpp"
#include "file_utils.hpp"
#include "binary_serializer.hpp"
#include "../core/endian.hpp"
#include "../core/constants.hpp"
#include "../core/errors.hpp"
#include <chrono>
#include <cstring>
#include <fstream>
#include <sodium.h>
#include <vector>
#include <string>
#include <algorithm>

namespace security::storage {
	FileDatabase::FileDatabase(std::shared_ptr<domain::interfaces::ICryptoService> crypto) : crypto_(std::move(crypto)) {
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
		const auto bank_type = type_system_.bankcard_type();
		const auto disc_type = type_system_.discountcard_type();
		const auto trans_type = type_system_.transportcard_type();
		const auto mnem_type = type_system_.mnemonic_type();
		const auto wifi_type = type_system_.wifi_type();
		const auto key_type = type_system_.key_type();
		for (const auto& r : password_records_) {
			std::vector<std::uint8_t> record_data;
			core::endian::append_u64_be(record_data, r.date);
			filesystem::storage::binary_serializer::write_field(record_data, r.login);
			filesystem::storage::binary_serializer::write_field(record_data, r.password);
			filesystem::storage::binary_serializer::write_field(record_data, r.url);
			filesystem::storage::binary_serializer::write_field(record_data, r.note);
			uint32_t total_len = 4 + 32 + static_cast<uint32_t>(record_data.size());
			core::endian::append_u32_be(buf, total_len);
			buf.insert(buf.end(), pw_type.begin(), pw_type.end());
			buf.insert(buf.end(), record_data.begin(), record_data.end());
		}
		for (const auto& r : note_records_) {
			std::vector<std::uint8_t> record_data;
			core::endian::append_u64_be(record_data, r.date);
			filesystem::storage::binary_serializer::write_field(record_data, r.title);
			filesystem::storage::binary_serializer::write_field(record_data, r.note);
			uint32_t total_len = 4 + 32 + static_cast<uint32_t>(record_data.size());
			core::endian::append_u32_be(buf, total_len);
			buf.insert(buf.end(), note_type.begin(), note_type.end());
			buf.insert(buf.end(), record_data.begin(), record_data.end());
		}
		for (const auto& r : bankcard_records_) {
			std::vector<std::uint8_t> record_data;
			core::endian::append_u64_be(record_data, r.date);
			filesystem::storage::binary_serializer::write_field(record_data, r.card_number);
			filesystem::storage::binary_serializer::write_field(record_data, r.expiry_date);
			filesystem::storage::binary_serializer::write_field(record_data, r.cvv);
			filesystem::storage::binary_serializer::write_field(record_data, r.cardholder_name);
			filesystem::storage::binary_serializer::write_field(record_data, r.note);
			uint32_t total_len = 4 + 32 + static_cast<uint32_t>(record_data.size());
			core::endian::append_u32_be(buf, total_len);
			buf.insert(buf.end(), bank_type.begin(), bank_type.end());
			buf.insert(buf.end(), record_data.begin(), record_data.end());
		}
		for (const auto& r : discount_records_) {
			std::vector<std::uint8_t> record_data;
			core::endian::append_u64_be(record_data, r.date);
			filesystem::storage::binary_serializer::write_field(record_data, r.card_number);
			filesystem::storage::binary_serializer::write_field(record_data, r.barcode);
			filesystem::storage::binary_serializer::write_field(record_data, r.cvv);
			filesystem::storage::binary_serializer::write_field(record_data, r.store_name);
			filesystem::storage::binary_serializer::write_field(record_data, r.note);
			uint32_t total_len = 4 + 32 + static_cast<uint32_t>(record_data.size());
			core::endian::append_u32_be(buf, total_len);
			buf.insert(buf.end(), disc_type.begin(), disc_type.end());
			buf.insert(buf.end(), record_data.begin(), record_data.end());
		}
		for (const auto& r : transport_records_) {
			std::vector<std::uint8_t> record_data;
			core::endian::append_u64_be(record_data, r.date);
			filesystem::storage::binary_serializer::write_field(record_data, r.card_number);
			filesystem::storage::binary_serializer::write_field(record_data, r.barcode);
			filesystem::storage::binary_serializer::write_field(record_data, r.expiry);
			filesystem::storage::binary_serializer::write_field(record_data, r.holder);
			filesystem::storage::binary_serializer::write_field(record_data, r.cvv);
			filesystem::storage::binary_serializer::write_field(record_data, r.note);
			uint32_t total_len = 4 + 32 + static_cast<uint32_t>(record_data.size());
			core::endian::append_u32_be(buf, total_len);
			buf.insert(buf.end(), trans_type.begin(), trans_type.end());
			buf.insert(buf.end(), record_data.begin(), record_data.end());
		}
		for (const auto& r : mnemonic_records_) {
			std::vector<std::uint8_t> record_data;
			core::endian::append_u64_be(record_data, r.date);
			core::endian::append_u64_be(record_data, r.mnemonic.size());
			for (const auto& word : r.mnemonic) {
				filesystem::storage::binary_serializer::write_field(record_data, word);
			}
			filesystem::storage::binary_serializer::write_field(record_data, r.passphrase);
			filesystem::storage::binary_serializer::write_field(record_data, r.language);
			core::endian::append_u32_be(record_data, r.iteration);
			filesystem::storage::binary_serializer::write_field(record_data, r.note);
			uint32_t total_len = 4 + 32 + static_cast<uint32_t>(record_data.size());
			core::endian::append_u32_be(buf, total_len);
			buf.insert(buf.end(), mnem_type.begin(), mnem_type.end());
			buf.insert(buf.end(), record_data.begin(), record_data.end());
		}
		for (const auto& r : wifi_records_) {
			std::vector<std::uint8_t> record_data;
			core::endian::append_u64_be(record_data, r.date);
			filesystem::storage::binary_serializer::write_field(record_data, r.ssid);
			filesystem::storage::binary_serializer::write_field(record_data, r.password);
			filesystem::storage::binary_serializer::write_field(record_data, r.security);
			filesystem::storage::binary_serializer::write_field(record_data, r.note);
			uint32_t total_len = 4 + 32 + static_cast<uint32_t>(record_data.size());
			core::endian::append_u32_be(buf, total_len);
			buf.insert(buf.end(), wifi_type.begin(), wifi_type.end());
			buf.insert(buf.end(), record_data.begin(), record_data.end());
		}
		for (const auto& r : key_records_) {
			std::vector<std::uint8_t> record_data;
			core::endian::append_u64_be(record_data, r.date);
			filesystem::storage::binary_serializer::write_field(record_data, r.chain);
			filesystem::storage::binary_serializer::write_field(record_data, r.symbol);
			filesystem::storage::binary_serializer::write_field(record_data, r.publickey);
			filesystem::storage::binary_serializer::write_field(record_data, r.privatekey);
			filesystem::storage::binary_serializer::write_field(record_data, r.note);
			uint32_t total_len = 4 + 32 + static_cast<uint32_t>(record_data.size());
			core::endian::append_u32_be(buf, total_len);
			buf.insert(buf.end(), key_type.begin(), key_type.end());
			buf.insert(buf.end(), record_data.begin(), record_data.end());
		}
		return buf;
	}
	void FileDatabase::deserialize_records(const std::vector<std::uint8_t>& plaintext) {
		password_records_.clear();
		note_records_.clear();
		bankcard_records_.clear();
		discount_records_.clear();
		transport_records_.clear();
		mnemonic_records_.clear();
		wifi_records_.clear();
		key_records_.clear();
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
		const auto bank_type = type_system_.bankcard_type();
		const auto disc_type = type_system_.discountcard_type();
		const auto trans_type = type_system_.transportcard_type();
		const auto mnem_type = type_system_.mnemonic_type();
		const auto wifi_type = type_system_.wifi_type();
		const auto key_type = type_system_.key_type();
		while (offset < plaintext.size()) {
			if (offset + 4 > plaintext.size()) {
				throw core::errors::DeserialisationError{
				  "Truncated record: missing length field."
				};
			}
			uint32_t record_len = core::endian::read_u32_be(plaintext.data() + offset);
			offset += 4;
			if (record_len < 4 + 32) {
				throw core::errors::DeserialisationError{
				  "Invalid record length."
				};
			}
			if (offset + 32 > plaintext.size()) {
				throw core::errors::DeserialisationError{
				  "Truncated record: missing type identifier."
				};
			}
			domain::models::RecordType tag;
			std::memcpy(tag.data(), plaintext.data() + offset, tag.size());
			offset += 32;
			std::size_t data_start = offset;
			bool known = true;
			if (sodium_memcmp(tag.data(), pw_type.data(), tag.size()) == 0) {
				domain::models::PasswordRecord rec;
				if (offset + 8 > plaintext.size()) {
					throw core::errors::DeserialisationError{
					  "Truncated PasswordRecord: not enough bytes for date."
					};
				}
				rec.date = core::endian::read_u64_be(plaintext.data() + offset);
				offset += 8;
				rec.login = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.password = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.url = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.note = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				password_records_.push_back(std::move(rec));
			}
			else if (sodium_memcmp(tag.data(), note_type.data(), tag.size()) == 0) {
				domain::models::NoteRecord rec;
				if (offset + 8 > plaintext.size()) {
					throw core::errors::DeserialisationError{
					  "Truncated NoteRecord: not enough bytes for date."
					};
				}
				rec.date = core::endian::read_u64_be(plaintext.data() + offset);
				offset += 8;
				rec.title = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.note = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				note_records_.push_back(std::move(rec));
			}
			else if (sodium_memcmp(tag.data(), bank_type.data(), tag.size()) == 0) {
				domain::models::BankCardRecord rec;
				if (offset + 8 > plaintext.size()) {
					throw core::errors::DeserialisationError{
					  "Truncated BankCardRecord: not enough bytes for date."
					};
				}
				rec.date = core::endian::read_u64_be(plaintext.data() + offset);
				offset += 8;
				rec.card_number = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.expiry_date = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.cvv = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.cardholder_name = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.note = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				bankcard_records_.push_back(std::move(rec));
			}
			else if (sodium_memcmp(tag.data(), disc_type.data(), tag.size()) == 0) {
				domain::models::DiscountCardRecord rec;
				if (offset + 8 > plaintext.size()) {
					throw core::errors::DeserialisationError{
					  "Truncated DiscountCardRecord: not enough bytes for date."
					};
				}
				rec.date = core::endian::read_u64_be(plaintext.data() + offset);
				offset += 8;
				rec.card_number = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.barcode = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.cvv = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.store_name = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.note = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				discount_records_.push_back(std::move(rec));
			}
			else if (sodium_memcmp(tag.data(), trans_type.data(), tag.size()) == 0) {
				domain::models::TransportCardRecord rec;
				if (offset + 8 > plaintext.size()) {
					throw core::errors::DeserialisationError{
					  "Truncated TransportCardRecord: not enough bytes for date."
					};
				}
				rec.date = core::endian::read_u64_be(plaintext.data() + offset);
				offset += 8;
				rec.card_number = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.barcode = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.expiry = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.holder = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.cvv = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.note = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				transport_records_.push_back(std::move(rec));
			}
			else if (sodium_memcmp(tag.data(), mnem_type.data(), tag.size()) == 0) {
				domain::models::MnemonicRecord rec;
				if (offset + 16 > plaintext.size()) {
					throw core::errors::DeserialisationError{
					  "Truncated MnemonicRecord: not enough bytes for fixed fields."
					};
				}
				rec.date = core::endian::read_u64_be(plaintext.data() + offset);
				offset += 8;
				std::uint64_t word_count = core::endian::read_u64_be(plaintext.data() + offset);
				offset += 8;
				rec.value = word_count;
				if (word_count > 24) {
					throw core::errors::DeserialisationError{
					  "Invalid word count in MnemonicRecord."
					};
				}
				for (std::uint64_t i = 0; i < word_count; ++i) {
					rec.mnemonic.push_back(filesystem::storage::binary_serializer::read_string_field(plaintext, offset));
				}
				rec.passphrase = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.language = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				if (offset + 4 > plaintext.size()) {
					throw core::errors::DeserialisationError{
					  "Truncated MnemonicPhraseRecord: missing iteration."
					};
				}
				rec.iteration = core::endian::read_u32_be(plaintext.data() + offset);
				offset += 4;
				rec.note = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				mnemonic_records_.push_back(std::move(rec));
			}
			else if (sodium_memcmp(tag.data(), wifi_type.data(), tag.size()) == 0) {
				domain::models::WiFiRecord rec;
				if (offset + 8 > plaintext.size()) {
					throw core::errors::DeserialisationError{
					  "Truncated WiFiRecord: not enough bytes for date."
					};
				}
				rec.date = core::endian::read_u64_be(plaintext.data() + offset);
				offset += 8;
				rec.ssid = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.password = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.security = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.note = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				wifi_records_.push_back(std::move(rec));
			}
			else if (sodium_memcmp(tag.data(), key_type.data(), tag.size()) == 0) {
				domain::models::KeyRecord rec;
				if (offset + 8 > plaintext.size()) {
					throw core::errors::DeserialisationError{
					  "Truncated KeyRecord: not enough bytes for date."
					};
				}
				rec.date = core::endian::read_u64_be(plaintext.data() + offset);
				offset += 8;
				rec.chain = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.symbol = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.publickey = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.privatekey = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				rec.note = filesystem::storage::binary_serializer::read_string_field(plaintext, offset);
				key_records_.push_back(std::move(rec));
			}
			else {
				known = false;
			}
			if (known) {
				std::size_t data_end = data_start + (record_len - 4 - 32);
				if (offset != data_end) {
					throw core::errors::DeserialisationError{
					  "Size mismatch for known record type."
					};
				}
			}
			else {
				std::size_t data_len = record_len - 4 - 32;
				if (offset + data_len > plaintext.size()) {
					throw core::errors::DeserialisationError{
					  "Truncated unknown record."
					};
				}
				offset += data_len;
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
		if (blob.size() != core::constants::kMaxFileSize) {
			throw core::errors::DeserialisationError{
			  "Database file size mismatch: expected " +
			  std::to_string(core::constants::kMaxFileSize) +
			  " bytes, got " + std::to_string(blob.size()) +
			  ". This version only supports new format databases."
			};
		}
		constexpr std::size_t kHeaderSize = core::constants::kHeaderAdSize;
		if (blob.size() < kHeaderSize + 4) {
			throw core::errors::DeserialisationError{
			  "File too short."
			};
		}
		std::size_t off = 0;
		if (std::memcmp(blob.data() + off, core::constants::kFileMagic, 4) != 0) {
			throw core::errors::DeserialisationError{
			  "Not a valid password database file."
			};
		}
		off += 4;
		if (off >= blob.size()) {
			throw core::errors::DeserialisationError{
			  "Invalid file format: missing null byte after magic."
			};
		}
		++off;
		if (off + core::constants::kSaltBytes > blob.size()) {
			throw core::errors::DeserialisationError{
			  "Truncated file: missing salt."
			};
		}
		std::vector<std::uint8_t> salt(blob.begin() + off,
			blob.begin() + off + core::constants::kSaltBytes);
		off += core::constants::kSaltBytes;
		if (off + core::constants::kAeadNonceBytes > blob.size()) {
			throw core::errors::DeserialisationError{
			  "Truncated file: missing nonce."
			};
		}
		std::vector<std::uint8_t> nonce(blob.begin() + off,
			blob.begin() + off + core::constants::kAeadNonceBytes);
		off += core::constants::kAeadNonceBytes;
		if (off + 8 > blob.size()) {
			throw core::errors::DeserialisationError{
			  "Truncated file: missing created timestamp."
			};
		}
		std::uint64_t stored_created = core::endian::read_u64_be(blob.data() + off);
		off += 8;
		if (off + 8 > blob.size()) {
			throw core::errors::DeserialisationError{
			  "Truncated file: missing modified timestamp."
			};
		}
		std::uint64_t stored_modified = core::endian::read_u64_be(blob.data() + off);
		off += 8;
		off += 4;
		if (off + 4 > blob.size()) {
			throw core::errors::DeserialisationError{
			  "Truncated file: missing payload length."
			};
		}
		std::uint32_t payload_len = core::endian::read_u32_be(blob.data() + off);
		off += 4;
		if (off + payload_len > blob.size()) {
			throw core::errors::DeserialisationError{
			  "Payload extends beyond end of file."
			};
		}
		std::size_t ad_size = off - 4;
		std::vector<std::uint8_t> ad(blob.begin(), blob.begin() + ad_size);
		auto derived_key = crypto_->derive_key(master_password, salt);
		std::vector<std::uint8_t> encrypted(blob.begin() + off,
			blob.begin() + off + payload_len);
		auto padded_plaintext = crypto_->aead_decrypt(encrypted, ad, nonce, derived_key);
		const std::size_t expected_padded_size = payload_len - core::constants::kAeadTagBytes;
		if (padded_plaintext.size() != expected_padded_size) {
			throw core::errors::DeserialisationError{
			  "Decrypted plaintext size mismatch."
			};
		}
		if (padded_plaintext.size() < 8) {
			throw core::errors::DeserialisationError{
			  "Padded plaintext too short."
			};
		}
		uint64_t real_size = core::endian::read_u64_be(padded_plaintext.data());
		if (real_size + 8 > padded_plaintext.size()) {
			throw core::errors::DeserialisationError{
			  "Declared data size exceeds padded plaintext."
			};
		}
		std::vector<std::uint8_t> real_plaintext(
			padded_plaintext.begin() + 8,
			padded_plaintext.begin() + 8 + real_size);
		deserialize_records(real_plaintext);
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
		header.insert(header.end(), core::constants::kFileMagic, core::constants::kFileMagic + 4);
		header.push_back(core::constants::kFileMagicNull);
		header.insert(header.end(), salt.begin(), salt.end());
		header.insert(header.end(), nonce.begin(), nonce.end());
		core::endian::append_u64_be(header, ts_created_);
		core::endian::append_u64_be(header, ts_modified_);
		core::endian::append_u32_be(header, static_cast<std::uint32_t>(record_count()));
		std::vector<std::uint8_t> real_data = serialize_records();
		uint64_t real_size = real_data.size();
		const size_t header_size = header.size();
		const size_t max_file_size = core::constants::kMaxFileSize;
		const size_t max_ciphertext_size = max_file_size - header_size - 4;
		const size_t padded_plaintext_size = max_ciphertext_size - core::constants::kAeadTagBytes;
		if (real_size + 8 > padded_plaintext_size) {
			throw core::errors::PolicyViolation(
				"Database too large to fit in maximum file size.");
		}
		std::vector<std::uint8_t> padded_plaintext(padded_plaintext_size);
		auto random_fill = crypto_->random_bytes(padded_plaintext_size);
		std::copy(random_fill.begin(), random_fill.end(), padded_plaintext.begin());
		core::endian::write_u64_be(padded_plaintext.data(), real_size);
		std::copy(real_data.begin(), real_data.end(), padded_plaintext.begin() + 8);
		std::vector<std::uint8_t> ciphertext = crypto_->aead_encrypt(
			padded_plaintext, header, nonce, derived_key);
		if (ciphertext.size() != max_ciphertext_size) {
			throw core::errors::CryptoError("Unexpected ciphertext size after padding.");
		}
		std::vector<std::uint8_t> out;
		out.reserve(header_size + 4 + ciphertext.size());
		out.insert(out.end(), header.begin(), header.end());
		core::endian::append_u32_be(out, static_cast<std::uint32_t>(ciphertext.size()));
		out.insert(out.end(), ciphertext.begin(), ciphertext.end());
		if (out.size() != max_file_size) {
			throw core::errors::CryptoError("Final file size mismatch.");
		}
		std::ofstream ofs(file_path, std::ios::binary | std::ios::trunc);
		if (!ofs.is_open()) return false;
		ofs.write(reinterpret_cast<const char*>(out.data()),
			static_cast<std::streamsize>(out.size()));
		return ofs.good();
	}
	std::size_t FileDatabase::estimate_remaining_capacity() const {
		std::vector<std::uint8_t> real_data = serialize_records();
		std::size_t real_size = real_data.size();
		const std::size_t fixed_overhead = core::constants::kHeaderAdSize + 4 + 8 + core::constants::kAeadTagBytes;
		const std::size_t max_real_data = core::constants::kMaxFileSize - fixed_overhead;
		if (real_size > max_real_data) {
			return 0;
		}
		return max_real_data - real_size;
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
	void FileDatabase::add_bankcard_record(domain::models::BankCardRecord record) {
		if (record.date == 0) record.date = unix_timestamp_now();
		bankcard_records_.push_back(std::move(record));
	}
	bool FileDatabase::remove_bankcard_record(std::size_t index) {
		if (index >= bankcard_records_.size()) return false;
		bankcard_records_.erase(bankcard_records_.begin() +
			static_cast<std::ptrdiff_t>(index));
		return true;
	}
	const std::vector<domain::models::BankCardRecord>&
		FileDatabase::bankcard_records() const noexcept {
		return bankcard_records_;
	}
	std::size_t FileDatabase::bankcard_record_count() const noexcept {
		return bankcard_records_.size();
	}
	void FileDatabase::add_discountcard_record(domain::models::DiscountCardRecord record) {
		if (record.date == 0) record.date = unix_timestamp_now();
		discount_records_.push_back(std::move(record));
	}
	bool FileDatabase::remove_discountcard_record(std::size_t index) {
		if (index >= discount_records_.size()) return false;
		discount_records_.erase(discount_records_.begin() +
			static_cast<std::ptrdiff_t>(index));
		return true;
	}
	const std::vector<domain::models::DiscountCardRecord>&
		FileDatabase::discountcard_records() const noexcept {
		return discount_records_;
	}
	std::size_t FileDatabase::discountcard_record_count() const noexcept {
		return discount_records_.size();
	}
	void FileDatabase::add_transportcard_record(domain::models::TransportCardRecord record) {
		if (record.date == 0) record.date = unix_timestamp_now();
		transport_records_.push_back(std::move(record));
	}
	bool FileDatabase::remove_transportcard_record(std::size_t index) {
		if (index >= transport_records_.size()) return false;
		transport_records_.erase(transport_records_.begin() +
			static_cast<std::ptrdiff_t>(index));
		return true;
	}
	const std::vector<domain::models::TransportCardRecord>&
		FileDatabase::transportcard_records() const noexcept {
		return transport_records_;
	}
	std::size_t FileDatabase::transportcard_record_count() const noexcept {
		return transport_records_.size();
	}
	void FileDatabase::add_mnemonic_record(domain::models::MnemonicRecord record) {
		if (record.date == 0) record.date = unix_timestamp_now();
		mnemonic_records_.push_back(std::move(record));
	}
	bool FileDatabase::remove_mnemonic_record(std::size_t index) {
		if (index >= mnemonic_records_.size()) return false;
		mnemonic_records_.erase(mnemonic_records_.begin() + static_cast<std::ptrdiff_t>(index));
		return true;
	}
	const std::vector<domain::models::MnemonicRecord>&
		FileDatabase::mnemonic_records() const noexcept {
		return mnemonic_records_;
	}
	std::size_t FileDatabase::mnemonic_record_count() const noexcept {
		return mnemonic_records_.size();
	}
	void FileDatabase::add_wifi_record(domain::models::WiFiRecord record) {
		if (record.date == 0) record.date = unix_timestamp_now();
		wifi_records_.push_back(std::move(record));
	}
	bool FileDatabase::remove_wifi_record(std::size_t index) {
		if (index >= wifi_records_.size()) return false;
		wifi_records_.erase(wifi_records_.begin() + static_cast<std::ptrdiff_t>(index));
		return true;
	}
	const std::vector<domain::models::WiFiRecord>&
		FileDatabase::wifi_records() const noexcept {
		return wifi_records_;
	}
	std::size_t FileDatabase::wifi_record_count() const noexcept {
		return wifi_records_.size();
	}
	void FileDatabase::add_key_record(domain::models::KeyRecord record) {
		if (record.date == 0) record.date = unix_timestamp_now();
		key_records_.push_back(std::move(record));
	}
	bool FileDatabase::remove_key_record(std::size_t index) {
		if (index >= key_records_.size()) return false;
		key_records_.erase(key_records_.begin() + static_cast<std::ptrdiff_t>(index));
		return true;
	}
	const std::vector<domain::models::KeyRecord>& FileDatabase::key_records() const noexcept {
		return key_records_;
	}
	std::size_t FileDatabase::key_record_count() const noexcept {
		return key_records_.size();
	}
	std::size_t FileDatabase::record_count() const noexcept {
		return password_records_.size() + note_records_.size() + bankcard_records_.size() +
			discount_records_.size() + transport_records_.size() + mnemonic_records_.size() +
			wifi_records_.size() + key_records_.size();
	}
	std::uint64_t FileDatabase::timestamp_created() const noexcept {
		return ts_created_;
	}
	std::uint64_t FileDatabase::timestamp_modified() const noexcept {
		return ts_modified_;
	}
}