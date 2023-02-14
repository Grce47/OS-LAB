#include <bits/stdc++.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define vert_cnt 80000
#define edge_cnt 5000000
#define consumer_cnt 10

// Component of edge list
struct node {
    int vertex;
    node* next;
};

// Function for adding new edges to adjacency list
node* add_edge(int u, int v, node* edge_list)
{
    edge_list[0].vertex = u;
    edge_list[1].vertex = v;
    return edge_list + 2;
}

// Function for modifying adjacency list to connect it with node list
void process_edge(int u, int v, node* edge_list, node** node_list)
{
    if(node_list[u] == NULL) node_list[u] = edge_list - 1;
    else
    {
        node* temp = node_list[u];
        node_list[u] = edge_list - 1;
        node_list[u]->next = temp;
    }
    if(node_list[v] == NULL) node_list[v] = edge_list - 2;
    else
    {
        node* temp = node_list[v];
        node_list[v] = edge_list - 2;
        node_list[v]->next = temp;
    }
}

int main()
{
    key_t key1 = 200, key2 = 201;
    int shmid1 = shmget(key1, vert_cnt, IPC_CREAT | 0666);
    if(shmid1 < 0)
    {
        perror("Shared Memory 1 allocation failed\n");
        exit(EXIT_FAILURE);
    }
    node** node_list = (node**)shmat(shmid1, NULL, 0);
    memset(node_list, 0, vert_cnt);

    int shmid2 = shmget(key2, edge_cnt, IPC_CREAT | 0666);
    if(shmid2 < 0)
    {
        perror("Shared Memory 2 allocation failed\n");
        exit(EXIT_FAILURE);
    }
    node* edge_list = (node*)shmat(shmid2, NULL, 0);
    memset(edge_list, 0, edge_cnt);

    FILE* ptr = fopen("facebook_combined.txt", "r");
    char str[50];
    if(ptr == NULL)
    {
        perror("Graph file can't be opened\n");
        exit(EXIT_FAILURE);
    }
    while(fgets(str, 50, ptr) != NULL)
    {
        int x, y;
        sscanf(str, "%d %d", &x, &y);
        edge_list = add_edge(x, y, edge_list);
        process_edge(x, y, edge_list, node_list);
    }
    fclose(ptr);

    // Uncomment below for printing adjacency list

    // for(int ind = 0; node_list[ind] != NULL; ind++)
    // {
    //     printf("%d :: ", ind);
    //     node* list_traverse = node_list[ind];
    //     while(list_traverse != NULL)
    //     {
    //         printf("%d ", list_traverse->vertex);
    //         list_traverse = list_traverse->next;
    //     }
    //     printf("\n");
    // }

    if(fork() == 0)
    {
        // call the producer process
        exit(EXIT_SUCCESS);
    }

    for(int i = 0; i < consumer_cnt; i++)
    {
        if(fork() == 0)
        {
            // call the consumer process
            exit(EXIT_SUCCESS);
        }
    }

    shmdt(node_list);
    shmdt(edge_list);
    shmctl(shmid1, IPC_RMID, NULL);
    shmctl(shmid2, IPC_RMID, NULL);
    return 0;
}