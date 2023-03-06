#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <random>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <queue>

using namespace std;

const int NUM_ACTION = 3;   // 3 actions
const int SLEEP_TIME = 120; // 120 seconds

const int NUM_READ_POST_THREAD = 10;
const int NUM_PUSH_UPDATE_THREAD = 25;

const int num_edges = 289003;
const int num_nodes = 37700;
const string csv_file_path = "musae_git_edges.csv";
const string sns_file_path = "sns.log";

// GLOBAL FILE
ofstream snsfile(sns_file_path);

// ACTION STRUCT
struct action
{
    int user_id;
    int action_id;
    int action_type;
    time_t action_time;
};

// NODE CLASS
class Node
{
public:
    int order;
    int count_actions[NUM_ACTION];

    // Wall Queue
    queue<action> wall;
    // Feed Queue
    queue<action> feed;

    Node()
    {
        for (int i = 0; i < NUM_ACTION; i++)
        {
            count_actions[i] = 0;
        }
        int randorder = rand() % 2;
        order = randorder;
    }
};

// GLOBAL VARIABLES
vector<vector<int>> graph(num_nodes);
vector<int> degree(num_nodes, 0);
vector<Node> nodes(num_nodes, Node());

// Shared queue
queue<Node> shared_queue; 

// THREAD FUNCTIONS
void *thread_userSimulator(void *arg)
{
    // 100 nodes * n actions then 2 minute sleep
    while (1)
    {
        cout << "User simulator wakes..." << endl;
        snsfile << "USER SIMULATER WAKES" << endl;
        // Select 100 random nodes
        for (int i = 0; i < 100; i++)
        {
            int randnode = rand() % num_nodes;
            int node_degree = degree[randnode];
            int proportion_constant = 1;
            int proportion_value = (int)log2(node_degree);
            int num_actions = proportion_constant * proportion_value;
            snsfile << "::Random Node: " << randnode << "::" << endl;
            snsfile << "::Node Degree: " << node_degree << "::" << endl;
            snsfile << "::Actions Generated: " << num_actions << "::" << endl;
            // Each of the num_actions
            for (int j = 0; j < num_actions; j++)
            {
                // Create an action struct
                int randaction = rand() % NUM_ACTION;
                struct action act;
                act.user_id = randnode;
                act.action_type = randaction;
                // Get the action_id (it is different for each node and each event)
                act.action_id = nodes[randnode].count_actions[randaction];
                nodes[randnode].count_actions[randaction]++;
                act.action_time = time(NULL);
                // Push the action to wall queue of that node
                nodes[randnode].wall.push(act);
            }
        }
        cout << "User simulator sleeps..." << endl;
        snsfile << "USER SIMULATER SLEEPS" << endl;
        // Sleep for 2 minutes
        sleep(SLEEP_TIME);
    }
    pthread_exit(0);
}

void *thread_readPost(void *arg)
{
    while(1)
    {
        if(!shared_queue.empty())
        {
            
        }
    }
    pthread_exit(0);
}

void *thread_pushUpdate(void *arg)
{

    pthread_exit(0);
}

int main(int argc, char **argv)
{
    cout << "Main thread started..." << endl;

    // Read the CSV file and load the graph
    vector<pair<int, int>> edges;
    ifstream file(csv_file_path);
    string temp;
    getline(file, temp);
    while (getline(file, temp))
    {
        string a, b;
        bool flag = 0;
        for (auto it : temp)
        {
            if (it == ',')
                flag = 1;
            else
            {
                if (flag)
                    b += it;
                else
                    a += it;
            }
        }
        edges.push_back(make_pair(stoi(a), stoi(b)));
    }
    for (auto &it : edges)
    {
        graph[it.first].push_back(it.second);
        graph[it.second].push_back(it.first);
    }

    // Do some graph precomputations
    for (auto &it : edges)
    {
        degree[it.first]++;
        degree[it.second]++;
    }

    // Spawn the threads and wait for them
    cout << "Spawning various threads..." << endl;
    pthread_t userSimulator, readPost[NUM_READ_POST_THREAD], pushUpdate[NUM_PUSH_UPDATE_THREAD];

    pthread_create(&userSimulator, NULL, *thread_userSimulator, NULL);
    for (int i = 0; i < NUM_READ_POST_THREAD; i++)
        pthread_create(&readPost[i], NULL, *thread_readPost, NULL);
    for (int i = 0; i < NUM_PUSH_UPDATE_THREAD; i++)
        pthread_create(&pushUpdate[i], NULL, *thread_pushUpdate, NULL);

    pthread_join(userSimulator, NULL);
    for (int i = 0; i < NUM_READ_POST_THREAD; i++)
        pthread_join(readPost[i], NULL);
    for (int i = 0; i < NUM_PUSH_UPDATE_THREAD; i++)
        pthread_join(pushUpdate[i], NULL);
    cout << "All threads joined.." << endl;

    return EXIT_SUCCESS;
}