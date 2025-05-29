#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>  // для getenv
#include <unistd.h> // для access, getcwd, chdir
#include <sys/wait.h> // для waitpid
#include <cstring>  // для strerror
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

// Функция для поиска исполняемого файла в PATH
string findInPath(const string& command) {
    const char* pathEnv = getenv("PATH");
    if (!pathEnv) {
        return "";
    }

    string pathStr = pathEnv;
    vector<string> directories;
    size_t pos = 0;
    string dir;
    while ((pos = pathStr.find(":")) != string::npos) {
        dir = pathStr.substr(0, pos);
        directories.push_back(dir);
        pathStr.erase(0, pos + 1);
    }
    directories.push_back(pathStr);

    for (const string& dir : directories) {
        string fullPath = dir + "/" + command;
        if (access(fullPath.c_str(), X_OK) == 0) {
            return fullPath;
        }
    }
    return "";
}

// Функция для выполнения внешних программ
void executeExternal(const vector<string>& args) {
    string path = findInPath(args[0]);
    if (path.empty()) {
        cout << args[0] << ": command not found\n";
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        cout << "fork failed: " << strerror(errno) << endl;
        return;
    }

    if (pid == 0) { // Дочерний процесс
        vector<char*> c_args;
        for (const auto& arg : args) {
            c_args.push_back(const_cast<char*>(arg.c_str()));
        }
        c_args.push_back(nullptr);

        execv(path.c_str(), c_args.data());
        // Если execv вернул управление, значит произошла ошибка
        cout << "exec failed: " << strerror(errno) << endl;
        exit(1);
    } else { // Родительский процесс
        int status;
        waitpid(pid, &status, 0);
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
    }

    string path = findInPath(commandToCheck);
    if (!path.empty()) {
        cout << commandToCheck << " is " << path << endl;
    } else {
        cout << commandToCheck << ": command not found" << endl;
    }
}

void runShell() {
    string command;
    
    map<string, CommandFunction> commandMap;
    commandMap["echo"] = echo;
    commandMap["exit"] = exitShell;
    commandMap["type"] = type;

    while (true) {
        cout << "$ ";
        cout.flush();
        getline(cin, command);

        if (command.empty()) {
            continue;
        }

        vector<string> tokens = split(command);
        if (tokens.empty()) {
            continue;
        }

        string cmdName = tokens[0];

        auto it = commandMap.find(cmdName);
        if (it != commandMap.end()) {
            it->second(tokens);
        } else {
            // Пытаемся выполнить как внешнюю команду
            executeExternal(tokens);
        }
    }
}

int main() {
    runShell();
    return 0;
}
