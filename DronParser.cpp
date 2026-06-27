#include "DronParser.hpp"

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
			dronList.push_back(parseValue());
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
		return DronNode{ std::stod(token.value) };
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
	default: {
		Token token = advance();
		return DronNode{ token.value };
	}
	}
}

void DronParser::parseKeyValue(const std::string& key, DronMap& map) {
	// consume the equals
	advance();
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