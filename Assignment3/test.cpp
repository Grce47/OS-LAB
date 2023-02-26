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
    int shmid, *capacity, size;
    key_t key;
    Shared_mem_info(key_t _key, key_t _capacity_key)
    {
        key = _key;
        size = 0;
        capacity = (int *)shmat(shmget(_capacity_key, sizeof(int), IPC_CREAT | 0666), NULL, 0);
    }
};

template <class T>
T *shared_memory_init(Shared_mem_info &shm)
{
    shm.shmid = shmget(shm.key, (*shm.capacity) * sizeof(T), IPC_CREAT | 0666);
    if (shm.shmid < 0)
    {
        cerr << "SHMID NOT MADE IN INIT\n";
        exit(EXIT_FAILURE);
    }
    T *ptr = (T *)shmat(shm.shmid, NULL, 0);
    return ptr;
}

template <class T>
T *shared_memory_realloc(T *ptr, int new_size, Shared_mem_info &shm)
{
    T *arr = (T *)malloc(*shm.capacity * sizeof(T));
    for (int i = 0; i < *shm.capacity; i++)
        arr[i] = ptr[i];

    shmdt(ptr);
    shmctl(shm.shmid, IPC_RMID, NULL);

    shm.shmid = shmget(shm.key, new_size * sizeof(T), IPC_CREAT | 0666);
    if (shm.shmid == -1)
    {
        cerr << "SHMID NOT MADE IN REALLOC\n";
        exit(EXIT_FAILURE);
    }

    ptr = (T *)shmat(shm.shmid, NULL, 0);

    for (int i = 0; i < *shm.capacity; i++)
        ptr[i] = arr[i];

    free(arr);

    *shm.capacity = new_size;
    return ptr;
}

int main()
{
    // done in main

    int cap_shmid = shmget(201, sizeof(int), IPC_CREAT | 0666);
    int *cap = (int *)shmat(cap_shmid, NULL, 0);
    *cap = 3;

    // till here

    Shared_mem_info shm(200, 201);

    int *arr = shared_memory_init<int>(shm);

    for (int i = 0; i < *shm.capacity; i++)
        arr[i] = i;

    arr = shared_memory_realloc<int>(arr, 5, shm);

    arr[4] = arr[3] = -1;

    for (int i = 0; i < *shm.capacity; i++)
        cout << arr[i] << " ";

    shmdt(arr);
    shmctl(shm.shmid, IPC_RMID, NULL);
    return 0;
}