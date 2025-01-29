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
      string lastinput="";
      bool openquote=false;
      for (char c : input) {
        if(c=='\'' || c=='\"')
        {
          openquote=!openquote;
          continue;
        }
        // if(openquote==true){
        //   word = word+c;
        // }
          // Iterate through each character in the user input sentence
        if (openquote==false && c == ' ') {
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





// vector<string> split_sentence(string input) {
//       vector<string> userinput;
//       string word = "";
//       for (char c : input) {  // Iterate through each character in the user input sentence
//         if (c == ' ') {  
//             userinput.emplace_back(word);// If a space is found, add the word to the vector
//             word = "";// Reset the word
//         }

//         else {
//             word += c;// Append the character to the current word
//         }
//       }
//       if (!word.empty()) { // Add the last word to the vector
//         userinput.emplace_back(word);
//       } 
//       return userinput;// Return the vector containing words
// }

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
           
            cout<<userinput[i]<<"";
          // for(char c : userinput[i])
          // {
          //   if(c=='\'' || c=='\"')
          //   {
          //     sig=!sig;
          //     continue;
          //   }
          //   else{
          //     word = word + c;
          //   }
          // }
          // userinput[i]=word;
          // word="";
          // if(sig==1 && lastout!=" "){
          //   lastout=userinput[i];
          //   cout<<userinput[i]<<" ";
          // }
          // else if(lastout!=" "){
          //   lastout=userinput[i];
          //   cout<<userinput[i]<<" ";
          // }
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
