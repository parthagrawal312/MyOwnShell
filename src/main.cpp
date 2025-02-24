#include <iostream>
#include <vector>
#include <sstream>
#include <filesystem>
#include <string>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
using namespace std;

extern char **environ;

void currentPathFinder() {
    string cwd = filesystem::current_path().string();
    cout << cwd << endl;
}

vector<string> split_string(const string &s, char delimiter) {
    stringstream ss(s);
    vector<string> return_vect;
    string token;
    while (getline(ss, token, delimiter)) {
        return_vect.push_back(token);
    }
    return return_vect;
}

string get_path(string command) {
    string path_env = getenv("PATH");
    stringstream ss(path_env);
    string path;
    while (getline(ss, path, ':')) {
        string abs_path = path + '/' + command;
        if (filesystem::exists(abs_path)) {
            return abs_path;
        }
    }
    return "";
}

vector<string> split_sentence(string input) {
    vector<string> userinput;
    string word = "";
    bool openquote = false;
    bool opendoublequote = false;
    bool keepNextCharSafe = false;

    for (size_t i = 0; i < input.length(); i++) {
        char c = input[i];

        if (keepNextCharSafe) {
            word += c;
            keepNextCharSafe = false;
            continue;
        }

        if (c == '\\') {
            if (opendoublequote) {
                if (i + 1 < input.length() && (input[i+1] == '\\' || input[i+1] == '$' || input[i+1] == '`' || input[i+1] == '"')) {
                    keepNextCharSafe = true;
                    continue;
                } else {
                    word += c;
                }
            } else if (openquote) {
                word += c;
            } else {
                if (i + 1 < input.length()) {
                    keepNextCharSafe = true;
                } else {
                    word += c;
                }
            }
            continue;
        }

        if (c == '\'' && !opendoublequote) {
            openquote = !openquote;
            continue;
        }

        if (c == '"' && !openquote) {
            opendoublequote = !opendoublequote;
            continue;
        }

        if (!openquote && !opendoublequote && c == ' ') {
            if (!word.empty()) {
                userinput.push_back(word);
                word = "";
            }
        } else {
            word += c;
        }
    }

    if (!word.empty()) {
        userinput.push_back(word);
    }

    return userinput;
}

void commandChecker(string s) {
    vector<string> builtInCommand = {"exit", "echo", "type", "pwd"};
    int flag = 0;
    for (int i = 0; i < builtInCommand.size(); i++) {
        if (s == builtInCommand[i]) {
            flag = 1;
            break;
        }
    }
    if (flag == 1) {
        cout << s << " is a shell builtin" << endl;
    } else {
        string path = get_path(s);
        if (path.empty()) {
            cout << s << " not found" << endl;
        } else {
            cout << s << " is " << path << endl;
        }
    }
}

