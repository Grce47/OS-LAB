#include <bits/stdc++.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>
using namespace std;

class Shared_mem_info
{
public:
    int shmid, *capacity, *size;
    key_t key;
    Shared_mem_info(key_t _key, key_t _capacity_key, key_t _size_key)
    {
        shmid = -1;
        key = _key;
        size = 0;
        capacity = (int *)shmat(shmget(_capacity_key, sizeof(int), IPC_CREAT | 0666), NULL, 0);
        size = (int *)shmat(shmget(_size_key, sizeof(int), IPC_CREAT | 0666), NULL, 0);
    }
};

template <class T>
T *shared_memory_init(Shared_mem_info &shm, bool intialize_to_null = false)
{
    shm.shmid = shmget(shm.key, (*shm.capacity) * sizeof(T), IPC_CREAT | 0666);
    if (shm.shmid < 0)
    {
        cerr << "SHMID NOT MADE IN INIT\n";
        exit(EXIT_FAILURE);
    }
    T *ptr = (T *)shmat(shm.shmid, NULL, 0);
    if (intialize_to_null)
        memset(ptr, 0, (*shm.capacity) * sizeof(T));
    return ptr;
}

struct node
{
    int vertex, offset, map;
};

int rand_num(int l, int r)
{
    return l + (rand() % (r - l + 1));
}

int get_degree(int u, Shared_mem_info &shm_node_list, node *node_list, node *edge_list)
{
    int res = 0, cur_off = node_list[u].offset;
    while (cur_off != -1)
    {
        res++;
        cur_off = edge_list[cur_off].offset;
    }
    return res;
}

/*
wakes up every 50 seconds, and updates the graph in the following
way: A number m in the range [10, 30] is chosen randomly, and m new nodes are added
to the graph. For each newly added node, select a number k in the range [1, 20] at
random; the new node will connect to k existing nodes. The probability of a new node
connecting to an existing node of degree d is proportional to d (in other words, a popular
node is more likely to get connections from new nodes). Assume all newly added edges
to have weight 1.
*/

int main()
{
    srand(time(0));

    Shared_mem_info shm_node_list(200, 201, 202);
    Shared_mem_info shm_edge_list(203, 204, 205);

    cout << (*shm_node_list.capacity) << " " << (*shm_node_list.size) << "\n";
    cout << (*shm_edge_list.capacity) << " " << (*shm_edge_list.size) << "\n";

    node *node_list = shared_memory_init<node>(shm_node_list);

    node *edge_list = shared_memory_init<node>(shm_edge_list);

    vector<int> degree(*shm_node_list.size);

    for (int i = 0; i < (*shm_node_list.size); i++)
        degree[i] = get_degree(i, shm_node_list, node_list, edge_list);

    while (1)
    {

        int m = rand_num(10, 30);

        for (int i = 0; i < m; i++)
        {
            node_list[*shm_node_list.size].vertex = *shm_node_list.size;
            int x = *shm_node_list.size;
            *shm_node_list.size = (*shm_node_list.size) + 1;
            degree.push_back(0);

            int k = rand_num(1, 20);
            for (int j = 0; j < k; j++)
            {
                vector<long long> pref(degree.begin(), degree.end());
                partial_sum(degree.begin(), degree.end(), pref.begin());

                long long random_index = rand_num(1, pref.back());
                int y = --upper_bound(pref.begin(), pref.end(), random_index) - pref.begin();

                degree[x]++;
                degree[y]++;
                edge_list[*shm_edge_list.size].vertex = x;
                edge_list[*shm_edge_list.size + 1].vertex = y;

                edge_list[*shm_edge_list.size + 1].offset = node_list[x].offset;
                node_list[x].offset = *shm_edge_list.size + 1;

                edge_list[*shm_edge_list.size].offset = node_list[y].offset;
                node_list[y].offset = *shm_edge_list.size;

                *shm_edge_list.size = (*shm_edge_list.size) + 2;
            }
        }

        for (int i = 0; i < (*shm_node_list.size); i++)
        {
            cout << i << " :: ";
            int cur_off = node_list[i].offset;
            while (cur_off != -1)
            {
                cout << edge_list[cur_off].vertex << " ";
                cur_off = edge_list[cur_off].offset;
            }
            cout << "\n";
        }
        cout << "\n";
        sleep(50);
    }

    shmdt(node_list);
    shmdt(edge_list);

    return 0;
}