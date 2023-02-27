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
    for (int i = 0; i < sz; i++)
    {
        if (map[i] == val)
            return i;
    }
    return -1;
}

void djikstra(vector<vector<int>> &graph, int n, int start, vector<vector<int>> &dist, vector<vector<int>> &parent)
{
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    pq.push({0, start});
    dist[start][start] = 0;
    parent[start][start] = start;

    while (!pq.empty())
    {
        int u = pq.top().second;
        int d_u = pq.top().first;
        pq.pop();

        if (d_u != dist[start][u])
            continue;

        for (auto v : graph[u])
        {
            if (dist[start][v] > dist[start][u] + 1)
            {
                dist[start][v] = dist[start][u] + 1;
                parent[start][v] = u;
                pq.push({dist[start][v], v});
            }
        }
    }

}

void optimizer(vector<vector<int>> &graph, int n, vector<vector<int>> &dist, vector<vector<int>> &parent,int new_n){
    cout<<"optimizer"<<endl;
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            for(int k = n - new_n; k < n; k++){
                if(dist[k][i] != INT_MAX && dist[k][j] != INT_MAX && dist[i][j] > dist[k][i] + dist[k][j]){
                    dist[i][j] = dist[k][i] + dist[k][j];
                    int cur = j;
                    while(cur != k){
                        parent[i][cur] = parent[k][cur];
                        cur = parent[k][cur];
                    }
                    cout<<i<<" "<<j<<endl;
                    cur = i;
                    while(cur != k){
                        parent[i][parent[k][cur]] = cur;
                        cur = parent[k][cur];
                    }
                }
            }
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

int main(int argc, char *argv[])
{
    freopen(argv[1], "w", stdout);
    Shared_mem_info shm_node_list(200, 201, 202);
    Shared_mem_info shm_edge_list(203, 204, 205);

    node *node_list = shared_memory_init<node>(shm_node_list);

    node *edge_list = shared_memory_init<node>(shm_edge_list);

    if(argc < 3){

        int n = *shm_node_list.size / 10;
        int *map = (int *)malloc(n * sizeof(int));
        int idx = 0;  
        vector<vector<int>> dist, parent;
        while(1){
            n = *shm_node_list.size / 10;

            map = (int *)realloc(map, n*(sizeof(int)));

            cout<<*shm_node_list.size<<" "<<idx<<endl;

            for (int i = 0; i < *shm_node_list.size; i++)
            {
                if (idx == n)
                    break;
                if (node_list[i].map == -1)
                {
                    map[idx] = i;
                    node_list[i].map = 1;
                    idx++;
                }
            }

            vector<vector<int>> graph;
            graph.resize(idx);
            dist.resize(idx);
            parent.resize(idx);

            for (int ind = 0; ind < (*shm_node_list.size); ind++)
            {
                int cur_off = node_list[ind].offset;
                while (cur_off != -1)
                {
                    int i = find(n, map, ind);
                    int j = find(n, map, edge_list[cur_off].vertex);
                    if (i != -1 && j != -1)
                    {
                        graph[i].push_back(j);
                        graph[j].push_back(i);
                    }
                    cur_off = edge_list[cur_off].offset;
                }
                for(int i=0;i<idx;i++){
                    dist[i].push_back(INT_MAX);
                    parent[i].push_back(-1);
                }
            }


            for (int i = 0; i < idx; i++)
            {
                djikstra(graph, idx, i, dist, parent);

                for (int j = 0; j < idx; j++)
                {
                    int cur = j;
                    if (dist[i][cur] == INT_MAX)
                    {
                        continue;
                    }
                    if (j != i)
                    {
                        vector<int> path;
                        while (parent[i][cur] != cur)
                        {
                            path.push_back(cur);
                            cur = parent[i][cur];
                        }
                        path.push_back(cur);
                        reverse(path.begin(), path.end());
                        int n = path.size();
                        for (int i = 0; i < n - 1; i++)
                            cout << map[path[i]] << "--> ";
                        cout << map[path[n - 1]] << endl;
                    }
                }
            }

            sleep(30);
        }
    }else{
        int n = *shm_node_list.size / 10;
        int *map = (int *)malloc(n * sizeof(int));
        int idx = 0;
        int prev_idx = 0;
        vector<vector<int>> graph;
        vector<vector<int>> dist, parent;
        while(1){
            n = *shm_node_list.size / 10;
            map = (int *)realloc(map, n*(sizeof(int)));
            for (int i = 0; i < *shm_node_list.size; i++)
            {
                if (idx == n)
                    break;
                if (node_list[i].map == -1)
                {
                    map[idx] = i;
                    node_list[i].map = 1;
                    idx++;
                }
            }

            cout<<*shm_node_list.size<<" "<<idx<<endl;

            graph.resize(idx);
            dist.resize(idx);
            parent.resize(idx);

            for(int i=prev_idx;i<idx;i++){
                int cur_off = node_list[map[i]].offset;
                while(cur_off != -1){
                    int i = find(n, map, map[i]);
                    int j = find(n, map, edge_list[cur_off].vertex);
                    if(i != -1 && j != -1){
                        graph[i].push_back(j);
                        graph[j].push_back(i);
                    }
                    cur_off = edge_list[cur_off].offset;
                }
                for(int j=0;j<idx;j++){
                    dist[i].push_back(INT_MAX);
                    parent[i].push_back(-1);
                }
            }

            cout<<'b'<<endl;


            for (int i = prev_idx; i < idx; i++)
            {
                djikstra(graph, idx, i, dist, parent);
            }

            cout<<'a'<<endl;
            optimizer(graph, idx, dist, parent, (idx - prev_idx));

            for (int i = 0; i < idx; i++)
            {
                for (int j = 0; j < idx; j++)
                {
                    int cur = j;
                    if (dist[i][cur] == INT_MAX){
                        cout<<map[i]<<endl;
                        continue;
                    }
                    if (j != i)
                    {
                        vector<int> path;
                        while (parent[i][cur] != cur)
                        {
                            path.push_back(cur);
                            cur = parent[i][cur];
                        }
                        path.push_back(cur);
                        reverse(path.begin(), path.end());
                        int n = path.size();
                        for (int i = 0; i < n - 1; i++)
                            cout << map[path[i]] << "--> ";
                        cout << map[path[n - 1]] << endl;
                    }
                }
            }
            prev_idx = idx;

            sleep(30);
        }
    }

    shmdt(node_list);
    shmdt(edge_list);
    return 0;
}