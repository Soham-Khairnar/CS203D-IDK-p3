#include "runner.hpp"
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    const std::string usage = "Usage: run --structure A|B < operations.txt\n";
    if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        std::cout << usage;
        return 0;
    }
    if (argc != 3 || std::string(argv[1]) != "--structure" ||
        (std::string(argv[2]) != "A" && std::string(argv[2]) != "B")) {
        std::cerr << usage;
        return 2;
    }
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    try {
        sensor::run(std::cin, std::cout,
            std::string(argv[2]) == "A" ? sensor::Structure::A : sensor::Structure::B);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
