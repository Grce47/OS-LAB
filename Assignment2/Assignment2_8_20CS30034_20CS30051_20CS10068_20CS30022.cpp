/*
    Assignment2_8_20CS30034_20CS30051_20CS10068_20CS30022.cpp

    Custom bash program to execute commands
        OS Group 8
            Grace Sharma
            Umang Singla
            Sourabh Das
            Mradul Agrawal
*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <queue>
#include <set>

#include <stdlib.h>
#include <cstring>

#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>

#include <readline/readline.h>
#include <filesystem>
#include <fnmatch.h>
#include <ncurses.h> 

using namespace std;
namespace fs = std::filesystem;

set<int> child_processes;

// Parsing related functions
void remove_excess_spaces(string &s);
vector<string> find_files(char *);

// straucture to store each command
struct Command
{
    vector<string> args;
    string output_redirect, input_redirect;
    bool background = false; 
};

// function to parse the user commnd
vector<Command> parseInput(const string &user_input);

// Readline key binding functions
static int bind_up_arrow_key(int count, int key);
static int bind_down_arrow_key(int count, int key);
static int bind_ctrl_a_key(int count, int key);
static int bind_ctrl_e_key(int count, int key); 

// cd function
void cd(string dir);

// flags to handle ctrl+c and ctrl+z
int status, ctrl_z_status = 0, child_pid;

// ctrl + c handler
void ctrl_c_handler(int signum)
{
    // kill the child process
    if(child_pid > 0){
        kill(child_pid, SIGKILL);
    }else{
        // clear the current line
        rl_replace_line("", 0);
        rl_reset_line_state();
        rl_redisplay();
        char hostname[1024];
        char* username = getenv("USER");
        gethostname(hostname, 1024);
        string str(hostname), usernameString(username); 
        string name = usernameString + "@" + str;
        cout << endl
            << "\033[1;35m" + name + ":\033[0m" + "\033[1;31m" + fs::current_path().string() + "$ " + "\033[0m";
        fflush(stdout);
    }
    return;
}

// handling the child process termination
void child_handler(int signum){
    int status;
    // wait for the child process to terminate
    pid_t pid = waitpid(-1, &status, WNOHANG);
    // remove the child process from the set
    if(pid > 0){
        child_processes.erase(pid);
    }
}

// ctrl + z handler
void ctrl_z_handler(int signum)
{
    ctrl_z_status = 1;
    child_pid = 0;
}

// class to store the history of commands
class CommandHistory
{
    // File to store the history
    string history_file = "history.txt";
    // Maximum number of commands to store
    int max_history_size = 1000;
    // Pointer to the current command
    int history_pointer = 0;
    // Queue to store the commands
    deque<string> history_queue;
    // File to store the history
    ofstream ofile;

public:
// Constructor to read the history from the file
    CommandHistory()
    {
        // Read the history from the file
        ifstream file;
        file.open(history_file);
        if (file.is_open())
        {
            string line;
            while (getline(file, line))
            {
                history_queue.push_back(line);
            }
            file.close();
        }
        reset_pointer();

        // For output in the file
        ofile.open(history_file, std::ios_base::app);
    }
    // Function to get the current command
    string current_command()
    {
        // If no command is present
        if (history_queue.size() == 0)
        {
            return "";
        }
        return history_queue[history_pointer];
    }
    // Function to add the command to the history
    void add_command(string command)
    {
        // If the history is full
        if (history_queue.size() == max_history_size)
        {
            history_queue.pop_front();
        }
        // Add the command to the history
        history_queue.push_back(command);
        // Reset the pointer
        history_pointer = history_queue.size();
        // Write the command to the file
        ofile << command << endl;
    }
    // Function to reset the pointer
    void reset_pointer()
    {
        history_pointer = history_queue.size();
    }
    // Function to decrease the pointer
    void decrease_pointer()
    {
        if (history_pointer > 0)
        {
            history_pointer--;
        }
    }
    // Function to increase the pointer
    int increase_pointer()
    {
        if (history_pointer < history_queue.size() - 1)
        {
            history_pointer++;
            return 1;
        }
        else
        {
            return 0;
        }
    }
};

CommandHistory command_history;

// function to execute the commands
void execute_process(vector<Command> &cmds);

int main()
{
    // Readline key bindings
    rl_initialize();
    rl_bind_keyseq("\\e[A", bind_up_arrow_key);
    rl_bind_keyseq("\\e[B", bind_down_arrow_key);
    rl_bind_keyseq("\\C-a", bind_ctrl_a_key);
    rl_bind_keyseq("\\C-e", bind_ctrl_e_key);

    // signal to handle child termination
    signal(SIGCHLD, child_handler);

    // Getting data to get the prompt
    char hostname[1024];
    char* username = getenv("USER");
    gethostname(hostname, 1024);
    string str(hostname), usernameString(username); 
    string name = usernameString + "@" + str;

    // Loop means a single process
    while (1)
    {
        // reset the status
        child_pid = 0;
        // Ignore the SIGTSTP
        signal (SIGTSTP, SIG_IGN);
        // Catch the SIGINT and send it to the ctrl_c_handler
        signal (SIGINT, ctrl_c_handler); 
        // Get the prompt
        string promptString =  "\033[1;35m" + name + ":\033[0m" + "\033[1;31m" + fs::current_path().string() + "$ " + "\033[0m";

        char *userInput;
        // tanking user command as input
        userInput = readline(promptString.c_str());
        int inputSize = strlen(userInput);

        string user_input;
        user_input.assign(userInput);

        // storing the command in the history
        if (user_input != "")
        {
            command_history.add_command(user_input);
        }

        // parsing the input
        vector<Command> cmds = parseInput(user_input);
        if (cmds.empty())
            continue;

        // handling the cd command
        if (cmds.front().args[0] == "cd")
        {
            // If there is no argument, go to home directory
            cd(cmds.front().args.size() > 1 ? cmds.front().args[1] : "~");
        }
        // If any other command
        else
            execute_process(cmds); // executing the commands

        free(userInput);    // freeing the memory
    }
    return 0;
}

// function to parse the input
vector<Command> parseInput(const string &user_input)
{
    vector<Command> vec; // vector to store the commands

    string pipe_commands;
    stringstream pipe_ss(user_input);

    // parsing the input
    while (getline(pipe_ss, pipe_commands, '|'))
    {
        remove_excess_spaces(pipe_commands); // removing the extra spaces in the input
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
            else if (token == "&")
            {
                current_command.background = true; 
            }
            else
            {
                current_command.args.push_back(token);
            }
        }
        vec.push_back(current_command); // adding the command to the vector
    }
    
    // handling the wildcards
    for (auto &cmd : vec)
    {
        vector<string> args;
        for (auto arg : cmd.args)
        {
            // replacing the regex with the actual file names
            if (arg.find("*") != string::npos || arg.find("?") != string::npos)
            {
                vector<string> files = find_files(const_cast<char *>(arg.c_str()));
                for (auto i : files)
                    args.push_back(i);
            }
            else
            {
                args.push_back(arg);
            }
        }
        cmd.args = args;
    }

    return vec;
}

// function to find files corresponding to the regex
vector<string> find_files(char *pattern)
{
    // getting the current path
    string process_path = fs::current_path().string();

    // applying level order traversal of tree to find the files
    queue<pair<string, int>> q;
    vector<string> res;
    char *tok = strtok(pattern, "/");
    string path(process_path);
    q.push({path, 0});
    int lvl = 1;
    while (!q.empty())
    {
        string cur_path = q.front().first;
        int path_lvl = q.front().second;
        while (tok != NULL && strcmp(tok, ".") == 0)
        {
            tok = strtok(NULL, "/");
        }

        // storing the files that match the regex in the result vector
        if (tok == NULL)
        {
            res.push_back(cur_path);
            q.pop();
        }
        else
        {
            // loop until the level of top element of queue doesn't change
            while (!q.empty() && q.front().second == path_lvl)
            {
                cur_path = q.front().first;
                if (strcmp(tok, "..") == 0)
                {
                    chdir(cur_path.c_str());
                    chdir("..");
                    q.push({fs::current_path().string(), lvl});
                    chdir(process_path.c_str());
                }
                else
                {
                    // iterating to all the files in the current directory
                    for (const auto &entry : fs::directory_iterator(cur_path))
                    {
                        fs::path outfilename = entry.path();
                        string outfilename_str = outfilename.string();
                        string temp(tok);
                        if (temp != ".")
                        {
                            temp = cur_path + "/" + temp;
                        }
                        else
                            temp = cur_path;

                        if (fnmatch(temp.c_str(), outfilename_str.c_str(), FNM_PATHNAME) == 0)
                        {
                            q.push({outfilename_str, lvl});
                        }
                    }
                }

                q.pop();
            }
            lvl++;
        }
        tok = strtok(NULL, "/");
    }
    chdir(process_path.c_str());

    // returning the result vector
    return res;
}

// Function to remove the extra spaces
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

// Function to bind up arrow key
static int bind_up_arrow_key(int count, int key)
{
    // Increase the pointer and replace the line with the command at the current pointer
    command_history.decrease_pointer();
    rl_replace_line(command_history.current_command().c_str(), 0);
    return 0;
}

// Function to bind down arrow key
static int bind_down_arrow_key(int count, int key)
{
    // If pointer is at the end of history, clear the line
    if (command_history.increase_pointer() == 0)
    {
        command_history.reset_pointer();
        rl_replace_line("", 0);
    }
    // Else, replace the line with the command at the current pointer
    else
    {
        rl_replace_line(command_history.current_command().c_str(), 0);
    }
    return 0;
}

// Function to bind ctrl+a key
static int bind_ctrl_a_key(int count, int key)
{
    // Move cursor to start of line
    rl_point = 0;
    return 0;
}

// Function to bind ctrl+e key
static int bind_ctrl_e_key(int count, int key)
{
    // Move cursor to end of line
    rl_point = rl_end;
    return 0;
}

// Function to change directory
void cd(string dir)
{
    // If no directory is specified, go to home directory
    if (dir == "~")
    {
        dir = getenv("HOME");
    }
    int chdir_status = chdir(dir.c_str());
    if (chdir_status == -1)
    {
        cerr << "Unable to cd in " << dir << endl;
    }
}

// Function to execute a command
void execute_process(vector<Command> &cmds)
{
    // Number of commands
    int n = cmds.size(), pipefd[2];

    // Execute individual commands
    for (int loop = 0; loop < n; loop++)
    {
        // infile and outfile set to default value (stdin and stdout)
        int infd = STDIN_FILENO, outfd = STDOUT_FILENO;

        if (cmds[loop].input_redirect != "")
        {
            infd = open(cmds[loop].input_redirect.c_str(), O_RDONLY, 0777);
        }
        if (cmds[loop].output_redirect != "")
        {
            outfd = open(cmds[loop].output_redirect.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0777);
        }

        // Creating pipe for the next command
        if (loop > 0)
        {
            infd = pipefd[0];
        }
        // Creating pipe for the next command
        if (loop + 1 < n)
        {
            // Creating pipe
            if (pipe(pipefd) == -1)
            {
                cerr << "PIPE NOT FORMED : ";
                for (auto &ele : cmds[loop].args)
                    cerr << ele << " ";
                cerr << endl;
            }
            outfd = pipefd[1];
        }

        // Forking a child process
        int childpid = fork();
        // Child process
        if (childpid == 0)
        {
            // Ignore SIGINT
            signal(SIGINT, SIG_IGN);
            // Changing input and output file descriptors
            if (infd != STDIN_FILENO)
            {
                dup2(infd, STDIN_FILENO);
                close(infd);
            }
            // Changing input and output file descriptors
            if (outfd != STDOUT_FILENO)
            {
                dup2(outfd, STDOUT_FILENO);
                close(outfd);
            }

            // Converting vector<string> to char *[] (c_string_args[
            char *c_string_args[cmds[loop].args.size() + 1];
            for (int i = 0; i < cmds[loop].args.size(); i++)
            {
                // +1 for null character
                c_string_args[i] = (char *)malloc(cmds[loop].args[i].size() + 1);
                for (int j = 0; j < cmds[loop].args[i].size() + 1; j++)
                {
                    c_string_args[i][j] = 0;
                }
                strcpy(c_string_args[i], cmds[loop].args[i].c_str());
            }
            // Last element of c_string_args should be NULL
            c_string_args[cmds[loop].args.size()] = NULL;

            // Executing the command
            if (execvp(c_string_args[0], c_string_args) < 0)
            {
                cerr << "Cannot Find Command : ";
                for (auto &ele : cmds[loop].args)
                    cerr << ele << " ";
                cerr << endl;
            }

            // Freeing the memory
            for (int i = 0; i < cmds[loop].args.size(); i++)
            {
                free(c_string_args[i]);
            }
        }
        // Parent process
        else if (childpid > 0)
        {
            // Closing the file descriptors
            close(pipefd[1]);
            // Setting the status of the child process
            ctrl_z_status = 0;
            child_pid = childpid;
            // Catch the SIGINT and SIGTSTP signals and handle them
            signal(SIGINT, ctrl_c_handler);        
            signal(SIGTSTP, ctrl_z_handler);

            // Inserting the child process in the set
            child_processes.insert(childpid);

            // Waiting for the child process to finish
            status = waitpid(childpid, NULL, WNOHANG);
            while (status == 0 && ctrl_z_status == 0 && cmds[loop].background == 0)
            {
                // Waiting for the child process to finish
                status = waitpid(childpid, NULL, WNOHANG);
            }
        }
    }
}
