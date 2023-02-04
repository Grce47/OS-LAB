#include <iostream>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h> 
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
        char * args[100];
        for(int i = 0; i < 100; i++)
        {
            args[i] = NULL;
        }
        int numArgs = 0;
        int argPointer = 0; 

        bool readingArg = 0; 
        
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
                    args[numArgs-1][argPointer] = userInput[i];
                    argPointer++;
                }
                else
                {
                    readingArg = true; 
                    numArgs++;
                    argPointer = 0; 
                    args[numArgs-1] = (char *) malloc(sizeof(char) * 100);
                    memset(args[numArgs-1], 0, 100);
                    args[numArgs-1][argPointer] = userInput[i];
                    argPointer++;
                }
            }
        }

        // Spawn a child process to run the input command
        int pid = fork(); 
        if(pid == 0)
        {
            execvp(args[0], args);
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
