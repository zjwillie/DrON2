#include "DronParser.hpp"

#include <utility>

const Token& DronParser::peek() const {
	if (position_ >= tokens_->size()) return tokens_->back();
	return (*tokens_)[position_];
}

const Token& DronParser::advance() {
	if (position_ >= tokens_->size()) return tokens_->back();
	return (*tokens_)[position_++];
}

DronNode DronParser::parseMap() {
	DronMap dronMap{};
	// consume the {
	advance();
	while (peek().type != TokenType::RIGHT_BRACE && peek().type != TokenType::END_OF_FILE) {
		if (peek().type == TokenType::COMMA ||
			peek().type == TokenType::INLINE_COMMENT ||
			peek().type == TokenType::COMMENT ||
			peek().type == TokenType::BLANK) {
			advance();
			continue;
		}
		else {
			Token key = advance();
			if (peek().type == TokenType::EQUALS) advance();  // consume = only if present
			dronMap[key.value] = parseValue();
		}
	}
	//consume the }
	advance();
	return DronNode{ dronMap };
}

DronNode DronParser::parseList() {
	DronList dronList{};
	// consume the [
	advance();
	while (peek().type != TokenType::RIGHT_BRACKET && peek().type != TokenType::END_OF_FILE) {
		if (peek().type == TokenType::COMMA ||
			peek().type == TokenType::INLINE_COMMENT ||
			peek().type == TokenType::COMMENT ||
			peek().type == TokenType::BLANK) {
			advance();
			continue;
		}
		else {
			size_t before = position_;
			DronNode value = parseValue();
			if (position_ == before) {
				// parseValue refused a stray non-value token (e.g. a '}' inside
				// a '['); skip it so the loop can't spin forever.
				advance();
				continue;
			}
			dronList.push_back(std::move(value));
		}
	}
	//consume the ]
	advance();
	return DronNode{ dronList };
}

DronNode DronParser::parseValue() {
	switch (peek().type) {
	case TokenType::INTEGER: {
		Token token = advance();
		try {
			return DronNode{ std::stoll(token.value) };
		}
		catch (const std::exception&) {
			// not a representable int64 - keep the raw text so it's visible
			return DronNode{ token.value };
		}
	}
	case TokenType::DOUBLE:	{
		Token token = advance();
		try {
			return DronNode{ std::stod(token.value) };
		}
		catch (const std::exception&) {
			// not a representable double - keep the raw text so it's visible
			return DronNode{ token.value };
		}
	}
	case TokenType::BOOLEAN: {
		Token token = advance();
		if (token.value == "true") {
			return DronNode{ true };
		}
		return DronNode{ false };
	}
	case TokenType::STRING:	{
		Token token = advance();
		return DronNode{ token.value };
	}
	case TokenType::MULTILINE_STRING: {
		Token token = advance();
		return DronNode{ token.value };
	}
	case TokenType::IDENTIFIER: {
		Token token = advance();
		return DronNode{ token.value };
	}
	case TokenType::LEFT_BRACE: {
		return parseMap();
	}
	case TokenType::LEFT_BRACKET: {
		return parseList();
	}
	case TokenType::RIGHT_BRACE:
	case TokenType::RIGHT_BRACKET:
	case TokenType::SECTION_HEADER:
	case TokenType::KEY:
	case TokenType::END_OF_FILE: {
		// No value present (e.g. "key =" or "{ x }"). Do NOT consume the token,
		// so the enclosing container/section loop sees its own terminator and
		// the next key/section isn't swallowed.
		return DronNode{ std::string{} };
	}
	default: {
		Token token = advance();
		return DronNode{ token.value };
	}
	}
}

void DronParser::parseKeyValue(const std::string& key, DronMap& map) {
	if (peek().type == TokenType::EQUALS) advance();  // consume = only if present
	map[key] = parseValue();
}

void DronParser::parseSection(const std::string& section_name) {
	DronMap dronMap{};
	while (peek().type != TokenType::END_OF_FILE && peek().type != TokenType::SECTION_HEADER) {
		const Token& token = advance();
		if (token.type == TokenType::KEY) {
			parseKeyValue(token.value, dronMap);
		}
	}
	document_[section_name] = DronNode{ dronMap };
}

void DronParser::parse(const std::vector<Token>& tokens) {
	if (tokens.empty()) return;
	tokens_ = &tokens;
	position_ = 0;
	document_.clear();

	while (peek().type != TokenType::END_OF_FILE) {
		const Token& token = advance();
		if (token.type == TokenType::SECTION_HEADER) {
			parseSection(token.value);
		}
	}
}