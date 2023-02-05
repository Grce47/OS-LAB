#include <iostream>
#include <vector> 
#include <filesystem>

#include <stdlib.h>
#include <cstring>

#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std; 
namespace fs = std::filesystem;

void pwd()
{
    fs::path cwd = fs::current_path(); 
    cout << cwd.string() << endl;
    fflush(stdout); 
}

void cd(char * dir)
{
    int status = chdir(dir);
    if(status == -1)
    {
        cerr << "Unable to cd in " << dir << endl; 
    }
}

int main()
{
    // Loop means a single process
    while(1)
    {
        cout << fs::current_path().string() << "$ "; 
        fflush(stdout);

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

        bool runBackground = false; 
        bool run_pwd = false; 
        bool run_cd = false; 

        for(int i = 0; i < tokens.size(); i++)
        {
            if(tokens[0] == "pwd")
            {
                run_pwd = true; 
            }
            if(tokens[0] == "cd")
            {
                run_cd = true; 
            }
            if(tokens[i] == "&")
            {
                runBackground = true; 
            }
            else if(tokens[i] == ">")
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

        if(run_pwd)
        {
            pwd(); 
        }
        else if(run_cd)
        {
            cd(args[1]); 
        }
        else
        {
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
                else
                {
                    if(execvp(args[0], args) < 0)
                    {   
                        cerr << "Cannot find the command: ";
                        for(int i = 0; i < arguments.size(); i++)
                        {
                            cerr << arguments[i] << " ";
                        }
                        cerr << endl; 
                    }
                }
                exit(0); 
            }
            else if(pid > 0)
            {
                wait(NULL);     
            }
            else
            {
                cerr << "Fork unsuccessull\n";
            }
        }
    }
    return 0;
}
