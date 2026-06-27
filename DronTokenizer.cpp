#include "DronTokenizer.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

namespace {
    std::string trimWhiteSpace(const std::string& line) {
        const std::string whiteSpace = " \t\r\n\v\f";

        // grab the first point there is no white space
        size_t start = line.find_first_not_of(whiteSpace);
        if (start == std::string::npos) return "";

        // now grab the last
        size_t end = line.find_last_not_of(whiteSpace);

        
        return line.substr(start, end - start + 1);
    }

    bool isAlpha(char c) { 
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    bool isNumeric(char c) {
        return (c >= '0' && c <= '9');
    }

    bool isUpper(char c) {
        return (c >= 'A' && c <= 'Z');
    }

    bool isLower(char c) {
        return (c >= 'a' && c <= 'z');
    }

    size_t getDecimalPosition(const std::string& line) {
        return line.find('.');
    }
}

void DronTokenizer::emitToken(TokenType type, std::string value, uint32_t line_number) {
    tokens_.push_back({ type, std::move(value), line_number, static_cast<uint32_t>(tokens_.size()) });
}

std::vector<Token> DronTokenizer::tokenizeFile(const std::string& file_location) {
    std::ifstream file(file_location);

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_location << "\n";
        return {};
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    
    return tokenizeString(buffer.str());
}

void DronTokenizer::handleMultiline(const std::string& trimmed) {
    // check if we are at the end
    if (trimmed == "\"\"\"") {
        emitToken(TokenType::MULTILINE_STRING, std::move(current_multiline_content_), line_number_);
        in_multiline_ = false;
        current_multiline_content_.clear();
    }
    // and if not accumulate the line
    else {
        if (!current_multiline_content_.empty()) current_multiline_content_ += '\n';
        current_multiline_content_ += trimmed;
    }
};

void DronTokenizer::handleSectionHeader(const std::string& trimmed) {
    /// we should have something like [Section Name]
    // verify it ends in ], or else we have and unknown but treat as find
    if (trimmed[trimmed.size() - 1] == ']') {
        // we have a section head that is valid
        emitToken(TokenType::SECTION_HEADER, std::move(trimmed.substr(1,trimmed.size()-2)), line_number_);
    }
    else {
        // we have a section that is not valid but started with a [ but doesn't end in one,
        //! let's emit it back but log an error, and not remove the last char
        std::cout << "Incorrect format for section header, tough likely you forgot it: " << line_number_ << ": " << trimmed << "\n";
        emitToken(TokenType::SECTION_HEADER, std::move(trimmed.substr(1, trimmed.size() - 1)), line_number_);
    }
};

void DronTokenizer::handleKeyValue(const std::string& trimmed) {
    // cool so we know that it didn't fit any of the other criteria so let's make sure it fits this one
    if (!isAlpha(trimmed[0])) {
        // not valid, should start with a char
        std::cout << "Not a valid key value, did not start with an alpha char: " 
            << line_number_ << ": " << trimmed << "\n";
        emitToken(TokenType::UNKNOWN, trimmed, line_number_);
        return;
    }

    // check if there is an =
    size_t equal_position = trimmed.find('=');
    if (equal_position == std::string::npos) {
        std::cout << "Not a valid key value, did not find an = sign: " 
            << line_number_ << ": " << trimmed << "\n";
        emitToken(TokenType::UNKNOWN, trimmed, line_number_);
        return;
    }

    std::string key = trimWhiteSpace(trimmed.substr(0, equal_position));
    std::string value = trimWhiteSpace(trimmed.substr(equal_position + 1));

    emitToken(TokenType::KEY, std::move(key), line_number_);
    emitToken(TokenType::EQUALS, "=", line_number_);

    if (value.empty()) return;
    tokenizeValue(value);
}

void DronTokenizer::tokenizeValue(const std::string& value) {
    // we need to walk the value sent and see what it is so we can act accordingly
    for (size_t position = 0; position < value.size(); ++position) {
        // blank, next
        if (value[position] == ' ') continue;

        // inline_comment
        if (value[position] == '#') {
            std::string token_string = value.substr(position);
            emitToken(TokenType::INLINE_COMMENT, std::move(token_string), line_number_);
            return;
        }

        // string time
        if (value[position] == '"') {
            if (value.substr(position, 3) == "\"\"\"") {
                in_multiline_ = true;
                return;
            }
            // increment once so we don't auto exit lol
            ++position;
            // great so we have a string start, now let's get to the end of the line
            std::string value_string{};
            while (position < value.size() && value[position] != '"') {
                value_string += value[position];
                ++position;
            }
            if (position >= value.size()) {
                // we didn't find a final ", so not valid
                emitToken(TokenType::UNKNOWN_VALUE, std::move(value_string), line_number_);
            }
            else {
                // all good here, our value was a string, emit it
                emitToken(TokenType::STRING, std::move(value_string.substr(0, value_string.size())), line_number_);
            }
            continue;
        }

        // now for the other possible symbols
        if (value[position] == '[') { emitToken(TokenType::LEFT_BRACKET, "[", line_number_); ++container_depth_; continue; }
        if (value[position] == ']') {
            emitToken(TokenType::RIGHT_BRACKET, "]", line_number_); 
            if (container_depth_ > 0) --container_depth_;
            continue;
        }
        if (value[position] == '{') { emitToken(TokenType::LEFT_BRACE, "{", line_number_); ++container_depth_; continue; }
        if (value[position] == '}') { 
            emitToken(TokenType::RIGHT_BRACE, "}", line_number_); 
            if (container_depth_ > 0) --container_depth_;
            continue; 
        }
        if (value[position] == ',') { emitToken(TokenType::COMMA, ",", line_number_); continue; }
        if (value[position] == '=') { emitToken(TokenType::EQUALS, "=", line_number_); continue; }

        // now for the values
        if (isAlpha(value[position])) {
            // so this is boolean, an identifier, or a key (if followed by an = sign)
            std::string word{};
            while (position < value.size() && (isAlpha(value[position]) || value[position] == '_' || isNumeric(value[position]))) {
                // collect the rest of the word, valid alpha, numeric, and _
                word += value[position];
                ++position;
            }
            // walk back to the last value as it was not dealt with but we did exit the loop;
            --position;

            if (word == "true" || word == "false") {
                emitToken(TokenType::BOOLEAN, std::move(word), line_number_);
            }
            else {
                emitToken(TokenType::IDENTIFIER, std::move(word), line_number_);
            }
            continue;
        }

        // now for int64_t vs double
        if (isNumeric(value[position]) || value[position] == '-' || value[position] == '.') {
            std::string number{};
            while (position < value.size() && (isNumeric(value[position]) || value[position] == '-' || value[position] == '.')) {
                number += value[position];
                ++position;
            }
            // walk back one
            --position;

            // check if it's a valid number
            if (number == "-" || number == ".") {
                std::cout << "Invalid numeric value at: " << line_number_ << ": " << number << "\n";
                emitToken(TokenType::UNKNOWN_VALUE, std::move(number), line_number_);
                continue;
            }

            // check the front ent for -. combo, and if so standardized to expected -0.
            if (number.size() >= 2 && number[0] == '-' && number[1] == '.') {
                number = "-0" + number.substr(1);
            }

            // check for a decimal and it there is one, it's a double, if not, integer
            if (getDecimalPosition(number) == std::string::npos) {
                emitToken(TokenType::INTEGER, std::move(number), line_number_);
            }
            else {
                emitToken(TokenType::DOUBLE, std::move(number), line_number_);
            }
            continue;
        }

        emitToken(TokenType::UNKNOWN_VALUE, std::string(1, value[position]), line_number_);
    }
}

std::vector<Token> DronTokenizer::tokenizeString(const std::string& raw_text) {
    
    // set up our token collector
    tokens_.clear();
    line_number_ = 0;
    in_multiline_ = false;
    current_multiline_content_.clear();
    container_depth_ = 0;

    // stream the text
    std::stringstream buffer(raw_text);
    std::string line{};

    while (std::getline(buffer, line)) {
        ++line_number_;
        std::string trimmed = trimWhiteSpace(line);

        if (in_multiline_) {
            handleMultiline(trimmed);
            continue;
        }

        if (container_depth_ > 0) {
            if (trimmed.empty()) {
                emitToken(TokenType::BLANK, std::move(line), line_number_);
            }
            else if (trimmed[0] == '#') {
                emitToken(TokenType::INLINE_COMMENT, std::move(trimmed), line_number_);
            }
            else {
                tokenizeValue(trimmed);
            }
            continue;
        }

        // easy one, is it empty
        if (trimmed.empty()) {
            emitToken(TokenType::BLANK, std::move(line), line_number_);
            continue;
        }

        // so at least one thing is in the trimmed string
        if (trimmed[0] == '#') {
            // we have a comment
            emitToken(TokenType::COMMENT, std::move(line), line_number_);
            continue;
        }

        if (trimmed[0] == '[') {
            // we have a section header
            handleSectionHeader(trimmed);
            continue;
        }

        if (trimmed == "\"\"\"") {
            in_multiline_ = true;
            current_multiline_content_.clear();
            continue;
        }

        // so this must be a key = value
        handleKeyValue(trimmed);
    }

    emitToken(TokenType::END_OF_FILE, "", line_number_);
    return tokens_;
}