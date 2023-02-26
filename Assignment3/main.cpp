#include <bits/stdc++.h>
#include <sys/shm.h>
#include <sys/wait.h>
using namespace std;

#define VERTEX_CNT 80000
#define EDGE_CNT 5000000
#define CONSUMER_CNT 10

int set_capacity(key_t key, int val)
{
    int shmid = shmget(key, sizeof(int), IPC_CREAT | 0666);
    if (shmid < 0)
    {
        cerr << "ERROR IN CAPACITY SETTING\n";
        exit(EXIT_FAILURE);
    }
    int *cap = (int *)shmat(shmid, NULL, 0);
    *cap = val;
    shmdt(cap);
    return shmid;
}

class Shared_mem_info
{
public:
    int shmid, *capacity, *size;
    key_t key;
    Shared_mem_info(key_t _key, key_t _capacity_key, key_t _size_key)
    {
        shmid = -1;
        key = _key;
        capacity = (int *)shmat(shmget(_capacity_key, sizeof(int), IPC_CREAT | 0666), NULL, 0);
        size = (int *)shmat(shmget(_size_key, sizeof(int), IPC_CREAT | 0666), NULL, 0);
    }
};

template <class T>
T *shared_memory_init(Shared_mem_info &shm, bool intialize_to_null = false, int def_val = 0)
{
    shm.shmid = shmget(shm.key, (*shm.capacity) * sizeof(T), IPC_CREAT | 0666);
    if (shm.shmid < 0)
    {
        cerr << "SHMID NOT MADE IN INIT\n";
        exit(EXIT_FAILURE);
    }
    T *ptr = (T *)shmat(shm.shmid, NULL, 0);
    if (intialize_to_null)
        memset(ptr, def_val, (*shm.capacity) * sizeof(T));
    return ptr;
}

struct node
{
    int vertex, offset;
};

int main()
{
    key_t key_node_list = 200, key_node_list_cap = 201, key_node_no = 202;
    key_t key_edge_list = 203, key_edge_list_cap = 204, key_egde_no = 205;

    int edge_cap_shm = set_capacity(key_edge_list_cap, EDGE_CNT);
    int node_cap_shm = set_capacity(key_node_list_cap, VERTEX_CNT);

    Shared_mem_info shm_node_list(key_node_list, key_node_list_cap, key_node_no);
    Shared_mem_info shm_edge_list(key_edge_list, key_edge_list_cap, key_egde_no);

    int edge_size_shm = set_capacity(key_egde_no, 0);
    int node_size_shm = set_capacity(key_node_no, 0);

    node *node_list = shared_memory_init<node>(shm_node_list, true, -1);
    node *edge_list = shared_memory_init<node>(shm_edge_list, true, -1);

    ifstream is("facebook_combined.txt");
    int x, y;
    while (is >> x >> y)
    {
        edge_list[*shm_edge_list.size].vertex = x;
        edge_list[*shm_edge_list.size + 1].vertex = y;

        if (node_list[x].vertex == -1)
        {
            node_list[x].vertex = x;
            node_list[x].offset = *shm_edge_list.size + 1;
            *shm_node_list.size = (*shm_node_list.size) + 1;
        }
        else
        {
            edge_list[*shm_edge_list.size + 1].offset = node_list[x].offset;
            node_list[x].offset = *shm_edge_list.size + 1;
        }

        if (node_list[y].vertex == -1)
        {
            node_list[y].vertex = y;
            node_list[y].offset = *shm_edge_list.size;
            *shm_node_list.size = (*shm_node_list.size) + 1;
        }
        else
        {
            edge_list[*shm_edge_list.size].offset = node_list[y].offset;
            node_list[y].offset = *shm_edge_list.size;
        }

        *shm_edge_list.size = (*shm_edge_list.size) + 2;
    }

    // Uncomment below for printing adjacency list
    // for (int ind = 0; ind < (*shm_node_list.size); ind++)
    // {
    //     cout << ind << " :: ";
    //     int cur_off = node_list[ind].offset;
    //     while (cur_off != -1)
    //     {
    //         cout << edge_list[cur_off].vertex << " ";
    //         cur_off = edge_list[cur_off].offset;
    //     }
    //     cout << "\n";
    // }

    if (fork() == 0)
    {
        // call the producer process
        execvp("./producer.out", NULL);
        exit(EXIT_SUCCESS);
    }

    for (int i = 0; i < CONSUMER_CNT; i++)
    {
        if (fork() == 0)
        {
            // call the consumer process
            exit(EXIT_SUCCESS);
        }
    }

    while (waitpid(-1, NULL, 0))
    {
        if (errno == ECHILD)
            break;
    }

    shmdt(shm_edge_list.capacity);
    shmdt(shm_node_list.capacity);

    shmdt(shm_edge_list.size);
    shmdt(shm_node_list.size);

    shmdt(node_list);
    shmdt(edge_list);
    shmctl(shm_edge_list.shmid, IPC_RMID, NULL);
    shmctl(shm_node_list.shmid, IPC_RMID, NULL);
    shmctl(edge_cap_shm, IPC_RMID, NULL);
    shmctl(node_cap_shm, IPC_RMID, NULL);
    shmctl(edge_size_shm, IPC_RMID, NULL);
    shmctl(node_size_shm, IPC_RMID, NULL);

    return 0;
}