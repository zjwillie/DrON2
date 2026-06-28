// DrON2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "DronNode.hpp"
#include "DronTests.hpp"
#include "DronReader.hpp"

int main()
{
	DronReader reader("torture_test.dron");
	reader.print();

	int64_t integer = reader["Sprite"]["width"].as<int64_t>(0);
	std::cout << integer;

	DronTests::runAll();
}