#include <bits/stdc++.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>

using namespace std;

int find(int sz, int map[], int val)
{
    for(int i=0;i<sz;i++)
    {
        if(map[i] == val)
            return i;
    }
    return -1;
}

void djikstra(int **graph, int n, int start){
    int visited[n] = {0};

    int distance[n];
    for(int i=0;i<n;i++){
        distance[i] = INT_MAX;
    }

    visited[start] = 1;
    int parent[n];
    memset(parent, -1 , sizeof(parent));
    parent[start] = start;

    distance[start] = 0;
    for(int i=0;i<n;i++){
        if(graph[start][i] == 1){
            distance[i] = 1;
            parent[i] = start;
        }
    }

    for(int i=0;i<n-1;i++){

        int v = 0;
        int d = INT_MAX;

        for(int i=0;i<n;i++){
            if(visited[i] == 0 && distance[i] <= d){
                v = i;
                d = distance[i];
            }
        }

        visited[v] = 1;

        for(int i=0;i<n;i++){
            if(visited[i] == 0 && graph[v][i] == 1){
                if(distance[i] != INT_MAX){
                    distance[i] = min(distance[i], distance[v] + 1)
                    parent[i] = v;
                }
            }
        }
    }

    for(int i=0;i<n;i++){

    }
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
    int vertex, offset, map;
};


int main()
{
    Shared_mem_info shm_node_list(200, 201, 202);
    Shared_mem_info shm_edge_list(203, 204, 205);

    node *node_list = shared_memory_init<node>(shm_node_list);

    node *edge_list = shared_memory_init<node>(shm_edge_list);


    int n = *shm_node_list.size/10;

    int map[n];
    int idx = 0;

    
    for(int i=0;i<*shm_node_list.size;i++)
    {
        if(idx == n)
            break;
        if(node_list[i].map == -1)
        {
            map[idx] = i;
            idx++;
            node_list[i] = 1;
        }
    }

    int **graph;
    graph = (int **)malloc(sizeof(int *)*idx);
    for(int i=0;i<idx;i++)
    {
        graph[i] = (int *)malloc(sizeof(int)*idx);
        for(int j=0;j<idx;j++)
            graph[i][j] = 0;
    }

    for (int ind = 0; ind < (*shm_node_list.size); ind++)
    {
        int cur_off = node_list[ind].offset;
        while (cur_off != -1)
        {
            int i = find(n, map, ind);
            int j = find(n, map, edge_list[cur_off].vertex);
            if(i != -1 && j != -1)
            {
                graph[i][j] = 1;
                graph[j][i] = 1;
            }
            cur_off = edge_list[cur_off].offset;
        }
    }

    djikstra(graph, idx, 0);

    for(int i=0;i<idx;i++)
    {
        free(graph[i]);
        for(int j=0;j<idx;j++)
            graph[i][j] = 0;
    }
    free(graph);

    shmdt(node_list);
    shmdt(edge_list);
    shmdt(mapped);
    return 0;
}