#include <bits/stdc++.h>
#include <sys/shm.h>
using namespace std;

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        shmctl(atoi(argv[i]), IPC_RMID, NULL);
    }
    return 0;
}