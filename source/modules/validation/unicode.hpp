#pragma once
#include <string>
#include <unicode/unistr.h>
#include <unicode/normalizer2.h>

namespace domain::validation {
	inline std::string normalize_nfkd(const std::string& input) {
		static const icu::Normalizer2* normalizer = []() -> const icu::Normalizer2* {
			UErrorCode err = U_ZERO_ERROR;
			const auto* instance = icu::Normalizer2::getNFKDInstance(err);
			if (U_FAILURE(err)) return nullptr;
			return instance;
			}();
		if (!normalizer) return input;
		icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(input);
		icu::UnicodeString normalized;
		UErrorCode status = U_ZERO_ERROR;
		normalizer->normalize(ustr, normalized, status);
		if (U_FAILURE(status)) return input;
		std::string result;
		normalized.toUTF8String(result);
		return result;
	}
}