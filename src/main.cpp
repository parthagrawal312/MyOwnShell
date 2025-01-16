#include <iostream>
#include <vector>
#include <sstream>
#include <filesystem>
#include <string>
#include <fstream>
using namespace std;

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
          string command = "exec " + path[i] + '/' + input;
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
