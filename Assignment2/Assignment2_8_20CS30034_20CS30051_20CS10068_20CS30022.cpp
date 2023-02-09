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
struct Command
{
    vector<string> args;
    string output_redirect, input_redirect;
    bool background = false; 
};
vector<Command> parseInput(const string &user_input);

// Readline key binding functions
static int bind_up_arrow_key(int count, int key);
static int bind_down_arrow_key(int count, int key);
static int bind_ctrl_a_key(int count, int key);
static int bind_ctrl_e_key(int count, int key); 

// cd function
void cd(string dir);

int status, ctrl_z_status = 0, child_pid;

void ctrl_c_handler(int signum)
{
    if(child_pid > 0){
        kill(child_pid, SIGKILL);
    }else{
        rl_replace_line("", 0);
        rl_reset_line_state();
        rl_redisplay();
        cout << endl
            << fs::current_path().string() << "$ ";
        fflush(stdout);
    }
    return;
}

void child_handler(int signum){
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG);
    if(pid > 0){
        child_processes.erase(pid);
    }
}

void ctrl_z_handler(int signum)
{
    ctrl_z_status = 1;
    child_pid = 0;
}

class CommandHistory
{
    string history_file = "history.txt";
    int max_history_size = 1000;
    int history_pointer = 0;
    deque<string> history_queue;
    ofstream ofile;

public:
    CommandHistory()
    {
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
    string current_command()
    {
        if (history_queue.size() == 0)
        {
            return "";
        }
        return history_queue[history_pointer];
    }
    void add_command(string command)
    {
        if (history_queue.size() == max_history_size)
        {
            history_queue.pop_front();
        }
        history_queue.push_back(command);
        history_pointer = history_queue.size();
        ofile << command << endl;
    }
    void reset_pointer()
    {
        history_pointer = history_queue.size();
    }
    void decrease_pointer()
    {
        if (history_pointer > 0)
        {
            history_pointer--;
        }
    }
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

void execute_process(vector<Command> &cmds);

int main()
{
    rl_initialize();
    rl_bind_keyseq("\\e[A", bind_up_arrow_key);
    rl_bind_keyseq("\\e[B", bind_down_arrow_key);
    rl_bind_keyseq("\\C-a", bind_ctrl_a_key);
    rl_bind_keyseq("\\C-e", bind_ctrl_e_key);
    signal(SIGCHLD, child_handler);

    char hostname[1024];
    char* username = getenv("USER");
    gethostname(hostname, 1024);
    string str(hostname), usernameString(username); 
    string name = usernameString + "@" + str;

    // Loop means a single process
    while (1)
    {
        child_pid = 0;
        signal (SIGTSTP, SIG_IGN); // Ignore the SIGTSTP signal
        signal (SIGINT, ctrl_c_handler);  // Ignore the SIGINT signal
        string promptString =  "\033[1;35m" + name + ":\033[0m" + "\033[1;31m" + fs::current_path().string() + "$ " + "\033[0m";

        char *userInput;
        userInput = readline(promptString.c_str());
        int inputSize = strlen(userInput);

        string user_input;
        user_input.assign(userInput);

        if (user_input != "")
        {
            command_history.add_command(user_input);
        }

        vector<Command> cmds = parseInput(user_input);
        if (cmds.empty())
            continue;

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
            else if (token == "&")
            {
                current_command.background = true; 
            }
            else
            {
                current_command.args.push_back(token);
            }
        }
        vec.push_back(current_command);
    }
    for (auto &cmd : vec)
    {
        vector<string> args;
        for (auto arg : cmd.args)
        {
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

vector<string> find_files(char *pattern)
{
    string process_path = fs::current_path().string();
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
        if (tok == NULL)
        {
            res.push_back(cur_path);
            q.pop();
        }
        else
        {
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
    return res;
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

static int bind_up_arrow_key(int count, int key)
{
    command_history.decrease_pointer();
    rl_replace_line(command_history.current_command().c_str(), 0);
    return 0;
}

static int bind_down_arrow_key(int count, int key)
{
    if (command_history.increase_pointer() == 0)
    {
        command_history.reset_pointer();
        rl_replace_line("", 0);
    }
    else
    {
        rl_replace_line(command_history.current_command().c_str(), 0);
    }
    return 0;
}

static int bind_ctrl_a_key(int count, int key)
{
    rl_point = 0;
    return 0;
}

static int bind_ctrl_e_key(int count, int key)
{
    rl_point = rl_end;
    return 0;
}

void cd(string dir)
{
    int chdir_status = chdir(dir.c_str());
    if (chdir_status == -1)
    {
        cerr << "Unable to cd in " << dir << endl;
    }
}

void execute_process(vector<Command> &cmds)
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
                cerr << "PIPE NOT FORMED : ";
                for (auto &ele : cmds[loop].args)
                    cerr << ele << " ";
                cerr << endl;
            }
            outfd = pipefd[1];
        }

        int childpid = fork();
        if (childpid == 0)
        {
            signal(SIGINT, SIG_IGN);
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

            // Converting vector<string> to char *[] (c_string_args[
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

            if (execvp(c_string_args[0], c_string_args) < 0)
            {
                cerr << "Cannot Find Command : ";
                for (auto &ele : cmds[loop].args)
                    cerr << ele << " ";
                cerr << endl;
            }

            for (int i = 0; i < cmds[loop].args.size(); i++)
            {
                free(c_string_args[i]);
            }
        }

        else if (childpid > 0)
        {
            close(pipefd[1]);
            ctrl_z_status = 0;
            child_pid = childpid;
            signal(SIGINT, ctrl_c_handler);         // Ignore the SIGINT signal
            signal(SIGTSTP, ctrl_z_handler); // Ignore the SIGTSTP signal

            child_processes.insert(childpid);

            status = waitpid(childpid, NULL, WNOHANG);
            while (status == 0 && ctrl_z_status == 0 && cmds[loop].background == 0)
            {
                status = waitpid(childpid, NULL, WNOHANG);
            }
        }
    }
}
