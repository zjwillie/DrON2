#pragma once

#include <string>
#include "DronNode.hpp"
#include "DronValue.hpp"

class DronReader {
public:
    DronReader() = default;
    DronReader(const std::string& path) { load(path); }

    void load(const std::string& path);                     // parse
    void print() const;                                     // see
    std::string toString() const;

    DronValue operator[](std::string_view section) const;   // deliver
    bool contains(std::string_view section) const;

    // iterate section headers
    auto begin() const { return doc_.begin(); }
    auto end()   const { return doc_.end(); }

private:
    DronDocument doc_{};

    static std::string nodeToString(const DronNode& node, int indent);
};