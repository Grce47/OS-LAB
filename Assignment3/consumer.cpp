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

void djikstra(vector<vector<int>> &graph, int n, int start, int map[]){
    vector<int> dist(n, INT_MAX);
    vector<int> parent(n, -1);

    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    pq.push({0, start});
    dist[start] = 0;
    parent[start] = start;

    while(!pq.empty()){
        int u = pq.top().second;
        int d_u = pq.top().first;
        pq.pop();

        if(d_u != dist[u])
            continue;

        for(auto v: graph[u]){
            if(dist[v] > dist[u] + 1){
                dist[v] = dist[u] + 1;
                parent[v] = u;
                pq.push({dist[v], v});
            }
        }
    }

    for(int i=0;i<n;i++){
        int cur = i;
        if(dist[cur] == INT_MAX)
            continue;
        if(i != start){
            vector<int> path;
            while(parent[cur] != cur){
                path.push_back(cur);
                cur = parent[cur];
            }
            path.push_back(cur);
            reverse(path.begin(), path.end());
            int n = path.size();
            for(int i=0;i<n-1;i++)
                cout << map[path[i]] << "--> ";
            cout<<map[path[n-1]]<<endl;
        }
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
            node_list[i].map = 1;
            idx++;
        }
    }

    vector<vector<int>> graph;
    graph.resize(idx);

    for (int ind = 0; ind < (*shm_node_list.size); ind++)
    {
        int cur_off = node_list[ind].offset;
        while (cur_off != -1)
        {
            int i = find(n, map, ind);
            int j = find(n, map, edge_list[cur_off].vertex);
            if(i != -1 && j != -1)
            {
                graph[i].push_back(j);
                graph[j].push_back(i);
            }
            cur_off = edge_list[cur_off].offset;
        }
    }

    for(int i=0;i<idx;i++)
    {
        djikstra(graph, idx, 0, map);
    }

    shmdt(node_list);
    shmdt(edge_list);
    return 0;
}