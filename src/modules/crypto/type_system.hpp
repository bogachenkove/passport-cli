#pragma once

#include "../models/record_types.hpp"
#include "../interface/interface_crypto_service.hpp"
#include <vector>
#include <cstdint>

namespace security::crypto {

    class TypeSystem {
    public:
        TypeSystem() = default;
        TypeSystem(const std::vector<std::uint8_t>& master_seed,
            const std::vector<std::uint8_t>& context_salt);

        // Генерация новых seed'ов с использованием криптосервиса
        void generate_seeds(domain::interfaces::ICryptoService& crypto);

        // Вывод RecordType по контекстной строке
        [[nodiscard]] domain::models::RecordType
            derive_type(const std::string& context_string) const;

        // Специализированные типы
        [[nodiscard]] domain::models::RecordType password_type() const;
        [[nodiscard]] domain::models::RecordType note_type() const;

        // Доступ к seed'ам (для сериализации)
        [[nodiscard]] const std::vector<std::uint8_t>& master_seed() const noexcept;
        [[nodiscard]] const std::vector<std::uint8_t>& context_salt() const noexcept;

        [[nodiscard]] bool is_initialised() const noexcept;

    private:
        std::vector<std::uint8_t> master_seed_;
        std::vector<std::uint8_t> context_salt_;
    };

} // namespace security::crypto