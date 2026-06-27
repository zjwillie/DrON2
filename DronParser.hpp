#pragma once

#include <vector>

#include "DronNode.hpp"
#include "DronToken.hpp"

class DronParser {
public:
	void parse(const std::vector<Token>& tokens);
	const DronDocument& getDocument() const { return document_; };

private:
	DronDocument document_{};
	const std::vector<Token>* tokens_ = nullptr;
	size_t position_ = 0;

	const Token& peek() const;
	const Token& advance();

	void parseSection(const std::string& section_name);
	void parseKeyValue(const std::string& key, DronMap& map);
	DronNode parseMap();
	DronNode parseList();
	DronNode parseValue();
};