#include "SerialManager.hh"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    SerialManager sm;

    if (!sm.Connect()) {
        std::cerr << "Failed to connect to serial.\n";
        return 1;
    }

    std::string line;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, line);
        if (line == "exit") break;

        sm.WriteLine(line);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        auto response = sm.GetBufferedResponse();
        if (response) {
            std::cout << "Response: " << *response << "\n";
        } else {
            std::cout << "(no response)\n";
        }
    }

    return 0;
}

