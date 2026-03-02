#pragma once
#include <string>
#include <string_view>
#include <unicode/unistr.h>
#include <unicode/normalizer2.h>
#include "../security/secure_string.hpp"
#include "../security/secure_buffer.hpp"

namespace domain::validation {
	inline security::SecureString normalize_nfkd(std::string_view input) {
		static const icu::Normalizer2* normalizer = []() -> const icu::Normalizer2* {
			UErrorCode err = U_ZERO_ERROR;
			const auto* instance = icu::Normalizer2::getNFKDInstance(err);
			if (U_FAILURE(err)) return nullptr;
			return instance;
			}
		();
		if (!normalizer || input.empty()) {
			return security::SecureString(input);
		}
		icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(
			icu::StringPiece(input.data(), static_cast<int32_t>(input.size())));
		icu::UnicodeString normalized;
		UErrorCode status = U_ZERO_ERROR;
		normalizer->normalize(ustr, normalized, status);
		if (U_FAILURE(status)) {
			return security::SecureString(input);
		}
		int32_t utf8Length = normalized.extract(0, normalized.length(), nullptr, "utf-8");
		if (utf8Length <= 0) {
			return security::SecureString();
		}
		security::SecureBuffer<char> buffer(utf8Length);
		char* dest = buffer.data();
		int32_t written = normalized.extract(0, normalized.length(), dest, "utf-8");
		if (written != utf8Length) {
			return security::SecureString(input);
		}
		return security::SecureString(std::string_view(dest, written));
	}
}