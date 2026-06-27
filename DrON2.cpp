// DrON2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "DronNode.hpp"
#include "DronTests.hpp"
#include "DronReader.hpp"

int main()
{
	DronReader reader("serialized_test.dron");
	//reader.print();

	for (auto [key, value] : reader["Maps"]["inline_map"].items()) {
		std::cout << key << " = " << value.as<int64_t>(0) << "\n";
	}
}