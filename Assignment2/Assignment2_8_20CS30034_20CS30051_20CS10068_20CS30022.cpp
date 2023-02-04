#include <iostream>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h> 
#include <vector> 
using namespace std; 

int main()
{
    // Loop means a single process
    while(1)
    {
        cout << "$ "; 
        char userInput[5000];
        memset(userInput, 0, sizeof(userInput));

        char charInput; 
        int inputSize = 0;
        // One by one take all the characters into the userInput array until a newline character is encountered
        while(1)
        {
            charInput = getchar(); 
            if(charInput == '\n')
            {
                break; 
            }
            else
            {
                userInput[inputSize] = charInput; 
                inputSize++; 
            }
        }  
        // userInput contain all the characters excluding the newline character

        // Parse the userInput
        vector<string> tokens; 
        bool readingArg = 0;
        int tokenPtr = -1; 
        
        for(int i = 0; i < inputSize; i++)
        {
            if(userInput[i] == ' ')
            {
                readingArg = false; 
            }
            else
            {
                if(readingArg)
                {
                    tokens[tokenPtr] += userInput[i];
                }
                else
                {
                    readingArg = true; 
                    tokenPtr++;
                    tokens.push_back("");
                    tokens[tokenPtr] += userInput[i];
                }
            }
        }

        char ** args = new char*[tokens.size() + 1];
        for(int i = 0; i < tokens.size(); i++)
        {
            args[i] = new char[tokens.size()+1];
            memset(args[i], 0, tokens.size()+1);
            strcpy(args[i], tokens[i].c_str());
        }

        // Spawn a child process to run the input command
        int pid = fork(); 
        if(pid == 0)
        {
            if(execvp(args[0], args) < 0)
            {   
                cerr << "Cannot find the command: " << args[0] << endl;
            }
            exit(0); 
        }
        else if(pid > 0)
        {
            wait(NULL);     
        }
        else
        {
            
        }
    }
    return 0;
}
