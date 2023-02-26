#include <bits/stdc++.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>
using namespace std;

struct node
{
    int vertex;
    node *next;
};

int rand_num(int l, int r)
{
    return l + (rand() % (r - l + 1));
}

int main()
{
    srand(time(0));

    key_t key1 = 200, key2 = 201, key3 = 202;

    int shmid3 = shmget(key3, sizeof(int), IPC_CREAT | 0666);
    int *total_nodes = (int *)shmat(shmid3, NULL, 0);

    int shmid1 = shmget(key1, (sizeof(node *) * (*total_nodes)), IPC_CREAT | 0666);
    node **node_list = (node **)shmat(shmid1, NULL, 0);

    

    while (1)
    {
        sleep(50);
        int m = rand_num(10, 30);
        for (int i = 0; i < m; i++)
        {
            int k = rand_num(1, 20);
        }
    }

    return 0;
}