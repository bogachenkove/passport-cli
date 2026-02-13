#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include "../crypto/type_system.hpp" 
#include "../models/password_record.hpp"
#include "../models/note_record.hpp"

namespace domain::interfaces {

class IDatabase {
public:
    virtual ~IDatabase() = default;

    // Загрузка из файла с мастер-паролем
    virtual bool load_from_file(
        const std::string& file_path,
        const std::string& master_password) = 0;

    // Сохранение в файл с мастер-паролем
    virtual bool save_to_file(
        const std::string& file_path,
        const std::string& master_password) = 0;

    // Добавление записей
    virtual void add_password_record(domain::models::PasswordRecord record) = 0;
    virtual void add_note_record(domain::models::NoteRecord record) = 0;

    // Удаление по индексу (0‑based)
    virtual bool remove_password_record(std::size_t index) = 0;
    virtual bool remove_note_record(std::size_t index) = 0;

    // Получение всех записей (только чтение)
    [[nodiscard]]
    virtual const std::vector<domain::models::PasswordRecord>& password_records() const noexcept = 0;
    [[nodiscard]]
    virtual const std::vector<domain::models::NoteRecord>& note_records() const noexcept = 0;

    // Количество записей
    [[nodiscard]]
    virtual std::size_t password_record_count() const noexcept = 0;
    [[nodiscard]]
    virtual std::size_t note_record_count() const noexcept = 0;
    [[nodiscard]]
    virtual std::size_t record_count() const noexcept = 0;

    // Метаданные
    [[nodiscard]]
    virtual std::uint64_t timestamp_created() const noexcept = 0;
    [[nodiscard]]
    virtual std::uint64_t timestamp_modified() const noexcept = 0;
};

} // namespace domain::interfaces