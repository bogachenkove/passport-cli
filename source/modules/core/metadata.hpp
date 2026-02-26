#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#define ProductName "Passport CLI"
#define FileDescription "Secure Password Database"
#define ProductVersion "0.0.0.10"
#define License "MIT License"
#define LicenseFile "LICENSE.txt"
#define Copyright "Copyright (c) 2026 Bogachenko Vyacheslav. All rights reserved."
#define Author "Bogachenko Vyacheslav"
#define Contact "bogachenkove@outlook.com"
#define Homepage "https://github.com/bogachenkove/passport"

inline std::string read_license_file() {
	std::ifstream file(LicenseFile);
	if (!file.is_open()) {
		return "Please read the license agreement at https://raw.githubusercontent.com/bogachenkove/passport/stable/LICENSE.md";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}
inline bool handle_metadata_flags(int argc, char* argv[]) {
	for (int i = 1; i < argc; ++i) {
		if (std::strcmp(argv[i], "--version") == 0) {
			std::cout << ProductName << ", version: " << ProductVersion << std::endl;
			return true;
		}
		if (std::strcmp(argv[i], "--homepage") == 0) {
			std::cout << Homepage << std::endl;
			return true;
		}
		if (std::strcmp(argv[i], "--license") == 0) {
			std::cout << ProductName << "\n" << Copyright <<
				"\nThis program is free software released under the " << License
				"\nUse, modification and distribution are permitted under the terms of the license below.\n\n";
			std::cout << read_license_file() << std::endl;
			return true;
		}
		if (std::strcmp(argv[i], "--help") == 0) {
			std::cout << "Available metadata flags:\n"
				<< "  --version   Show version information\n"
				<< "  --homepage  Show project homepage URL\n"
				<< "  --license   Show license information\n"
				<< "  --help      Show this help message\n";
			return true;
		}
		if (argv[i][0] == '-') {
			std::cerr << "Unknown option: " << argv[i] << "\n";
			std::cerr << "Try '--help' for more information.\n";
			return true;
		}
	}
	return false;
}