#include <iostream>
#include <string>
#include <vector>
using namespace std;

void runShell() {
    std::string command;
    while (true)
    {
        std::cout << "$ ";
        std::getline(std::cin, command);
        std::cout << command << ": command not found";
    std::cout << std::endl;
    }
}

int main() {
    runShell();
    return 0;
}
