#pragma once
#include "../models/password_record.hpp"
#include "../models/note_record.hpp"
#include "../models/bank_card_record.hpp"
#include "../models/discount_card_record.hpp"
#include "../models/transport_card_record.hpp"
#include "../models/mnemonic_record.hpp"
#include "../models/wifi_record.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace domain::interfaces {
	class IDatabase {
	public:
		virtual ~IDatabase() = default;
		virtual bool load_from_file(const std::string& file_path, const std::string& master_password) = 0;
		virtual bool save_to_file(const std::string& file_path, const std::string& master_password) = 0;
		virtual void add_password_record(domain::models::PasswordRecord record) = 0;
		virtual void add_note_record(domain::models::NoteRecord record) = 0;
		virtual void add_bankcard_record(domain::models::BankCardRecord record) = 0;
		virtual void add_discountcard_record(domain::models::DiscountCardRecord record) = 0;
		virtual void add_transportcard_record(domain::models::TransportCardRecord record) = 0;
		virtual void add_mnemonic_record(domain::models::MnemonicRecord record) = 0;
		virtual void add_wifi_record(domain::models::WiFiRecord record) = 0;
		virtual bool remove_password_record(std::size_t index) = 0;
		virtual bool remove_note_record(std::size_t index) = 0;
		virtual bool remove_bankcard_record(std::size_t index) = 0;
		virtual bool remove_discountcard_record(std::size_t index) = 0;
		virtual bool remove_transportcard_record(std::size_t index) = 0;
		virtual bool remove_mnemonic_record(std::size_t index) = 0;
		virtual bool remove_wifi_record(std::size_t index) = 0;
		[[nodiscard]] virtual const std::vector<domain::models::PasswordRecord>& password_records() const noexcept = 0;
		[[nodiscard]] virtual const std::vector<domain::models::NoteRecord>& note_records() const noexcept = 0;
		[[nodiscard]] virtual const std::vector<domain::models::BankCardRecord>& bankcard_records() const noexcept = 0;
		[[nodiscard]] virtual const std::vector<domain::models::DiscountCardRecord>& discountcard_records() const noexcept = 0;
		[[nodiscard]] virtual const std::vector<domain::models::TransportCardRecord>& transportcard_records() const noexcept = 0;
		[[nodiscard]] virtual const std::vector<domain::models::MnemonicRecord>& mnemonic_records() const noexcept = 0;
		[[nodiscard]] virtual const std::vector<domain::models::WiFiRecord>& wifi_records() const noexcept = 0;
		[[nodiscard]] virtual std::size_t password_record_count() const noexcept = 0;
		[[nodiscard]] virtual std::size_t note_record_count() const noexcept = 0;
		[[nodiscard]] virtual std::size_t bankcard_record_count() const noexcept = 0;
		[[nodiscard]] virtual std::size_t discountcard_record_count() const noexcept = 0;
		[[nodiscard]] virtual std::size_t transportcard_record_count() const noexcept = 0;
		[[nodiscard]] virtual std::size_t mnemonic_record_count() const noexcept = 0;
		[[nodiscard]] virtual std::size_t wifi_record_count() const noexcept = 0;
		[[nodiscard]] virtual std::size_t record_count() const noexcept = 0;
		[[nodiscard]] virtual std::uint64_t timestamp_created() const noexcept = 0;
		[[nodiscard]] virtual std::uint64_t timestamp_modified() const noexcept = 0;
	};
}