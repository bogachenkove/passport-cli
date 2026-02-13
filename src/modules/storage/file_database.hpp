#pragma once

#include "../interface/interface_database.hpp"
#include "../interface/interface_crypto_service.hpp"
#include "../crypto/type_system.hpp"
#include <cstdint>
#include <memory>
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

        bool load_from_file(const std::string& file_path,
            const std::string& master_password) override;
        bool save_to_file(const std::string& file_path,
            const std::string& master_password) override;

        void add_password_record(domain::models::PasswordRecord record) override;
        void add_note_record(domain::models::NoteRecord record) override;

        bool remove_password_record(std::size_t index) override;
        bool remove_note_record(std::size_t index) override;

        [[nodiscard]] const std::vector<domain::models::PasswordRecord>&
            password_records() const noexcept override;
        [[nodiscard]] const std::vector<domain::models::NoteRecord>&
            note_records() const noexcept override;

        [[nodiscard]] std::size_t password_record_count() const noexcept override;
        [[nodiscard]] std::size_t note_record_count() const noexcept override;
        [[nodiscard]] std::size_t record_count() const noexcept override;

        [[nodiscard]] std::uint64_t timestamp_created() const noexcept override;
        [[nodiscard]] std::uint64_t timestamp_modified() const noexcept override;

    private:
        std::shared_ptr<domain::interfaces::ICryptoService> crypto_;
        std::vector<domain::models::PasswordRecord> password_records_;
        std::vector<domain::models::NoteRecord>     note_records_;
        security::crypto::TypeSystem type_system_;
        std::uint64_t ts_created_ = 0;
        std::uint64_t ts_modified_ = 0;

        std::vector<std::uint8_t> serialize_records() const;
        void deserialize_records(const std::vector<std::uint8_t>& plaintext);
        static std::uint64_t unix_timestamp_now();
    };

} // namespace security::storage