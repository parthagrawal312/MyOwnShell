#include <iostream>
#include <vector>
#include <sstream>
#include <filesystem>
#include <string>
#include <fstream>
using namespace std;


void currentPathFinder(){
  string cwd = std::filesystem::current_path();
  string print_cwd = cwd.substr(0, cwd.length());
  cout << print_cwd << "\n";
}

vector<string> split_string(const string &s, char delimiter){
    stringstream ss(s);
    vector<string> return_vect;
    string token;
    while(getline(ss, token, delimiter)){
      return_vect.push_back(token);
    }
    return return_vect;
  }


string get_path(string command){
  string path_env = getenv("PATH");
  stringstream ss(path_env);
  string path;
   while(!ss.eof()){
      getline(ss, path, ':');

        string abs_path = path + '/' + command;

        if(filesystem::exists(abs_path)){
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

        // Handle escape sequences
        if (keepNextCharSafe) {
            word += c;  // Add the escaped character to the word
            keepNextCharSafe = false;  // Reset the escape flag
            continue;
        }

        // Handle backslashes inside double quotes (escape special characters)
        if (c == '\\' && opendoublequote) {
            if (i + 1 < input.length() && (input[i + 1] == '\\' || input[i + 1] == '$' || input[i + 1] == '`' || input[i + 1] == '"')) {
                keepNextCharSafe = true;  // Mark next character as escaped
                continue;
            } else {
                word += c;  // Treat backslash as a regular character if not followed by an escape sequence
                continue;
            }
        }

        // Handle backslashes inside single quotes (escape backslashes)
        if (c == '\\' && openquote) {
            word += c;  // Treat the backslash as part of the word in single quotes
            continue;
        }

        // Handle single quotes (toggle openquote)
        if (c == '\'' && !opendoublequote) {
            openquote = !openquote;
            continue;
        }

        // Handle double quotes (toggle opendoublequote)
        if (c == '"' && !openquote) {
            opendoublequote = !opendoublequote;
            continue;
        }

        // Space handling (only split if not inside quotes)
        if (!openquote && !opendoublequote && c == ' ') {
            if (!word.empty()) {
                userinput.push_back(word);  // Push the word to the vector
                word = "";  // Reset the word
            }
        } else {
            word += c;  // Add character to the current word
        }
    }

    // Add the last word to the vector if it's not empty
    if (!word.empty()) {
        userinput.push_back(word);
    }

    return userinput;
}

void commandChecker(string s){
  vector<string> builtInCommand = {"exit","echo","type","pwd"};
  int flag=0;
  for(int i=0; i<builtInCommand.size(); i++)
  {
    if(s==builtInCommand[i])
    {
      flag=1;
      break;
    }
  }
  if(flag==1){
    cout<<s<<" is a shell builtin"<<endl;
  }
  else{
    string path = get_path(s);
    if(path.empty()){
      cout<<s<<" not found\n";
    }
    else{
      cout<<s<<" is "<<path<<endl;
    }
  }
}

int main() {
  // Flush after every std::cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;

  // Uncomment this block to pass the first stage
  while(true){
    cout << "$ ";
    string input;
    getline(cin, input);
    
    vector<string> userinput = split_sentence(input);
    if(userinput.empty()) continue;
    if(userinput[0]=="exit"){
        return 0;
    }
    else if(userinput[0]=="echo"){
        string word="";
        string lastout="";
        bool sig=0;
        for(int i=1; i<userinput.size(); i++)
        {
          if(userinput[i]==""){
            continue;
          }
          else{
          cout<<userinput[i]<<" ";
         }
        }
        cout<<endl;
    }
    else if(userinput[0]=="type"){
      if(userinput.size() < 2) {
          cout << "type: missing argument" << endl;
          continue;
      }
      commandChecker(userinput[1]);
    }
    else if(userinput[0]=="pwd"){
      currentPathFinder();
    }
    else if(userinput[0]=="cd"){
      string target_dir;
      if (userinput.size() < 2) {
          target_dir = getenv("HOME");
      } else {
          target_dir = userinput[1];
          if (target_dir == "~") {
              target_dir = getenv("HOME");
          }
      }
      error_code ec;
      filesystem::current_path(target_dir, ec);
      if (ec) {
          cout << "cd: " << target_dir << ": " << ec.message() << endl;
      }
    }
else{
  string path_string = getenv("PATH");
  vector<string> path = split_string(path_string, ':');
  string filepath;
  bool found = false;
  for(int i = 0; i < path.size(); i++){
    filepath = path[i] + '/' + userinput[0];
    if(filesystem::exists(filepath) && filesystem::is_regular_file(filepath)){
      // Build the command with proper quoting
      string command;
      for (const auto& arg : userinput) {
          string escaped_arg;
          escaped_arg += "'"; // Start single quote
          for (char c : arg) {
              if (c == '\'') {
                  // Replace ' with '\''
                  escaped_arg += "'\\''";
              } else {
                  escaped_arg += c;
              }
          }
          escaped_arg += "'"; // End single quote
          command += escaped_arg + " ";
      }
      if (!command.empty()) {
          command.pop_back(); // Remove trailing space
      }
      int result = system(command.c_str());
      if (result != 0) {
          cerr << "Command execution failed with code " << result << endl;
      }
      found = true;
      break;
    } 
  }
  if (!found) {
      cout << userinput[0] << ": not found" << endl;
  }
}
  }
}