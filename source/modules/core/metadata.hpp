#pragma once
#include <iostream>
#include <cstring>
#define ProductName "Passport CLI"
#define FileDescription "Secure Password Manager"
#define ProductVersion "0.0.0.10"
#define License "MIT License"
#define Copyright "Copyright © 2026 Bogachenko Vyacheslav. All rights reserved."
#define Author "Bogachenko Vyacheslav"
#define Contact "bogachenkove@outlook.com"
#define Homepage "https://github.com/bogachenkove/passport-cli"

inline bool handle_metadata_flags(int argc, char* argv[]) {
	for (int i = 1; i < argc; ++i) {
		if (std::strcmp(argv[i], "--version") == 0) {
			std::cout << ProductName << " version " << ProductVersion << std::endl;
			return true;
		}
		if (std::strcmp(argv[i], "--homepage") == 0) {
			std::cout << Homepage << std::endl;
			return true;
		}
		if (std::strcmp(argv[i], "--license") == 0) {
			std::cout << License << std::endl;
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
	}
	return false;
}