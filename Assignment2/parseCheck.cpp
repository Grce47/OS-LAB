#include <iostream>
#include <vector>
#include <cstring>
#include <sstream>
#include <algorithm>
using namespace std;

void remove_excess_spaces(string &s);

struct Command
{
    vector<string> args;
    string output_redirect, input_redirect;
    void display()
    {
        for (int i = 0; i < args.size(); i++)
        {
            cout << "{" << i << ":" << args[i] << "} ";
        }
        cout << "\nInput : " << input_redirect << " Output : " << output_redirect << "\n";
    }
};

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

int main()
{
    string user_input;
    getline(cin, user_input);

    // add neccessary spaces to user_input maybe ...eg "./a.out >out.txt" --> "./a.out > out.txt"
    cout << user_input << "\n";
    vector<Command> res = parseInput(user_input);
    for (Command &ele : res)
        ele.display();
    return 0;
}

void remove_excess_spaces(string &s)
{
    // Remove leading spaces
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](int c) { return !isspace(c); }));

    // Remove trailing spaces
    s.erase(find_if(s.rbegin(), s.rend(), [](int c) { return !isspace(c); }).base(),s.end());

    // Remove consecutive spaces
    string::iterator new_end = unique(s.begin(), s.end(), [](char lhs, char rhs){ return isspace(lhs) && isspace(rhs); });
    s.erase(new_end, s.end());
}