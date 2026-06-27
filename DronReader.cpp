#include <iostream>
#include <iomanip>
#include <sstream>

#include "DronReader.hpp"
#include "DronTokenizer.hpp"
#include "DronParser.hpp"

void DronReader::load(const std::string& path) {
	DronTokenizer tokenizer{};
	DronParser parser{};

	auto tokens = tokenizer.tokenizeFile(path);

	parser.parse(tokens);

	doc_ = parser.getDocument();
}

DronValue DronReader::operator[](std::string_view section) const {
	if (!contains(section)) {
		return DronValue{ nullptr, std::string(section) };
	}
	return DronValue{ &doc_.at(std::string(section)), std::string(section) };
}

bool DronReader::contains(std::string_view section) const {
	return doc_.contains(std::string(section));
}

std::string DronReader::toString() const {
    std::stringstream ss{};

    for (auto& [section, node] : doc_) {
        ss << "[" << section << "]\n";

        if (node.isMap()) {
            for (auto& [key, child] : node.as<DronMap>()) {
                ss << key << " = " << nodeToString(child, 0) << "\n";
            }
        }
        ss << "\n";
    }

    return ss.str();
}

std::string DronReader::nodeToString(const DronNode& node, int indent) {
    std::string pad(indent * 2, ' ');
    std::string childPad((indent + 1) * 2, ' ');
    std::stringstream ss{};

    if (node.is<int64_t>())          ss << node.as<int64_t>();
    else if (node.is<double>()) {
        double d = node.as<double>();
        std::stringstream ds{};
        ds << std::setprecision(15) << d;
        std::string s = ds.str();
        // if no decimal point got printed, add one so doubles are visually distinct
        if (s.find('.') == std::string::npos && s.find('e') == std::string::npos) {
            s += ".0";
        }
        ss << s;
    }
    else if (node.is<bool>())        ss << (node.as<bool>() ? "true" : "false");
    else if (node.is<std::string>()) ss << node.as<std::string>();
    else if (node.isList()) {
        const DronList& list = node.as<DronList>();
        if (list.empty()) {
            ss << "[]";
        }
        else {
            ss << "[\n";
            for (size_t i = 0; i < list.size(); ++i) {
                ss << childPad << nodeToString(list[i], indent + 1);
                if (i + 1 < list.size()) ss << ",";
                ss << "\n";
            }
            ss << pad << "]";
        }
    }
    else if (node.isMap()) {
        const DronMap& map = node.as<DronMap>();
        if (map.empty()) {
            ss << "{}";
        }
        else {
            ss << "{\n";
            for (auto& [key, child] : map) {
                ss << childPad << key << " = " << nodeToString(child, indent + 1) << "\n";
            }
            ss << pad << "}";
        }
    }

    return ss.str();
}

void DronReader::print() const {
	std::cout << toString();
}