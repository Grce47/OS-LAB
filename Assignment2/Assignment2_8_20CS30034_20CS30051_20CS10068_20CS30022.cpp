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

void execute_process(const vector<Command> &cmds)
{
    int n = cmds.size(), pipefd[2];

    for (int loop = 0; loop < n; loop++)
    {
        int infd = STDIN_FILENO, outfd = STDOUT_FILENO;
        if (cmds[loop].input_redirect != "")
        {
            infd = open(cmds[loop].input_redirect.c_str(), O_RDONLY, 0777);
        }
        if (cmds[loop].output_redirect != "")
        {
            outfd = open(cmds[loop].output_redirect.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0777);
        }

        if (loop > 0)
        {
            infd = pipefd[0];
        }
        if (loop + 1 < n)
        {
            if (pipe(pipefd) == -1)
            {
                perror("Pipe error: ");
            }
            outfd = pipefd[1];
        }

        int childpid = fork();
        if (childpid == 0)
        {
            if (infd != STDIN_FILENO)
            {
                dup2(infd, STDIN_FILENO);
                close(infd);
            }
            if (outfd != STDOUT_FILENO)
            {
                dup2(outfd, STDOUT_FILENO);
                close(outfd);
            }
            if (cmds[loop].args[0] == "pwd")
            {
                pwd();
                exit(EXIT_SUCCESS);
            }
            else
            {
                char *c_string_args[cmds[loop].args.size() + 1];
                for (int i = 0; i < cmds[loop].args.size(); i++)
                {
                    c_string_args[i] = (char *)malloc(cmds[loop].args[i].size() + 1);
                    for (int j = 0; j < cmds[loop].args[i].size() + 1; j++)
                    {
                        c_string_args[i][j] = 0;
                    }
                    strcpy(c_string_args[i], cmds[loop].args[i].c_str());
                }
                c_string_args[cmds[loop].args.size()] = NULL;
                execvp(c_string_args[0], c_string_args);
                for (int i = 0; i < cmds[loop].args.size(); i++)
                {
                    free(c_string_args[i]);
                }
            }
            perror("Exec error: ");
            exit(EXIT_FAILURE);
        }
        else if (childpid > 0)
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
    }
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
        if (cmds.front().args[0] == "cd")
        {
            cd(cmds.front().args.size() > 1 ? cmds.front().args[1] : ".");
        }
        else
            execute_process(cmds);

        free(userInput);
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