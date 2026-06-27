#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <variant>
#include <vector>

struct DronNode;

using DronList = std::vector<DronNode>;
using DronMap = std::map<std::string, DronNode>;

struct DronNode {
    // Integers are stored as int64_t. Read them with as<int64_t>(); cast to int
    // at the call site if a consumer needs a narrower type. There is no implicit
    // narrowing - ask for the type that's actually stored.
    std::variant<int64_t,
        double, bool,
        std::string,
        DronList,
        DronMap> value;

    // Valid T: int64_t, double, bool, std::string. Ask for the type the variant
    // stores - there is no narrowing or conversion. Passing int or const char*
    // will never match and will always return the fallback.
    template<typename T>
    const T& as() const { return std::get<T>(value); }

    template<typename T>
    T& as() { return std::get<T>(value); }

    template<typename T>
    bool is() const { return std::holds_alternative<T>(value); }

    bool isMap() const { return std::holds_alternative<DronMap>(value); }
    bool isList() const { return std::holds_alternative<DronList>(value); }
};

using DronDocument = DronMap;