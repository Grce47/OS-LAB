/*
    Assignment2_8_20CS30034_20CS30051_20CS10068_20CS30022.cpp

    Custom bash program to execute commands
        OS Group 8
            Grace Sharma
            Umang Singla
            Saurabh Das
            Mradul Agrawal

    Compilation command:
        g++ Assignment2_8_20CS30034_20CS30051_20CS10068_20CS30022.cpp -std=c++17 -lstdc++fs -lreadline -o a.out
    Execution command:
        ./a.out
*/

#include <iostream>
#include <vector>
#include <filesystem>
#include <algorithm>

#include <stdlib.h>
#include <cstring>

#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#include <readline/readline.h>

using namespace std;
namespace fs = std::filesystem;

void remove_excess_spaces(string &s);
struct Command
{
    vector<string> args;
    string output_redirect, input_redirect;
};
vector<Command> parseInput(const string &user_input);

int status, ctrl_z_status = 0;

void parent_handler(int signum)
{
    cout << endl
         << fs::current_path().string() << "$ ";
    fflush(stdout);
    return;
}

void ctrl_z_handler(int signum)
{
    ctrl_z_status = 1;
}

void pwd()
{
    fs::path cwd = fs::current_path();
    cout << cwd.string() << endl;
    fflush(stdout);
}

void cd(string dir)
{
    int status = chdir(dir.c_str());
    if (status == -1)
    {
        cerr << "Unable to cd in " << dir << endl;
    }
}

void bind_up_arrow_key()
{
}

int main()
{
    rl_initialize();

    // Loop means a single process
    while (1)
    {
        signal(SIGINT, parent_handler);  // Ignore the SIGINT signal
        signal(SIGTSTP, parent_handler); // Ignore the SIGTSTP signal
        string promptString = fs::current_path().string() + "$ ";

        char *userInput;
        userInput = readline(promptString.c_str());
        int inputSize = strlen(userInput);

        string user_input;
        user_input.assign(userInput);

        // necessary?
        fflush(stdin);

        vector<Command> cmds = parseInput(user_input);

        if (cmds.size() == 1)
        {
            char *c_string_args[cmds.front().args.size() + 1];
            for (int i = 0; i < cmds.front().args.size(); i++)
            {
                c_string_args[i] = (char *)malloc(cmds.front().args[i].size() + 1);
                for (int j = 0; j < cmds.front().args[i].size() + 1; j++)
                {
                    c_string_args[i][j] = 0;
                }
                strcpy(c_string_args[i], cmds.front().args[i].c_str());
            }
            c_string_args[cmds.front().args.size()] = NULL;

            int inputFileDesc = open(cmds.front().input_redirect.c_str(), O_RDONLY, 0777);
            int outputFileDesc = open(cmds.front().output_redirect.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0777);

            // Command executions
            // Seperate handling for pwd and cd

            if (cmds.front().args[0] == "pwd")
            {
                pwd();
            }
            else if (cmds.front().args[0] == "cd")
            {
                cd(cmds.front().args[1]);
            }
            // All other commands handled by execvp
            else
            {
                // Spawn a child process to run the input command
                int pid = fork();
                if (pid == 0)
                {
                    if (cmds.front().input_redirect != "")
                    {
                        dup2(inputFileDesc, 0);
                        close(inputFileDesc);
                    }
                    if (cmds.front().output_redirect != "")
                    {
                        dup2(outputFileDesc, 1);
                        close(outputFileDesc);
                    }

                    if (execvp(cmds.front().args[0].c_str(), c_string_args) < 0)
                    {
                        cerr << "Cannot find the command: ";
                        for (auto &ele : cmds.front().args)
                            cerr << ele << " ";
                        cerr << endl;
                    }
                    exit(0);
                }
                else if (pid > 0)
                {
                    ctrl_z_status = 0;
                    signal(SIGINT, SIG_IGN);         // Ignore the SIGINT signal
                    signal(SIGTSTP, ctrl_z_handler); // Ignore the SIGTSTP signal

                    status = waitpid(-1, NULL, WNOHANG);
                    while (status == 0 && ctrl_z_status == 0)
                    {
                        status = waitpid(-1, NULL, WNOHANG);
                    }
                }
                else
                {
                    cerr << "Fork unsuccessull\n";
                }
            }
            for (int i = 0; i < cmds.front().args.size(); i++)
            {
                free(c_string_args[i]);
            }
        }
        else
        {
            // pipe exists
        }
    }
    return 0;
}

vector<Command> parseInput(const string &user_input)
{
    vector<Command> vec;

    string pipe_commands;
    stringstream pipe_ss(user_input);

    while (getline(pipe_ss, pipe_commands, '|'))
    {
        remove_excess_spaces(pipe_commands);
        if (pipe_commands.empty())
            continue;

        Command current_command;
        current_command.input_redirect = current_command.output_redirect = "";

        string token;
        istringstream space_sep_ss(pipe_commands);

        bool add_to_output = false, add_to_input = false;
        while (space_sep_ss >> token)
        {
            remove_excess_spaces(token);
            if (token.empty())
                continue;

            if (add_to_input)
            {
                current_command.input_redirect = token;
                add_to_input = false;
            }
            else if (add_to_output)
            {
                current_command.output_redirect = token;
                add_to_output = false;
            }
            else if (token == ">")
            {
                add_to_output = true;
            }
            else if (token == "<")
            {
                add_to_input = true;
            }
            else
            {
                current_command.args.push_back(token);
            }
        }
        vec.push_back(current_command);
    }

    return vec;
}
void remove_excess_spaces(string &s)
{
    // Remove leading spaces
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](int c)
                               { return !isspace(c); }));

    // Remove trailing spaces
    s.erase(find_if(s.rbegin(), s.rend(), [](int c)
                    { return !isspace(c); })
                .base(),
            s.end());

    // Remove consecutive spaces
    string::iterator new_end = unique(s.begin(), s.end(), [](char lhs, char rhs)
                                      { return isspace(lhs) && isspace(rhs); });
    s.erase(new_end, s.end());
}