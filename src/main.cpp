#include <iostream>
#include <vector>
using namespace std;

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
        cout<endl;
    }
    else{
        cout<<input<<": command not found"<<endl;
    }
    
  }
  
}
