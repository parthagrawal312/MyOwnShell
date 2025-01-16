#include <iostream>
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
    if(input== "exit 0")
    {
      return 0;
    }
    cout<<input<<": command not found"<<endl;
  }
  
}
