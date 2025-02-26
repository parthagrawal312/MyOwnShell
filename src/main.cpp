#include <iostream>
#include <vector>
#include <sstream>
#include <filesystem>
#include <string>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include <algorithm>
using namespace std;

extern char **environ;

struct termios orig_termios;

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

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
  bool openquote = false;  // for single quote
  bool opendoublequote = false;  // for double quote
  bool keepNextCharSafe = false;  // for escaped characters

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
  vector<string> builtInCommand = {"exit", "echo", "type", "pwd", "cd"};
  int flag = 0;
  for (size_t i = 0; i < builtInCommand.size(); i++) {
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

    enable_raw_mode();

    while (true) {
        cout << "$ " << flush;

        string input_buffer;
        char c;
        int tab_press_count = 0;

        while (read(STDIN_FILENO, &c, 1) == 1) {
            if (c == '\n') {
                cout << endl;
                break;
            } else if (c == '\t') {
                tab_press_count++;
                size_t first_space = input_buffer.find(' ');
                // Consider only the first token (command) for tab completion.
                string current_command = input_buffer.substr(0, first_space);

                // First check built-in commands.
                vector<string> builtin_matches;
                vector<string> builtins = {"echo", "exit", "type", "pwd", "cd"};
                for (const auto &b : builtins) {
                    if (b.find(current_command) == 0) {
                        builtin_matches.push_back(b);
                    }
                }
                sort(builtin_matches.begin(), builtin_matches.end());

                if (!builtin_matches.empty()) {
                    if (builtin_matches.size() == 1) {
                        string completed = builtin_matches[0];
                        string new_buffer;
                        if (first_space != string::npos) {
                            new_buffer = completed + " " + input_buffer.substr(first_space + 1);
                        } else {
                            new_buffer = completed + " ";
                        }
                        input_buffer = new_buffer;
                        cout << "\r\033[K$ " << input_buffer << flush;
                        tab_press_count = 0;
                    } else {
                        if (tab_press_count == 1) {
                            cout << '\a' << flush;
                        } else if (tab_press_count >= 2) {
                            cout << "\n";
                            for (size_t i = 0; i < builtin_matches.size(); ++i) {
                                if (i > 0) cout << "  ";
                                cout << builtin_matches[i];
                            }
                            cout << "\n$ " << input_buffer << flush;
                            tab_press_count = 0;
                        }
                    }
                } else {
                    // If no built-ins match, check for external executables.
                    vector<string> external_matches;
                    string path_env = getenv("PATH");
                    vector<string> path_dirs = split_string(path_env, ':');

                    for (const auto &dir : path_dirs) {
                        if (!filesystem::exists(dir) || !filesystem::is_directory(dir)) {
                            continue;
                        }
                        error_code ec;
                        for (const auto &entry : filesystem::directory_iterator(dir, ec)) {
                            if (ec) continue;
                            if (!entry.is_regular_file()) continue;
                            string filepath = entry.path().string();
                            if (access(filepath.c_str(), X_OK) != 0) continue;
                            string filename = entry.path().filename().string();
                            if (filename.find(current_command) == 0) {
                                if (find(external_matches.begin(), external_matches.end(), filename) == external_matches.end())
                                    external_matches.push_back(filename);
                            }
                        }
                    }
                    sort(external_matches.begin(), external_matches.end());

                    if (external_matches.empty()) {
                        cout << '\a' << flush;
                        tab_press_count = 0;
                    } else if (external_matches.size() == 1) {
                        string completed = external_matches[0];
                        string new_buffer;
                        if (first_space != string::npos) {
                            new_buffer = completed + " " + input_buffer.substr(first_space + 1);
                        } else {
                            new_buffer = completed + " ";
                        }
                        input_buffer = new_buffer;
                        cout << "\r\033[K$ " << input_buffer << flush;
                        tab_press_count = 0;
                    } else {
                        if (tab_press_count == 1) {
                            cout << '\a' << flush;
                        } else if (tab_press_count >= 2) {
                            cout << "\n";
                            for (size_t i = 0; i < external_matches.size(); ++i) {
                                if (i > 0) cout << "  ";
                                cout << external_matches[i];
                            }
                            cout << "\n$ " << input_buffer << flush;
                            tab_press_count = 0;
                        }
                    }
                }
            } else if (c == 127) { // Backspace
                tab_press_count = 0;
                if (!input_buffer.empty()) {
                    input_buffer.pop_back();
                    cout << "\r\033[K$ " << input_buffer << flush;
                }
            } else {
                tab_press_count = 0;
                input_buffer += c;
                cout << c << flush;
            }
        }

        vector<string> userinput = split_sentence(input_buffer);
        if (userinput.empty()) continue;

        vector<string> args;
        string output_file;
        string error_file;
        bool output_append = false;
        bool error_append = false;

        for (size_t i = 0; i < userinput.size();) {
            if (userinput[i] == ">" || userinput[i] == "1>") {
                if (i + 1 >= userinput.size()) {
                    cerr << "Syntax error: no output file provided for redirection." << endl;
                    args.clear();
                    output_file.clear();
                    error_file.clear();
                    output_append = false;
                    error_append = false;
                    break;
                }
                output_file = userinput[i + 1];
                output_append = false;
                i += 2;
            } else if (userinput[i] == ">>" || userinput[i] == "1>>") {
                if (i + 1 >= userinput.size()) {
                    cerr << "Syntax error: no output file provided for redirection." << endl;
                    args.clear();
                    output_file.clear();
                    error_file.clear();
                    output_append = false;
                    error_append = false;
                    break;
                }
                output_file = userinput[i + 1];
                output_append = true;
                i += 2;
            } else if (userinput[i] == "2>") {
                if (i + 1 >= userinput.size()) {
                    cerr << "Syntax error: no error file provided for redirection." << endl;
                    args.clear();
                    output_file.clear();
                    error_file.clear();
                    output_append = false;
                    error_append = false;
                    break;
                }
                error_file = userinput[i + 1];
                error_append = false;
                i += 2;
            } else if (userinput[i] == "2>>") {
                if (i + 1 >= userinput.size()) {
                    cerr << "Syntax error: no error file provided for redirection." << endl;
                    args.clear();
                    output_file.clear();
                    error_file.clear();
                    output_append = false;
                    error_append = false;
                    break;
                }
                error_file = userinput[i + 1];
                error_append = true;
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

            if (!output_file.empty()) {
                saved_stdout = dup(STDOUT_FILENO);
                if (saved_stdout == -1) {
                    perror("dup");
                    redirect_failed = true;
                } else {
                    int flags = O_WRONLY | O_CREAT;
                    if (output_append)
                        flags |= O_APPEND;
                    else
                        flags |= O_TRUNC;
                    int fd = open(output_file.c_str(), flags, 0644);
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

            if (!error_file.empty() && !redirect_failed) {
                saved_stderr = dup(STDERR_FILENO);
                if (saved_stderr == -1) {
                    perror("dup stderr");
                    redirect_failed = true;
                } else {
                    int flags_err = O_WRONLY | O_CREAT;
                    if (error_append)
                        flags_err |= O_APPEND;
                    else
                        flags_err |= O_TRUNC;
                    int fd_err = open(error_file.c_str(), flags_err, 0644);
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

            if (!output_file.empty()) {
                saved_stdout = dup(STDOUT_FILENO);
                if (saved_stdout == -1) {
                    perror("dup stdout");
                    redirect_failed = true;
                } else {
                    int flags = O_WRONLY | O_CREAT;
                    if (output_append)
                        flags |= O_APPEND;
                    else
                        flags |= O_TRUNC;
                    int fd = open(output_file.c_str(), flags, 0644);
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

            if (!error_file.empty() && !redirect_failed) {
                saved_stderr = dup(STDERR_FILENO);
                if (saved_stderr == -1) {
                    perror("dup stderr");
                    redirect_failed = true;
                } else {
                    int flags_err = O_WRONLY | O_CREAT;
                    if (error_append)
                        flags_err |= O_APPEND;
                    else
                        flags_err |= O_TRUNC;
                    int fd_err = open(error_file.c_str(), flags_err, 0644);
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

            if (!output_file.empty()) {
                saved_stdout = dup(STDOUT_FILENO);
                if (saved_stdout == -1) {
                    perror("dup stdout");
                    redirect_failed = true;
                } else {
                    int flags = O_WRONLY | O_CREAT;
                    if (output_append)
                        flags |= O_APPEND;
                    else
                        flags |= O_TRUNC;
                    int fd = open(output_file.c_str(), flags, 0644);
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

            if (!error_file.empty() && !redirect_failed) {
                saved_stderr = dup(STDERR_FILENO);
                if (saved_stderr == -1) {
                    perror("dup stderr");
                    redirect_failed = true;
                } else {
                    int flags_err = O_WRONLY | O_CREAT;
                    if (error_append)
                        flags_err |= O_APPEND;
                    else
                        flags_err |= O_TRUNC;
                    int fd_err = open(error_file.c_str(), flags_err, 0644);
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

            if (!output_file.empty()) {
                saved_stdout = dup(STDOUT_FILENO);
                if (saved_stdout == -1) {
                    perror("dup stdout");
                    redirect_failed = true;
                } else {
                    int flags = O_WRONLY | O_CREAT;
                    if (output_append)
                        flags |= O_APPEND;
                    else
                        flags |= O_TRUNC;
                    int fd = open(output_file.c_str(), flags, 0644);
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

            if (!error_file.empty() && !redirect_failed) {
                saved_stderr = dup(STDERR_FILENO);
                if (saved_stderr == -1) {
                    perror("dup stderr");
                    redirect_failed = true;
                } else {
                    int flags_err = O_WRONLY | O_CREAT;
                    if (error_append)
                        flags_err |= O_APPEND;
                    else
                        flags_err |= O_TRUNC;
                    int fd_err = open(error_file.c_str(), flags_err, 0644);
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
            // External command execution.
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
                for (const auto &dir : path) {
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
                    int flags = O_WRONLY | O_CREAT;
                    if (output_append)
                        flags |= O_APPEND;
                    else
                        flags |= O_TRUNC;
                    int fd = open(output_file.c_str(), flags, 0644);
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
                    int flags_err = O_WRONLY | O_CREAT;
                    if (error_append)
                        flags_err |= O_APPEND;
                    else
                        flags_err |= O_TRUNC;
                    int fd_err = open(error_file.c_str(), flags_err, 0644);
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
                for (auto &arg : args) {
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

    return 0;
}
