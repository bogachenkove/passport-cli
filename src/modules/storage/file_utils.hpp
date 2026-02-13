#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "../interface/interface_crypto_service.hpp"   // для ICryptoService

namespace filesystem::storage {

    // Генерация уникального имени файла: passport_<16 hex>.db
    // Требует криптосервис для случайных байт
    [[nodiscard]] std::string generate_unique_db_filename(
        domain::interfaces::ICryptoService& crypto);

    // Нормализация пути: абсолютный путь без генерации имени
    [[nodiscard]] std::string normalise_path(const std::string& raw);

    // Нормализация пути: абсолютный путь + генерация имени, если передан каталог
    // Требует криптосервис для создания уникального имени
    [[nodiscard]] std::string normalise_db_path(
        const std::string& raw,
        domain::interfaces::ICryptoService& crypto);

    // Проверка возможности записи/чтения файла (через открытие потока)
    bool check_file_access(const std::string& path, bool for_write);

    // Валидация размера файла (не превышает kMaxDatabaseFileSize)
    void validate_file_size(const std::string& path);

    // Валидация размера блоба в памяти
    void validate_blob_size(std::size_t size);

    // Проверка существования и типа файла (обёртки над core::platform)
    bool is_directory(const std::string& path);
    bool is_regular_file(const std::string& path);

} // namespace filesystem::storage