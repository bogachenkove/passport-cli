#include "field_validator.hpp"
#include <cctype>
#include <algorithm>
#include <regex>
#include <string>
#include <vector>
#include <cstdint>
#include <array>
#include <sodium.h>

namespace domain::validation {
    bool is_field_empty(std::string_view value) {
        return value.empty() || std::all_of(value.begin(), value.end(), [](unsigned char c) {
                    return std::isspace(c);
                }
            );
    }
    bool is_ascii_field_valid(std::string_view value,
        std::size_t min_len,
        std::size_t max_len,
        bool optional) {
        if (optional && value.empty()) return true;
        if (!optional && value.empty()) return false;
        if (value.size() < min_len || value.size() > max_len) return false;
        for (unsigned char ch : value) {
            if (ch < 0x20 || ch > 0x7E) return false;
        }
        return true;
    }
    bool is_valid_url(std::string_view url) {
        if (url.empty()) return true;
        try {
            std::string url_str(url);
            std::regex url_regex(R"(^(https?://)?([a-zA-Z0-9][-a-zA-Z0-9]*\.)+[a-zA-Z]{2,6}(/[-a-zA-Z0-9._~:/?#[\]@!$&'()*+,;=]*)?$)");
            return std::regex_match(url_str, url_regex);
        }
        catch (const std::regex_error&) {
            return false;
        }
    }
    std::string ensure_url_protocol(std::string_view url) {
        if (url.empty()) return std::string(url);
        if (url.starts_with("http://") || url.starts_with("https://"))
            return std::string(url);
        return "http://" + std::string(url);
    }
    bool is_single_char_valid(std::string_view input, char expected1, char expected2) {
        if (input.length() != 1) return false;
        char c = std::tolower(static_cast<unsigned char>(input[0]));
        return c == expected1 || c == expected2;
    }
    bool is_single_char_valid_with_cancel(std::string_view input,
        char expected1,
        char expected2) {
        if (input.length() != 1) return false;
        char c = std::tolower(static_cast<unsigned char>(input[0]));
        return c == expected1 || c == expected2 || c == 'q';
    }
    bool is_digits_only(std::string_view value) {
        return !value.empty() && std::all_of(value.begin(), value.end(),
            [](unsigned char c) {
                return std::isdigit(c);
            }
        );
    }
    bool is_valid_expiry(std::string_view value) {
        if (value.size() != 5) return false;
        if (value[2] != '/') return false;
        if (!std::isdigit(static_cast<unsigned char>(value[0])) ||
            !std::isdigit(static_cast<unsigned char>(value[1])) ||
            !std::isdigit(static_cast<unsigned char>(value[3])) ||
            !std::isdigit(static_cast<unsigned char>(value[4]))) return false;
        int month = (value[0] - '0') * 10 + (value[1] - '0');
        return month >= 1 && month <= 12;
    }
    bool is_letters_and_spaces(std::string_view value) {
        return !value.empty() && std::all_of(value.begin(), value.end(),
            [](unsigned char c) {
                return std::isalpha(c) || std::isspace(c);
            }
        );
    }
    bool is_luhn_valid(std::string_view card_number) {
        std::array<int, 19> digits;
        size_t digit_count = 0;
        for (unsigned char c : card_number) {
            if (std::isdigit(c)) {
                if (digit_count >= 19) return false;
                digits[digit_count++] = c - '0';
            }
        }
        if (digit_count < 2) return false;
        int sum = 0;
        bool alternate = false;
        for (size_t i = digit_count; i-- > 0;) {
            int n = digits[i];
            if (alternate) {
                n *= 2;
                if (n > 9) n -= 9;
            }
            sum += n;
            alternate = !alternate;
        }
        return (sum % 10) == 0;
    }
    bool is_mnemonic_valid(const std::vector<security::SecureString>& words, const std::vector<std::string>& wordlist) {
        const size_t word_count = words.size();
        if (word_count != 12 && word_count != 15 && word_count != 18 && word_count != 21 && word_count != 24) {
            return false;
        }
        std::vector<uint16_t> indices(word_count);
        for (size_t i = 0;
            i < word_count;
            ++i) {
            std::string_view word(words[i].c_str(), words[i].size());
            auto it = std::find(wordlist.begin(), wordlist.end(), word);
            if (it == wordlist.end()) {
                return false;
            }
            indices[i] = static_cast<uint16_t>(std::distance(wordlist.begin(), it));
        }
        const size_t total_bits = word_count * 11;
        const size_t total_bytes = (total_bits + 7) / 8;
        std::vector<uint8_t> bin(total_bytes, 0);
        size_t bit_pos = 0;
        for (uint16_t idx : indices) {
            for (int b = 10;
                b >= 0;
                --b) {
                if (idx & (1 << b)) {
                    bin[bit_pos / 8] |= (1 << (7 - (bit_pos % 8)));
                }
                ++bit_pos;
            }
        }
        const size_t cs_bits = word_count / 3;
        const size_t entropy_bits = total_bits - cs_bits;
        const size_t entropy_bytes = entropy_bits / 8;
        std::vector<uint8_t> hash(crypto_hash_sha256_BYTES);
        crypto_hash_sha256(hash.data(), bin.data(), entropy_bytes);
        uint8_t stored_checksum = bin[entropy_bytes] >> (8 - cs_bits);
        uint8_t expected_checksum = hash[0] >> (8 - cs_bits);
        return stored_checksum == expected_checksum;
    }
}