// DronTokenizer.hpp
#pragma once

#include <vector>
#include <string>

#include "DronToken.hpp"

class DronTokenizer {
public:
    // Core tokenization variants
    std::vector<Token> tokenizeFile(const std::string& fileLocation);
    std::vector<Token> tokenizeString(const std::string& rawText);

    const std::vector<Token>& getTokens() const { return tokens_; }

private:
    void handleMultiline(const std::string& trimmed);
    void handleSectionHeader(const std::string& trimmed);
    void handleKeyValue(const std::string& trimmed);
    void tokenizeValue(const std::string& value);

    void emitToken(TokenType type, std::string value, uint32_t lineNumber);

    uint32_t line_number_{ 0 };
    bool in_multiline_{ false };
    std::string current_multiline_content_{};

    uint32_t container_depth_{ 0 };

    std::vector<Token> tokens_{};
};