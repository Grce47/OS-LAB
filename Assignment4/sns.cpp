// TODO: Check all critical section codes
// TODO: Use chrono
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <random>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <queue>
#include <map>
#include <set>
#include <chrono>

using namespace std;

#define PRIORITY 0
#define CHRONOLOGICAL 1

const int NUM_ACTION = 3;   // 3 actions
const int SLEEP_TIME = 120; // 120 seconds

const int NUM_READ_POST_THREAD = 10;   // 10 threads
const int NUM_PUSH_UPDATE_THREAD = 25; // 25 threads

const int num_edges = 289003;
const int num_nodes = 37700;
const string csv_file_path = "musae_git_edges.csv";
const char *sns_file_path = "sns.log";

// ACTION STRUCT
struct action
{
    int user_id;
    int action_id;
    int action_type;
    time_t action_time;
    int priority;
};

// Comparator Class
class Compare
{
public:
    bool operator()(action A, action B)
    {
        return A.priority < B.priority;
    }
};

// NODE CLASS
class Node
{
public:
    int order;
    int count_actions[NUM_ACTION];

    // Dynamic computation;
    bool computed;

    // Wall Queue
    queue<action> wall;

    // Feed Queue
    priority_queue<action, vector<action>, Compare> feed;

    Node()
    {
        for (int i = 0; i < NUM_ACTION; i++)
        {
            count_actions[i] = 0;
        }
        int randorder = rand() % 2;
        order = randorder;
        computed = false;
    }
};

// GLOBAL FILE
FILE *snsfile = fopen(sns_file_path, "wb");

// GLOBAL VARIABLES
vector<Node> nodes(num_nodes, Node());

/**
 * Static variables related to graph
 * Remain unchanged after once created in the main thread
 * graph: adjacency list representation of the given graph data
 * degree: stores degree of each node
 * common_edges: number of common edges between 2 nodes
    - Dynamically filled when userSimulator thread push some nodes
    - Used by the pushUpdate thread when pushing actions of a node
    - Although the data strucutre is shared between these threads there will not be any race condition
    - As a node is accessed by the pushUpdate thread only when it is created by the userSimulator thread
 * neighbours: set of neighbours of a particular node (used for the computation of common_edges)
*/
vector<vector<int>> graph(num_nodes);
vector<int> degree(num_nodes, 0);
map<pair<int, int>, int> common_edges;
set<int> neighbours[num_nodes];

/**
 * Shared queue for sharing actions
 * Producer: userSimulator thread
 * Consumer: pushUpdate thread
 **/
queue<action> live_action;
pthread_mutex_t live_action_mutex;
pthread_cond_t live_action_cond;
int live_action_size;

pthread_mutex_t feed_queue_mutex[num_nodes];
int feed_queue_size[num_nodes];

queue<int> live_neighbour;
pthread_mutex_t live_neighbour_mutex;
pthread_cond_t live_neighbour_cond;
int live_neighbour_size;

// THREAD FUNCTIONS
void *thread_userSimulator(void *arg)
{
    char buffer[100];
    int len;
    // 100 nodes * n actions then 2 minute sleep
    while (1)
    {
        sprintf(buffer, "::USER_SIMULATOR_WAKES::\n");
        len = sizeof(char) * strlen(buffer);
        fwrite(buffer, sizeof(char), len, snsfile);
        fwrite(buffer, sizeof(char), len, stdout);
        // Select 100 random nodes
        for (int i = 0; i < 100; i++)
        {
            int randnode = rand() % num_nodes;

            // Do the precomputation for this node
            if (nodes[randnode].computed == false)
            {
                for (auto neigh : graph[randnode])
                {
                    set<int> common;
                    for (auto ch : neighbours[randnode])
                    {
                        if (neighbours[neigh].count(ch))
                        {
                            common.insert(ch);
                        }
                    }
                    common_edges[make_pair(randnode, neigh)] = (int)common.size();
                    common_edges[make_pair(neigh, randnode)] = (int)common.size();
                }
                nodes[randnode].computed = true;
            }

            int node_degree = degree[randnode];
            int proportion_constant = 1;
            int proportion_value = (int)log2(node_degree);
            int num_actions = proportion_constant * proportion_value;
            sprintf(buffer, "::USER_SIMULATOR Random Node %-5d Node Degree %-5d Actions Generated %-5d::\n", randnode, node_degree, num_actions);
            len = sizeof(char) * strlen(buffer);
            fwrite(buffer, sizeof(char), len, snsfile);
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
                // act.action_time = time(0);
                auto now = chrono::system_clock::now(); 
                act.action_time = chrono::system_clock::to_time_t(now); 
                if (nodes[randnode].order == CHRONOLOGICAL)
                {
                    act.priority = act.action_time;
                }
                // Push the action to wall queue of that node
                nodes[randnode].wall.push(act);

                // ******* CRITICAL SECTION BEGINS *******
                pthread_mutex_lock(&live_action_mutex);
                // Push the action to live_action queue
                live_action.push(act);
                live_action_size++;
                pthread_cond_signal(&live_action_cond);
                pthread_mutex_unlock(&live_action_mutex);
                // ******* CRITICAL SECTION ENDS *******
            }
        }
        sprintf(buffer, "::USER_SIMULATOR_SLEEPS::\n");
        len = sizeof(char) * strlen(buffer);
        fwrite(buffer, sizeof(char), len, snsfile);
        fwrite(buffer, sizeof(char), len, stdout);
        // Sleep for 2 minutes
        sleep(SLEEP_TIME);
    }
    pthread_exit(0);
}

