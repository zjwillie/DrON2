#pragma once

// DronToken.hpp

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>

enum class TokenType : uint8_t {
	// structural
	SECTION_HEADER,
	EQUALS,
	LEFT_BRACKET,
	RIGHT_BRACKET,
	LEFT_BRACE,
	RIGHT_BRACE,
	COMMA,
	COMMENT,
	INLINE_COMMENT,
	BLANK,

	// errors
	UNKNOWN,
	UNKNOWN_VALUE,

	// values
	INTEGER,
	DOUBLE,
	BOOLEAN,
	STRING,
	MULTILINE_STRING,
	IDENTIFIER,

	// keys
	KEY,

	// housekeeping
	END_OF_FILE
};

struct Token {
	TokenType type{};
	std::string value{};
	uint32_t lineNumber{ 0 };
	uint32_t index{ 0 };
};

// -----------------------------------------------------------------------------
// Lookup Table (LUT)
// -----------------------------------------------------------------------------
inline const std::vector<std::pair<TokenType, std::string_view>>& tokenTypeLUT() {
	static const std::vector<std::pair<TokenType, std::string_view>> lut = {
		// structural
		{ TokenType::SECTION_HEADER,	"SECTION_HEADER" },
		{ TokenType::EQUALS,			"EQUALS" },
		{ TokenType::LEFT_BRACKET,		"LEFT_BRACKET" },
		{ TokenType::RIGHT_BRACKET,		"RIGHT_BRACKET" },
		{ TokenType::LEFT_BRACE,		"LEFT_BRACE" },
		{ TokenType::RIGHT_BRACE,		"RIGHT_BRACE" },
		{ TokenType::COMMA,				"COMMA" },
		{ TokenType::COMMENT,			"COMMENT" },
		{ TokenType::INLINE_COMMENT,	"INLINE_COMMENT" },
		{ TokenType::BLANK,				"BLANK" },

		// errors
		{ TokenType::UNKNOWN,			"UNKNOWN" },
		{ TokenType::UNKNOWN_VALUE,		"UNKNOWN_VALUE" },

		// values
		{ TokenType::INTEGER,			"INTEGER" },
		{ TokenType::DOUBLE,			"DOUBLE" },
		{ TokenType::BOOLEAN,			"BOOLEAN" },
		{ TokenType::STRING,			"STRING" },
		{ TokenType::MULTILINE_STRING,	"MULTILINE_STRING" },
		{ TokenType::IDENTIFIER,		"IDENTIFIER" },

		// keys
		{ TokenType::KEY,				"KEY" },

		// housekeeping
		{ TokenType::END_OF_FILE,		"END_OF_FILE" }
	};
	return lut;
}

//! really seems like we should namespace and wrap these...
// -----------------------------------------------------------------------------
// Conversion: DronToken -> string
// -----------------------------------------------------------------------------
inline std::string tokenTypeToString(TokenType tokenType) {
	for (const auto& [t, name] : tokenTypeLUT()) {
		if (t == tokenType) return std::string(name);
	}
	return "Unknown DronToken type sent to DronToken.hpp:tokenTypeToString.";
}

// -----------------------------------------------------------------------------
// Conversion: string -> TokenType
// -----------------------------------------------------------------------------
inline TokenType stringToTokenType(const std::string& s) {
	std::string lowered = s;
	std::transform(lowered.begin(), lowered.end(), lowered.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

	for (const auto& [type, name] : tokenTypeLUT()) {
		// Convert string_view LUT name to lowercase for comparison if needed, 
		// or match directly if your source strings are uppercase.
		// Assuming direct case-insensitive or exact match context:
		std::string nameLower(name);
		std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

		if (lowered == nameLower) return type;
	}

	// Aliases for compatibility
	if (lowered == "eof") return TokenType::END_OF_FILE;

	throw std::runtime_error("[TokenTypeEnum] Unknown token type string: " + s);
}