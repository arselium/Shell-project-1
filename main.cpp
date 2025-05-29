#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>  // для getenv
using namespace std;

// Указатель на функцию
using CommandFunction = void(*)(const vector<string>&);

// Функция split
vector<string> split(const string& input) {
    vector<string> tokens;
    string token;

    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];
        if (c == ' ') {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
            continue;
        }
        token += c;
    }

    if (!token.empty()) {
        tokens.push_back(token);
    }

    return tokens;
}

// Функция echo
void echo(const vector<string>& args) {
    for (size_t i = 1; i < args.size(); ++i) {
        cout << args[i];
        if (i < args.size() - 1) {
            cout << " ";
        }
    }
    cout << endl;
}

// Функция exit
void exitShell(const vector<string>& args) {
    if (args.size() > 1 && args[1] == "0") {
        exit(0);
    } else {
        cout << "command not found\n";
    }
}

// Функция type
void type(const vector<string>& args) {
    if (args.size() < 2) {
        cout << "type: missing argument" << endl;
        return;
    }

    map<string, CommandFunction> commandMap;
    commandMap["echo"] = echo;
    commandMap["exit"] = exitShell;
    commandMap["type"] = type;

    string commandToCheck = args[1];
    
    auto it = commandMap.find(commandToCheck);
    if (it != commandMap.end()) {
        cout << commandToCheck << " is a shell builtin" << endl;
        return;
    } else {
        cout << commandToCheck << ": command not found" << endl;
    }

    /*string path = findInPath(commandToCheck);
    if (!path.empty()) {
        cout << commandToCheck << " is " << path << endl;
    } else {
        cout << commandToCheck << ": not found" << endl;
    }*/
}

void runShell() {
    string command;
    
    map<string, CommandFunction> commandMap;
    commandMap["echo"] = echo;
    commandMap["exit"] = exitShell;
    commandMap["type"] = type;

    while (true) {
        cout << "$ ";
        getline(cin, command);

        if (command.empty()) {
            continue;
        }

        vector<string> tokens = split(command);
        string cmdName = tokens[0];

        auto it = commandMap.find(cmdName);
        if (it != commandMap.end()) {
            it->second(tokens);
        } else {
            std::cout << command << ": command not found";
            std::cout << std::endl;
        }
    }
}

int main() {
    runShell();
    return 0;
}
