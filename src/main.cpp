#include <iostream>
#include <vector>
#include <sstream>
#include <filesystem>
#include <string>
using namespace std;


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
      for (char c : input) {  // Iterate through each character in the user input sentence
        if (c == ' ') {  
            userinput.emplace_back(word);// If a space is found, add the word to the vector
            word = "";// Reset the word
        }
        else {
            word += c;// Append the character to the current word
        }
      }
      if (!word.empty()) { // Add the last word to the vector
        userinput.emplace_back(word);
      } 
      return userinput;// Return the vector containing words
}

void commandChecker(string s){
  vector<string> builtInCommand = {"exit","echo","type"};
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
    string path = get_path(input);
    if(path.empty()){
      cout<<input<<" not found\n";
    }
    else{
      cout<<input<<" is "<<path<<endl;
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
        for(int i=1; i<userinput.size(); i++)
        {
          cout<<userinput[i]<<" ";
        }
        cout<<endl;
    }
    else if(userinput[0]=="type"){
      commandChecker(userinput[1]);
    }
    else{
        cout<<input<<": command not found"<<endl;
    }
    
  }
  
}
