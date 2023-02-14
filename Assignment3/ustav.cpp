#include <bits/stdc++.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/shm.h>
#include <iostream>
#include <string.h>

using namespace std;

#define CONSUMER_COUNT 10

int main()
{
    key_t unique_key = ftok("./shared_memory_file", 15);

    if (unique_key == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    int shmid = shmget(unique_key, 1000, IPC_CREAT | 0666);
    if (shmid == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    char *str = (char *)shmat(shmid, (void *)0, 0);

    string graph = "";
    FILE *fp = fopen("my_graph.txt", "r");

    while (1)
    {
        char c = fgetc(fp);
        if (c == EOF)
            break;

        graph += c;
    }

    memcpy(str, graph.c_str(), graph.length());

    if (fork() == 0)
    {
        system("./producer");
        exit(EXIT_SUCCESS);
    }
    else
    {
        sleep(2);
        for(int i=0; i<CONSUMER_COUNT; ++i)
        {
            if (fork() == 0)
            {
                execl("./consumer", "./consumer", to_string(i).c_str(), NULL);
                exit(EXIT_SUCCESS);
            }
        }
    }

    sleep(30);

    cout << "Graph: \n" << str << endl;

    shmdt(str);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}