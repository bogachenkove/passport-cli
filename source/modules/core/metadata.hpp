#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>

#define ProductName "Passport CLI"
#define FileDescription "Secure Database"
#define ProductVersion "0.0.0.16"
#define License "MIT License"
#define LicenseFile "docs/LICENSE.txt"
#define Copyright "Copyright (c) 2026 Bogachenko Vyacheslav"
#define Author "Bogachenko Vyacheslav"
#define Contact "bogachenkove@outlook.com"
#define Homepage "https://github.com/bogachenkove/passport"

inline std::string read_license_file() {
	std::ifstream file(LicenseFile);
	if (!file.is_open()) {
		return
			"License file not found locally.\n"
			"Please read the license agreement online:\n"
			"https://raw.githubusercontent.com/bogachenkove/passport/stable/LICENSE.md";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}
inline bool handle_metadata_flags(int argc, char* argv[]) {
	for (int i = 1; i < argc; ++i) {
		if (std::strcmp(argv[i], "--about") == 0) {
			std::cout << ProductName << " "
				<< ProductVersion << " - " << FileDescription << "\n"
				<< Copyright << "\n\n"
				<< "Author:   " << Author << "\n"
				<< "Contact:  " << Contact << "\n"
				<< "Homepage: " << Homepage << std::endl;
			return true;
		}
		if (std::strcmp(argv[i], "--version") == 0) {
			std::cout << ProductName << " "
				<< ProductVersion << std::endl;
			return true;
		}
		if (std::strcmp(argv[i], "--license") == 0) {
			std::cout << ProductName << " "
				<< ProductVersion << " - " << FileDescription << "\n"
				<< Copyright << "\n\n"
				<< "This software is released under the "
				<< License << "."
				<< "\nYou are free to use, modify, and distribute it "
				<< "in accordance with the license terms.\n\n";
			std::cout << read_license_file() << std::endl;
			return true;
		}
		if (std::strcmp(argv[i], "--help") == 0) {
			std::cout << ProductName << " "
				<< ProductVersion << " - " << FileDescription << "\n"
				<< Copyright << "\n\n"
				<< "Available metadata flags:\n"
				<< "  --about     Show information about the program\n"
				<< "  --version   Show version information\n"
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