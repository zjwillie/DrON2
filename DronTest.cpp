#include <iostream>

#include "DronNode.hpp"
#include "DronTests.hpp"
#include "DronReader.hpp"

namespace {
    int failures = 0;

    void check(bool condition, const std::string& label) {
        if (condition) {
            std::cout << "PASS: " << label << "\n";
        }
        else {
            std::cout << "FAIL: " << label << "\n";
            ++failures;
        }
    }
}

void DronTests::runAll() {
    DronReader reader("torture_test.dron");

    check(reader["Scalars"]["plain_int"].as<int64_t>(0) == 42, "plain_int");
    check(reader["Scalars"]["neg_int"].as<int64_t>(0) == -7, "neg_int");
    check(reader["Scalars"]["int64_max"].as<int64_t>(0) == 9223372036854775807LL, "int64_max");
    check(reader["Scalars"]["plain_double"].as<double>(0.0) == 3.14, "plain_double");
    check(reader["Scalars"]["true_val"].as<bool>(false) == true, "true_val");
    check(reader["Scalars"]["false_val"].as<bool>(true) == false, "false_val");
    check(reader["Scalars"]["quoted"].as<std::string>("") == "hello world", "quoted");
    check(reader["Scalars"]["ident"].as<std::string>("") == "LAYER_DYNAMIC", "ident");
    check(reader["BadNumbers"]["too_big_int"].as<std::string>("") == "99999999999999999999", "too_big_int as string");
    check(reader["BadNumbers"]["double_dash"].as<std::string>("") == "--5", "double_dash as string");
    check(reader["BadNumbers"]["lone_minus"].as<std::string>("") == "-", "lone_minus as string");
    check(reader["TrickyStrings"]["has_braces"].as<std::string>("") == "looks like {x = 1}", "has_braces");
    check(reader["TrickyStrings"]["has_equals"].as<std::string>("") == "a = b = c", "has_equals");
    check(reader["Lists"]["ints"][0].as<int64_t>(0) == 10, "ints[0]");
    check(reader["Lists"]["ints"][1].as<int64_t>(0) == 20, "ints[1]");
    check(reader["Lists"]["ints"][2].as<int64_t>(0) == 30, "ints[2]");
    check(reader["DeepNest"]["five_levels"]["l1"]["l2"]["l3"]["l4"]["l5"].as<std::string>("") == "reached", "five_levels deep");
    check(reader["DeepNest"]["list_of_maps"][0]["label"].as<std::string>("") == "first", "list_of_maps[0].label");
    check(reader["DeepNest"]["list_of_maps"][1]["id"].as<int64_t>(0) == 2, "list_of_maps[1].id");
    check(reader["Nope"]["missing"].as<int64_t>(99) == 99, "missing section returns fallback");
    check(reader["Scalars"]["plain_int"].as<std::string>("default") == "default", "wrong type returns fallback");
    check(reader["Lists"]["ints"][99].as<int64_t>(-1) == -1, "list index out of bounds returns fallback");
    check(reader["Scalars"]["int64_min"].as<int64_t>(0) == (-9223372036854775807LL - 1), "int64_min");
    check(reader["Scalars"]["zero_double"].as<double>(-1.0) == 0.0, "zero_double");
    check(reader["Scalars"]["leading_dot"].as<double>(-1.0) == 0.5, "leading_dot normalized");
    check(reader["Scalars"]["neg_leading"].as<double>(0.0) == -0.25, "neg_leading normalized");
    check(reader["Scalars"]["empty_string"].as<std::string>("x") == "", "empty_string");
    check(reader["Scalars"]["single_char"].as<std::string>("") == "x", "single_char");
    check(reader["TrickyStrings"]["has_hash"].as<std::string>("") == "color #FF0000", "has_hash");
    check(reader["TrickyStrings"]["has_brackets"].as<std::string>("") == "looks like [1, 2, 3]", "has_brackets");
    check(reader["TrickyStrings"]["has_commas"].as<std::string>("") == "one, two, three", "has_commas");
    check(reader["TrickyStrings"]["padded"].as<std::string>("") == "   spaced out   ", "padded keeps spaces");
    check(reader["Lists"]["mixed"][0].as<int64_t>(0) == 1, "mixed[0] int");
    check(reader["Lists"]["mixed"][1].as<double>(0.0) == 2.5, "mixed[1] double");
    check(reader["Lists"]["mixed"][2].as<std::string>("") == "three", "mixed[2] string");
    check(reader["Lists"]["mixed"][3].as<bool>(false) == true, "mixed[3] bool");
    check(reader["Lists"]["mixed"][4].as<std::string>("") == "IDENT", "mixed[4] ident");
    check(reader["Lists"]["nested_list"][0][0].as<int64_t>(0) == 1, "nested_list[0][0]");
    check(reader["Lists"]["nested_list"][1][1].as<int64_t>(0) == 4, "nested_list[1][1]");
    check(reader["Maps"]["flat"]["b"].as<int64_t>(0) == 2, "flat.b");
    check(reader["Maps"]["typed"]["speed"].as<double>(0.0) == 4.5, "typed.speed");
    check(reader["Maps"]["typed"]["alive"].as<bool>(false) == true, "typed.alive");
    check(reader["Maps"]["nested_map"]["outer"]["inner"]["deep"].as<std::string>("") == "bottom", "nested_map deep");
    int emptyListCount = 0;
    for (auto e : reader["Lists"]["empty_list"].elements()) { (void)e; ++emptyListCount; }
    check(emptyListCount == 0, "empty_list yields nothing");
    int emptyMapCount = 0;
    for (auto [k, v] : reader["Maps"]["empty_map"].items()) { (void)k; (void)v; ++emptyMapCount; }
    check(emptyMapCount == 0, "empty_map yields nothing");
    int flatCount = 0;
    for (auto [k, v] : reader["Maps"]["flat"].items()) { (void)k; (void)v; ++flatCount; }
    check(flatCount == 3, "flat has 3 entries");
    check(reader.contains("Scalars") == true, "reader contains Scalars");
    check(reader.contains("Nope") == false, "reader does not contain Nope");
    check(reader["Maps"]["flat"].contains("a") == true, "flat contains a");
    check(reader["Maps"]["flat"].contains("zzz") == false, "flat does not contain zzz");
    check(reader["Scalars"].valid() == true, "Scalars valid");
    check(reader["Nope"].valid() == false, "Nope invalid");
    check(reader["Lists"]["ints"].isList() == true, "ints isList");
    check(reader["Maps"]["flat"].isMap() == true, "flat isMap");
    check(reader["Scalars"]["plain_int"].is<int64_t>() == true, "plain_int is int64");
    int sectionCount = 0;
    for (auto& [name, node] : reader) { (void)name; (void)node; ++sectionCount; }
    check(sectionCount > 0, "reader iterates sections");

    int count = 0;
    for (auto element : reader["Lists"]["ints"].elements()) {
        (void)element;
        ++count;
    }
    check(count == 3, "elements() yields 3 items");

    check(reader["Multiline"]["normal"].as<std::string>("") == "line one\nline two\nline three", "multiline normal");
    check(reader["Multiline"]["single"].as<std::string>("") == "just one", "multiline single");

    std::cout << "\n" << failures << " failure(s)\n";
}