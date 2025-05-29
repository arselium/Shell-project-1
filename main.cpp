#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>  // для getenv
#include <unistd.h> // для access, getcwd, chdir
#include <sys/wait.h> // для waitpid
#include <cstring>  // для strerror
using namespace std;

// Функция Split с учетом одинарных и двойных кавычек
vector<string> newSplit(const string& input) {
    vector<string> tokens;
    string token;
    bool in_single_quotes = false;
    bool in_double_quotes = false;

    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];
        // Если встретили ' вне ""
        if (c == '\'' && !in_double_quotes) {
            if (in_single_quotes) {
                // Закрываем одинарные кавычки
                tokens.push_back(token);
                token.clear();
                in_single_quotes = false;
            } else {
                // Открываем одинарные кавычки
                in_single_quotes = true;
            }
            continue;
        }
        // Если встретили " вне ''
        if (c == '"' && !in_single_quotes) {
            if (in_double_quotes) {
                // Закрываем двойные кавычки
                tokens.push_back(token);
                token.clear();
                in_double_quotes = false;
            } else {
                // Открываем двойные кавычки
                in_double_quotes = true;
            }
            continue;
        }

        if (c == ' ' && !in_single_quotes && !in_double_quotes) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
            continue;
        }

        token += c;
    }

    // Добавляем последний токен, если он есть
    if (!token.empty()) {
        tokens.push_back(token);
    }

    return tokens;
}

// Тип указателя на функцию
using CommandFunction = void(*)(const vector<string>&);

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
        cout << ": command not found\n";
    }
}

// Функция pwd
void pwd(const vector<string>& args) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        cout << cwd << endl;
    } else {
        cout << "pwd: error getting current directory" << endl;
    }
}

// Функция cd
void cd(const vector<string>& args) {
    string path;
    // Если аргумент не указан, переходим в HOME
    if (args.size() < 2) {
        const char* home = getenv("HOME");
        path = home;
    } else {
        path = args[1];
        // Обработка тильды
        if (path == "~") {
            const char* home = getenv("HOME");
            // home задан, но пусть будет если что
            if (home == nullptr) {
                cout << "cd: HOME not set" << endl;
                return;
            } // ---home задан---

            path = home;
        } else if (path.find("~/") == 0) {
            const char* home = getenv("HOME");
            path = string(home) + path.substr(1);
        }
    }

    // Проверка существования пути
    if (access(path.c_str(), F_OK) != 0) {
        cout << "cd: " << path << ": No such file or directory" << endl;
        return;
    }

    // Смена текущего каталога
    if (chdir(path.c_str()) != 0) { 
        cout << "cd: " << path << ": No such file or directory" << endl;
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
    commandMap["pwd"] = pwd;
    commandMap["cd"] = cd;

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
        cout << commandToCheck << ": not found" << endl;
    }
}

// Функция для запуска оболочки
void runShell() {
    string command;
    
    map<string, CommandFunction> commandMap;
    commandMap["echo"] = echo;
    commandMap["exit"] = exitShell;
    commandMap["type"] = type;
    commandMap["pwd"] = pwd;
    commandMap["cd"] = cd;
    
    while (true) {
        cout << "$ ";
        cout.flush(); // Гарантируем вывод приглашения
        getline(cin, command);

        if (command.empty()) {
            continue;
        }

        vector<string> tokens = newSplit(command);
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