int main() {
    cout << unitbuf;
    cerr << unitbuf;

    while (true) {
        cout << "$ ";
        string input;
        getline(cin, input);

        vector<string> userinput = split_sentence(input);
        if (userinput.empty()) continue;

        vector<string> args;
        string output_file;
        string error_file;

        for (size_t i = 0; i < userinput.size();) {
            if (userinput[i] == ">" || userinput[i] == "1>") {
                if (i + 1 >= userinput.size()) {
                    cerr << "Syntax error: no output file provided for redirection." << endl;
                    args.clear();
                    output_file.clear();
                    error_file.clear();
                    break;
                }
                output_file = userinput[i + 1];
                i += 2;
            } else if (userinput[i] == "2>") {
                if (i + 1 >= userinput.size()) {
                    cerr << "Syntax error: no error file provided for redirection." << endl;
                    args.clear();
                    output_file.clear();
                    error_file.clear();
                    break;
                }
                error_file = userinput[i + 1];
                i += 2;
            } else {
                args.push_back(userinput[i]);
                i++;
            }
        }

        if (args.empty()) continue;

        string command = args[0];

        if (command == "exit") {
            return 0;
        } else if (command == "echo") {
            int saved_stdout = -1;
            int saved_stderr = -1;
            bool redirect_failed = false;

            // Redirect stdout
            if (!output_file.empty()) {
                saved_stdout = dup(STDOUT_FILENO);
                if (saved_stdout == -1) {
                    perror("dup");
                    redirect_failed = true;
                } else {
                    int fd = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd == -1) {
                        perror("open");
                        redirect_failed = true;
                    } else {
                        if (dup2(fd, STDOUT_FILENO) == -1) {
                            perror("dup2");
                            close(fd);
                            redirect_failed = true;
                        } else {
                            close(fd);
                        }
                    }
                }
            }

            // Redirect stderr
            if (!error_file.empty() && !redirect_failed) {
                saved_stderr = dup(STDERR_FILENO);
                if (saved_stderr == -1) {
                    perror("dup stderr");
                    redirect_failed = true;
                } else {
                    int fd_err = open(error_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd_err == -1) {
                        perror("open stderr");
                        redirect_failed = true;
                    } else {
                        if (dup2(fd_err, STDERR_FILENO) == -1) {
                            perror("dup2 stderr");
                            close(fd_err);
                            redirect_failed = true;
                        } else {
                            close(fd_err);
                        }
                    }
                }
            }

            if (redirect_failed) {
                if (saved_stdout != -1) {
                    dup2(saved_stdout, STDOUT_FILENO);
                    close(saved_stdout);
                }
                if (saved_stderr != -1) {
                    dup2(saved_stderr, STDERR_FILENO);
                    close(saved_stderr);
                }
                continue;
            }

            if (args.size() > 1) {
                cout << args[1];
                for (size_t i = 2; i < args.size(); i++) {
                    cout << " " << args[i];
                }
            }
            cout << endl;

            if (saved_stdout != -1) {
                dup2(saved_stdout, STDOUT_FILENO);
                close(saved_stdout);
            }
            if (saved_stderr != -1) {
                dup2(saved_stderr, STDERR_FILENO);
                close(saved_stderr);
            }
        } else if (command == "type") {
            int saved_stdout = -1;
            int saved_stderr = -1;
            bool redirect_failed = false;

            // Redirect stdout
            if (!output_file.empty()) {
                saved_stdout = dup(STDOUT_FILENO);
                if (saved_stdout == -1) {
                    perror("dup stdout");
                    redirect_failed = true;
                } else {
                    int fd = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd == -1) {
                        perror("open stdout");
                        redirect_failed = true;
                    } else {
                        if (dup2(fd, STDOUT_FILENO) == -1) {
                            perror("dup2 stdout");
                            close(fd);
                            redirect_failed = true;
                        } else {
                            close(fd);
                        }
                    }
                }
            }

            // Redirect stderr
            if (!error_file.empty() && !redirect_failed) {
                saved_stderr = dup(STDERR_FILENO);
                if (saved_stderr == -1) {
                    perror("dup stderr");
                    redirect_failed = true;
                } else {
                    int fd_err = open(error_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd_err == -1) {
                        perror("open stderr");
                        redirect_failed = true;
                    } else {
                        if (dup2(fd_err, STDERR_FILENO) == -1) {
                            perror("dup2 stderr");
                            close(fd_err);
                            redirect_failed = true;
                        } else {
                            close(fd_err);
                        }
                    }
                }
            }

            if (redirect_failed) {
                if (saved_stdout != -1) {
                    dup2(saved_stdout, STDOUT_FILENO);
                    close(saved_stdout);
                }
                if (saved_stderr != -1) {
                    dup2(saved_stderr, STDERR_FILENO);
                    close(saved_stderr);
                }
                continue;
            }

            if (args.size() < 2) {
                cerr << "type: missing argument" << endl;
            } else {
                commandChecker(args[1]);
            }

            if (saved_stdout != -1) {
                dup2(saved_stdout, STDOUT_FILENO);
                close(saved_stdout);
            }
            if (saved_stderr != -1) {
                dup2(saved_stderr, STDERR_FILENO);
                close(saved_stderr);
            }
        } else if (command == "pwd") {
            int saved_stdout = -1;
            int saved_stderr = -1;
            bool redirect_failed = false;

            // Redirect stdout
            if (!output_file.empty()) {
                saved_stdout = dup(STDOUT_FILENO);
                if (saved_stdout == -1) {
                    perror("dup stdout");
                    redirect_failed = true;
                } else {
                    int fd = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd == -1) {
                        perror("open stdout");
                        redirect_failed = true;
                    } else {
                        if (dup2(fd, STDOUT_FILENO) == -1) {
                            perror("dup2 stdout");
                            close(fd);
                            redirect_failed = true;
                        } else {
                            close(fd);
                        }
                    }
                }
            }

            // Redirect stderr
            if (!error_file.empty() && !redirect_failed) {
                saved_stderr = dup(STDERR_FILENO);
                if (saved_stderr == -1) {
                    perror("dup stderr");
                    redirect_failed = true;
                } else {
                    int fd_err = open(error_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd_err == -1) {
                        perror("open stderr");
                        redirect_failed = true;
                    } else {
                        if (dup2(fd_err, STDERR_FILENO) == -1) {
                            perror("dup2 stderr");
                            close(fd_err);
                            redirect_failed = true;
                        } else {
                            close(fd_err);
                        }
                    }
                }
            }

            if (redirect_failed) {
                if (saved_stdout != -1) {
                    dup2(saved_stdout, STDOUT_FILENO);
                    close(saved_stdout);
                }
                if (saved_stderr != -1) {
                    dup2(saved_stderr, STDERR_FILENO);
                    close(saved_stderr);
                }
                continue;
            }

            currentPathFinder();

            if (saved_stdout != -1) {
                dup2(saved_stdout, STDOUT_FILENO);
                close(saved_stdout);
            }
            if (saved_stderr != -1) {
                dup2(saved_stderr, STDERR_FILENO);
                close(saved_stderr);
            }
        } else if (command == "cd") {
            int saved_stdout = -1;
            int saved_stderr = -1;
            bool redirect_failed = false;

            // Redirect stdout (though cd doesn't output to stdout)
            if (!output_file.empty()) {
                saved_stdout = dup(STDOUT_FILENO);
                if (saved_stdout == -1) {
                    perror("dup stdout");
                    redirect_failed = true;
                } else {
                    int fd = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd == -1) {
                        perror("open stdout");
                        redirect_failed = true;
                    } else {
                        if (dup2(fd, STDOUT_FILENO) == -1) {
                            perror("dup2 stdout");
                            close(fd);
                            redirect_failed = true;
                        } else {
                            close(fd);
                        }
                    }
                }
            }

            // Redirect stderr
            if (!error_file.empty() && !redirect_failed) {
                saved_stderr = dup(STDERR_FILENO);
                if (saved_stderr == -1) {
                    perror("dup stderr");
                    redirect_failed = true;
                } else {
                    int fd_err = open(error_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd_err == -1) {
                        perror("open stderr");
                        redirect_failed = true;
                    } else {
                        if (dup2(fd_err, STDERR_FILENO) == -1) {
                            perror("dup2 stderr");
                            close(fd_err);
                            redirect_failed = true;
                        } else {
                            close(fd_err);
                        }
                    }
                }
            }

            if (redirect_failed) {
                if (saved_stdout != -1) {
                    dup2(saved_stdout, STDOUT_FILENO);
                    close(saved_stdout);
                }
                if (saved_stderr != -1) {
                    dup2(saved_stderr, STDERR_FILENO);
                    close(saved_stderr);
                }
                continue;
            }

            string target_dir;
            if (args.size() < 2) {
                target_dir = getenv("HOME");
            } else {
                target_dir = args[1];
                if (target_dir == "~") {
                    target_dir = getenv("HOME");
                }
            }

            error_code ec;
            filesystem::current_path(target_dir, ec);
            if (ec) {
                cerr << "cd: " << target_dir << ": " << ec.message() << endl;
            }

            if (saved_stdout != -1) {
                dup2(saved_stdout, STDOUT_FILENO);
                close(saved_stdout);
            }
            if (saved_stderr != -1) {
                dup2(saved_stderr, STDERR_FILENO);
                close(saved_stderr);
            }
        } else {
            string path_string = getenv("PATH");
            vector<string> path = split_string(path_string, ':');
            string filepath;
            bool found = false;

            if (command.find('/') != string::npos) {
                if (filesystem::exists(command) && filesystem::is_regular_file(command)) {
                    filepath = command;
                    found = true;
                }
            } else {
                for (const auto& dir : path) {
                    filepath = dir + "/" + command;
                    if (filesystem::exists(filepath) && filesystem::is_regular_file(filepath)) {
                        found = true;
                        break;
                    }
                }
            }

            if (!found) {
                cout << command << ": not found" << endl;
                continue;
            }

            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                continue;
            } else if (pid == 0) {
                if (!output_file.empty()) {
                    int fd = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd == -1) {
                        perror("open");
                        exit(EXIT_FAILURE);
                    }
                    if (dup2(fd, STDOUT_FILENO) == -1) {
                        perror("dup2");
                        close(fd);
                        exit(EXIT_FAILURE);
                    }
                    close(fd);
                }

                if (!error_file.empty()) {
                    int fd_err = open(error_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd_err == -1) {
                        perror("open");
                        exit(EXIT_FAILURE);
                    }
                    if (dup2(fd_err, STDERR_FILENO) == -1) {
                        perror("dup2");
                        close(fd_err);
                        exit(EXIT_FAILURE);
                    }
                    close(fd_err);
                }

                vector<char*> argv;
                for (auto& arg : args) {
                    argv.push_back(const_cast<char*>(arg.c_str()));
                }
                argv.push_back(nullptr);

                execve(filepath.c_str(), argv.data(), environ);
                perror("execve");
                exit(EXIT_FAILURE);
            } else {
                int status;
                waitpid(pid, &status, 0);
            }
        }
    }
}