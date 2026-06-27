#include <iostream>

#include "DronNode.hpp"
#include "DronTests.hpp"

void DronTests::testDronNode() {
    // --- scalars ---
    DronNode i;
    i.value = int64_t{ 42 };
    std::cout << i.as<int64_t>() << "\n";          // 42
    std::cout << i.is<int64_t>() << "\n";          // 1
    std::cout << i.is<double>() << "\n";           // 0

    // --- building a map by hand (like a parsed section) ---
    DronMap transform;
    transform["x"] = DronNode{ double{160.0} };
    transform["y"] = DronNode{ double{90.0} };
    transform["rotation"] = DronNode{ double{0.0} };

    DronNode section;
    section.value = transform;
    std::cout << section.isMap() << "\n";                           // 1
    std::cout << section.as<DronMap>()["x"].as<double>() << "\n";  // 160

    // --- building a document ---
    DronDocument doc;
    doc["Transform"] = section;

    // round-trip the x value back out
    double x = doc["Transform"].as<DronMap>()["x"].as<double>();
    double y = doc["Transform"].as<DronMap>()["y"].as<double>();
    std::cout << "Transform.x: " << x << "  .y: " << y << "\n";  // 160

    // --- list ---
    DronList tags;
    tags.push_back(DronNode{ std::string{"asteroid"} });
    tags.push_back(DronNode{ std::string{"large"} });

    DronNode tagNode;
    tagNode.value = tags;
    std::cout << tagNode.isList() << "\n";                              // 1
    std::cout << tagNode.as<DronList>()[0].as<std::string>() << "\n";  // asteroid

    // --- what happens on a type mismatch ---
    try {
        i.as<std::string>();  // i holds int64, this throws
    }
    catch (const std::bad_variant_access& e) {
        std::cout << "caught: " << e.what() << "\n";
    }
}