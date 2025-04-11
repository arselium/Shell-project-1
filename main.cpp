#include <iostream>
#include <string>
#include <vector>
using namespace std;


// Split(строка, разделитель)
std::vector<std::string> split(std::string& s, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    tokens.push_back(s);

    /* Вывод токенов
    vector t = split(command, " ");
        for (int i = 1; i < t.size(); i++){
            std::cout << t[i] << " ";
        }*/

    return tokens;
}

void echo(string command){
       cout << command.substr(5);
}



void runShell() {
    std::string command;
    while (true) {
        std::cout << "$ ";
        std::getline(std::cin, command);
        //std::cin >> command;
        //if (command == "exit 0") {break;}
        /*else*/ if  (command.empty()) {continue;}
        else {std::cout << command << ": command not found";}
        //echo(command);
        

    std::cout << std::endl;
    }

}


int main() {

    runShell();

    return 0;
}
