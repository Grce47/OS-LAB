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

// Component of edge list
struct node
{
    int vertex, offset;
};

int rand_num(int l, int r)
{
    return l + (rand() % (r - l + 1));
}

int main()
{
    srand(time(0));

    Shared_mem_info shm_node_list(200, 201, 202);
    Shared_mem_info shm_edge_list(203, 204, 205);

    cout << (*shm_node_list.capacity) << " " << (*shm_node_list.size) << "\n";
    cout << (*shm_edge_list.capacity) << " " << (*shm_edge_list.size) << "\n";

    node *node_list = shared_memory_init<node>(shm_node_list);

    node *edge_list = shared_memory_init<node>(shm_edge_list);

    // for (int i = 0; i < *shm_edge_list.size; i++)
    // {
    //     cout << edge_list[i].vertex << " ";
    // }
    // cout << "\n";
    
    for (int ind = 0; ind < (*shm_node_list.size); ind++)
    {
        cout << ind << " :: ";
        int cur_off = node_list[ind].offset;
        while (cur_off != -1)
        {
            cout << edge_list[cur_off].vertex << " ";
            cur_off = edge_list[cur_off].offset;
        }
        cout << "\n";
    }

    cout << "here";

    shmdt(node_list);
    shmdt(edge_list);

    return 0;
}