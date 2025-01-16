#include <iostream>
#include <vector>
using namespace std;

int main() {
  // Flush after every std::cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;

  // Uncomment this block to pass the first stage
  while(true){
    cout << "$ ";
    string input;
    getline(cin, input);


    vector<string> split_sentence(string sen) {
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

    vector<string> userinput = split_sentence(input);
    switch(userinput[0]){
      case "exit":
        return 0;
      case "echo":
        for(int i=1; i<userinput.size(); i++)
        {
          cout<<userinput[i]<<" ";
        }
        break;
      default:
        cout<<input<<": command not found"<<endl;
        break;
    }
    
  }
  
}
