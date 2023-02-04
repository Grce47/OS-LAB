#include <iostream>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
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

        bool readOutputFile = false, readInputFile = false; 
        string inputFile = "", outputFile = ""; 

        vector<string> arguments; 
        bool readArg = true; 

        for(int i = 0; i < tokens.size(); i++)
        {
            if(tokens[i] == ">")
            {
                readOutputFile = true; 
                readArg = false; 
            }
            else if(tokens[i] == "<")
            {   
                readInputFile = true; 
                readArg = false; 
            }
            else
            {
                if(readInputFile)
                {
                    readInputFile = false; 
                    inputFile = tokens[i];
                }
                else if(readOutputFile)
                {
                    readOutputFile = false; 
                    outputFile = tokens[i];
                }
                else
                {
                    if(readArg)
                    {
                        arguments.push_back(tokens[i]);
                    }
                }
            }
        }

        char ** args = (char **) malloc(sizeof(char *) * arguments.size()); 
        for(int i = 0; i < arguments.size(); i++)
        {
            args[i] = (char *) malloc(arguments[i].size()+1);
            for(int j = 0; j < arguments[i].size()+1; j++)
            {
                args[i][j] = 0; 
            }
            strcpy(args[i], arguments[i].c_str());     
        }

        // Get the input and output file pointers
        int inputFileDesc = open(inputFile.c_str(), O_RDONLY, 0777);
        int outputFileDesc = open(outputFile.c_str(), O_CREAT|O_WRONLY|O_TRUNC, 0777);

        // Spawn a child process to run the input command
        int pid = fork(); 
        if(pid == 0)
        {
            if(inputFile != "")
            {
                dup2(inputFileDesc, 0);
                close(inputFileDesc);
            }
            if(outputFile != "")
            {
                dup2(outputFileDesc, 1);
                close(outputFileDesc);
            }
            if(execvp(args[0], args) < 0)
            {   
                cerr << "Cannot find the command: ";
                for(int i = 0; i < arguments.size(); i++)
                {
                    cerr << arguments[i] << " ";
                }
                cerr << endl; 
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
