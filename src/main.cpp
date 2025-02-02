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
  string newCommand;
  string chFront, chBack;
  if(command.front() == '\'' || command.front() == '\"') {
    if(command.size() > 0 && (command.front() == '\'' || command.front() == '\"'))
      newCommand = command.substr(1);
    if(command.size() > 0 && (command.back() == '\'' || command.back() == '\"'))
      newCommand = newCommand.substr(0, newCommand.size()-1);
    chFront = command.front();
    chBack = command.back();
  }
  else {
    newCommand = command;
    chFront = "";
    chBack = "";
  }
  string path_env = getenv("PATH");
  stringstream ss(path_env);
  string path;
    while (!ss.eof())
    {
      getline(ss , path, ':');
      string fullPath = path + "/" + newCommand;
      if (filesystem::exists(fullPath)){
        return path + "/" + chFront + newCommand + chBack;
      }
    }
    return "";  
}











// vector<string> split_sentence(string input) {
//     vector<string> userinput;
//     string word = "";
//     bool openquote = false;
//     bool opendoublequote = false;
//     bool keepNextCharSafe = false;

//     for (char c : input) {
//         if (keepNextCharSafe==true) {
//             word += c;
//             keepNextCharSafe = false;
//             continue;
//         }

//         if (c == '\\' && openquote==false) {
//             keepNextCharSafe = true;
//             continue;
//         }
//         else if (c == '\\' && openquote==true) {
//             word += c;
//             continue;
//         }
//         else if (c == '\'' && opendoublequote==false) {
//             openquote = !openquote;
//             continue;
//         } 
//         else if (c == '"' && openquote==false) {
//             opendoublequote = !opendoublequote;
//             continue;
//         }

//         if (openquote==false && opendoublequote==false && c == ' ') {
//             if (!word.empty()) {
//                 userinput.push_back(word);
//                 word = "";
//             }
//         } else {
//             word += c;
//         }
//     }

//     if (!word.empty()) {
//         userinput.push_back(word);
//     }

//     return userinput;
// }



// vector<string> split_sentence(string input) {
//     vector<string> userinput;
//     string word = "";
//     bool openquote = false;
//     bool opendoublequote = false;
//     bool keepNextCharSafe = false;

//     for (size_t i = 0; i < input.length(); i++) {
//         char c = input[i];

//         // Handle escape sequences
//         if (keepNextCharSafe==true) {
//             word += c;  // Add the escaped character to the word
//             keepNextCharSafe = false;  // Reset the escape flag
//             continue;
//         }

//         // Handle backslashes inside double quotes (escape special characters)
//         if (c == '\\' && opendoublequote) {
//             if (i + 1 < input.length() && (input[i + 1] == '\\' || input[i + 1] == '$' || input[i + 1] == '"' || (input[i + 1] == '\\' && input[i + 2] == 'n') || input[i + 1] == '\'')) {
//                 keepNextCharSafe = true;  // Mark next character as escaped
//                 //word += c;  // Add backslash to word
//                 continue;
//             } else {
//                 word += c;  // Treat backslash as a regular character if not followed by an escape sequence
//                 continue;
//             }
//         }

//         // Handle backslashes inside single quotes
//         if (c == '\\' && openquote) {
//             word += c;  // Treat the backslash as part of the word in single quotes
//             continue;
//         }

//         // Handle single quotes (toggle openquote)
//         if (c == '\'' && !opendoublequote) {
//             openquote = !openquote;
//             continue;
//         }

//         // Handle double quotes (toggle opendoublequote)
//         if (c == '"' && !openquote) {
//             opendoublequote = !opendoublequote;
//             continue;
//         }

//         // Space handling (only split if not inside quotes)
//         if (!openquote && !opendoublequote && c == ' ') {
//             if (!word.empty()) {
//                 userinput.push_back(word);  // Push the word to the vector
//                 word = "";  // Reset the word
//             }
//         } else {
//             word += c;  // Add character to the current word
//         }
//     }

//     // Add the last word to the vector if it's not empty
//     if (!word.empty()) {
//         userinput.push_back(word);
//     }

//     return userinput;
// }



vector<string> split_sentence(string input) {
    vector<string> userinput;
    string word = "";
    bool openquote = false;  // for single quote
    bool opendoublequote = false;  // for double quote
    bool keepNextCharSafe = false;  // for escaped characters

    for (size_t i = 0; i < input.length(); i++) {
        char c = input[i];

        // Handle escape sequences (escape characters like \')
        if (keepNextCharSafe) {
            word += c;  // Add the escaped character to the word
            keepNextCharSafe = false;  // Reset the escape flag
            continue;
        }

        // Handle backslashes inside double quotes (escape special characters)
        if (c == '\\' && opendoublequote) {
            if (i + 1 < input.length() && (input[i + 1] == '\\' || input[i + 1] == '$' || input[i + 1] == '"' || input[i + 1] == '\'')) {
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
      commandChecker(userinput[1]);
    }
    else if(userinput[0]=="pwd"){
      currentPathFinder();
    }
    else if(userinput[0]=="cd"){
      if (userinput[1] == "~"){
        std::filesystem::current_path(getenv("HOME"));
      }
      else  if (std::filesystem::exists(userinput[1])){
          std::filesystem::current_path(userinput[1]);
      } 
      else cout << userinput[1] << ": No such file or directory" <<endl;
    }
    else{
      string path_string = getenv("PATH");
      //cout<<"path_string is: "<<path_string<<endl;
      vector<string> path = split_string(path_string, ':');
      string filepath;
     // cout<<"filepath is: "<<filepath<<endl;
      for(int i = 0; i < path.size(); i++){
        filepath = path[i] + '/' + userinput[0];
      //  cout<<"filepath is : "<<filepath<<endl;
        ifstream file(filepath);
        if(file.good()){
          string command =  input;  // "exec " + path[i] + '/' +
      //    cout<<"command is: "<<command<<endl;
          system(command.c_str());
          break;
        } 
        else if(i == path.size() - 1){
       //   cout<<"userinput[0] is: "<<userinput[0]<<endl;
           cout << userinput[0] << ": not found\n";
        }
      }
    }
  }
  
}
