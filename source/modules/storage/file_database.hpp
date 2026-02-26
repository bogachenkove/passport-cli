#pragma once
#include "../interface/interface_database.hpp"
#include "../interface/interface_crypto_service.hpp"
#include "../models/password_record.hpp"
#include "../models/note_record.hpp"
#include "../models/bank_card_record.hpp"
#include "../models/discount_card_record.hpp"
#include "../models/transport_card_record.hpp"
#include "../models/mnemonic_record.hpp"
#include "../models/wifi_record.hpp"
#include "../crypto/type_system.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace security::storage {
	class FileDatabase final : public domain::interfaces::IDatabase {
	public:
		explicit FileDatabase(std::shared_ptr<domain::interfaces::ICryptoService> crypto);
		~FileDatabase() override = default;
		FileDatabase(const FileDatabase&) = delete;
		FileDatabase& operator=(const FileDatabase&) = delete;
		FileDatabase(FileDatabase&&) = default;
		FileDatabase& operator=(FileDatabase&&) = default;
		bool load_from_file(const std::string& file_path, const std::string& master_password) override;
		bool save_to_file(const std::string& file_path, const std::string& master_password) override;
		void add_password_record(domain::models::PasswordRecord record) override;
		void add_note_record(domain::models::NoteRecord record) override;
		void add_bankcard_record(domain::models::BankCardRecord record) override;
		void add_discountcard_record(domain::models::DiscountCardRecord record) override;
		void add_transportcard_record(domain::models::TransportCardRecord record) override;
		void add_mnemonic_record(domain::models::MnemonicRecord record) override;
		void add_wifi_record(domain::models::WiFiRecord record) override;
		bool remove_password_record(std::size_t index) override;
		bool remove_note_record(std::size_t index) override;
		bool remove_bankcard_record(std::size_t index) override;
		bool remove_discountcard_record(std::size_t index) override;
		bool remove_transportcard_record(std::size_t index) override;
		bool remove_mnemonic_record(std::size_t index) override;
		bool remove_wifi_record(std::size_t index) override;
		[[nodiscard]] const std::vector<domain::models::PasswordRecord>& password_records() const noexcept override;
		[[nodiscard]] const std::vector<domain::models::NoteRecord>& note_records() const noexcept override;
		[[nodiscard]] const std::vector<domain::models::BankCardRecord>& bankcard_records() const noexcept override;
		[[nodiscard]] const std::vector<domain::models::DiscountCardRecord>& discountcard_records() const noexcept override;
		[[nodiscard]] const std::vector<domain::models::TransportCardRecord>& transportcard_records() const noexcept override;
		[[nodiscard]] const std::vector<domain::models::MnemonicRecord>& mnemonic_records() const noexcept override;
		[[nodiscard]] const std::vector<domain::models::WiFiRecord>& wifi_records() const noexcept override;
		[[nodiscard]] std::size_t password_record_count() const noexcept override;
		[[nodiscard]] std::size_t note_record_count() const noexcept override;
		[[nodiscard]] std::size_t bankcard_record_count() const noexcept override;
		[[nodiscard]] std::size_t discountcard_record_count() const noexcept override;
		[[nodiscard]] std::size_t transportcard_record_count() const noexcept override;
		[[nodiscard]] std::size_t mnemonic_record_count() const noexcept override;
		[[nodiscard]] std::size_t wifi_record_count() const noexcept override;
		[[nodiscard]] std::size_t record_count() const noexcept override;
		[[nodiscard]] std::uint64_t timestamp_created() const noexcept override;
		[[nodiscard]] std::uint64_t timestamp_modified() const noexcept override;
	private:
		std::shared_ptr<domain::interfaces::ICryptoService> crypto_;
		std::vector<domain::models::PasswordRecord> password_records_;
		std::vector<domain::models::NoteRecord>     note_records_;
		std::vector<domain::models::BankCardRecord> bankcard_records_;
		std::vector<domain::models::DiscountCardRecord> discount_records_;
		std::vector<domain::models::TransportCardRecord> transport_records_;
		std::vector<domain::models::MnemonicRecord> mnemonic_records_;
		std::vector<domain::models::WiFiRecord> wifi_records_;
		security::crypto::TypeSystem type_system_;
		std::uint64_t ts_created_ = 0;
		std::uint64_t ts_modified_ = 0;
		std::vector<std::uint8_t> serialize_records() const;
		void deserialize_records(const std::vector<std::uint8_t>& plaintext);
		static std::uint64_t unix_timestamp_now();
	};
}