void *thread_pushUpdate(void *arg)
{
    char buffer[100];
    int len;
    while (1)
    {
        // ******* CRITICAL SECTION BEGINS *******
        pthread_mutex_lock(&live_action_mutex);
        while (live_action_size == 0)
        {
            pthread_cond_wait(&live_action_cond, &live_action_mutex);
        }
        live_action_size--;
        struct action act = live_action.front();
        live_action.pop();
        sprintf(buffer, "::PUSH_UPDATE Dequeue Node %-5d::\n", act.user_id);
        len = sizeof(char) * strlen(buffer);
        fwrite(buffer, sizeof(char), len, snsfile);
        fwrite(buffer, sizeof(char), len, stdout);
        pthread_mutex_unlock(&live_action_mutex);
        // ******* CRITICAL SECTION ENDS *******

        // Add this action to all the neighbours of the node
        int act_node = act.user_id;
        for (auto neigh : graph[act_node])
        {
            if (nodes[neigh].order == PRIORITY)
            {
                act.priority = common_edges[make_pair(act.user_id, neigh)];
            }
            // ******* CRITICAL SECTION BEGINS *******
            pthread_mutex_lock(&feed_queue_mutex[neigh]);
            nodes[neigh].feed.push(act);
            sprintf(buffer, "::PUSH_UPDATE Dequeue Node %-5d Neighbour %-5d::\n", act.user_id, neigh);
            len = sizeof(char) * strlen(buffer);
            fwrite(buffer, sizeof(char), len, snsfile);
            fwrite(buffer, sizeof(char), len, stdout);
            pthread_mutex_unlock(&feed_queue_mutex[neigh]);
            // ******* CRITICAL SECTION ENDS *******

            // ******* CRITICAL SECTION BEGINS *******
            pthread_mutex_lock(&live_neighbour_mutex);
            // Push the action to live_neighbour queue
            live_neighbour.push(neigh);
            live_neighbour_size++;
            pthread_cond_signal(&live_neighbour_cond);
            pthread_mutex_unlock(&live_neighbour_mutex);
            // ******* CRITICAL SECTION ENDS *******
        }
    }

    pthread_exit(0);
}

void *thread_readPost(void *arg)
{
    char buffer[100];
    int len;
    while (1)
    {
        // ******* CRITICAL SECTION BEGINS *******
        pthread_mutex_lock(&live_neighbour_mutex);
        while (live_neighbour_size == 0)
        {
            pthread_cond_wait(&live_neighbour_cond, &live_neighbour_mutex);
        }
        live_neighbour_size--;
        int neighbour = live_neighbour.front();
        live_neighbour.pop();
        pthread_mutex_unlock(&live_neighbour_mutex);
        // ******* CRITICAL SECTION ENDS *******

        // ******* CRITICAL SECTION BEGINS *******
        pthread_mutex_lock(&feed_queue_mutex[neighbour]);
        sprintf(buffer, "::READ_POST Reading feed queue of %-5d::\n", neighbour);
        len = sizeof(char) * strlen(buffer);
        fwrite(buffer, sizeof(char), len, snsfile);
        fwrite(buffer, sizeof(char), len, stdout);
        while (!nodes[neighbour].feed.empty())
        {
            action curr_act = nodes[neighbour].feed.top();
            nodes[neighbour].feed.pop();
            sprintf(buffer, "::READ_POST I read action number %-5d of type %-5d posted by user %-5d at time %-10d::\n", curr_act.action_id, curr_act.action_type, curr_act.user_id, (int)curr_act.action_time);
            len = sizeof(char) * strlen(buffer);
            fwrite(buffer, sizeof(char), len, snsfile);
            fwrite(buffer, sizeof(char), len, stdout);
        }
        pthread_mutex_unlock(&feed_queue_mutex[neighbour]);
        // ******* CRITICAL SECTION ENDS *******
    }

    pthread_exit(0);
}

int main(int argc, char **argv)
{
    // TODO: Check the correctness of this thing
    // Automatic flush
    setvbuf(snsfile, NULL, _IONBF, 0);

    // Mutex and Conditional Variable initialisations
    pthread_mutex_init(&live_action_mutex, NULL);
    pthread_cond_init(&live_action_cond, NULL);
    live_action_size = 0;
    for (int i = 0; i < num_nodes; i++)
    {
        pthread_mutex_init(&feed_queue_mutex[i], NULL);
        feed_queue_size[i] = 0;
    }
    pthread_mutex_init(&live_neighbour_mutex, NULL);
    pthread_cond_init(&live_neighbour_cond, NULL);
    live_neighbour_size = 0;

    cout << "::MAIN_THREAD_STARTED::" << endl;

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
    for (int i = 0; i < num_nodes; i++)
    {
        for (auto it : graph[i])
        {
            neighbours[i].insert(it);
        }
    }

    // Spawn the threads and wait for them
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

    // Mutex and Conditional Variables Destructions
    pthread_mutex_destroy(&live_action_mutex);
    pthread_cond_destroy(&live_action_cond);
    for (int i = 0; i < num_nodes; i++)
    {
        pthread_mutex_destroy(&feed_queue_mutex[i]);
    }
    pthread_mutex_destroy(&live_neighbour_mutex);
    pthread_cond_destroy(&live_neighbour_cond);

    return EXIT_SUCCESS;
}