#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <vector>
#include <assert.h>
#include <semaphore.h>
#include <sys/types.h>
#include <fcntl.h>

using namespace std;

int get_random(int l, int r)
{
    return l + (rand() % (r - l + 1));
}

class Guest
{
public:
    int priority;
    Guest(int _prior)
    {
        priority = _prior;
    }
};

class Room
{
public:
    Guest *guest;
    int previous_guest;
    bool cleaned;
    Room()
    {
        cleaned = true;
        previous_guest = 0;
        guest = NULL;
    }
};

int x, y, n;
Room *rooms;
sem_t sem_id;
pthread_t *guest_tid, *staff_tid;
vector<Guest> guests;

void *guest_thread(void *args)
{
    int guest_index = -1;
    for (int i = 0; i < y; i++)
    {
        if ((pthread_t *)args == &guest_tid[i])
        {
            guest_index = i;
            break;
        }
    }
    assert(guest_index != -1);
    Guest *guest = &guests[guest_index];
    while (1)
    {
        sleep(get_random(10, 20));

        // a case

        // b case
        // assert no room empty

        for (int i = 0; i < n; i++)
        {
            if (rooms[i].guest->priority < guest->priority)
            {
                // do swapping
            }
        }
    }
    pthread_exit(0);
}

void *cleaning_staff_thread(void *args)
{
    while (1)
    {
        // wakes up when required

        // select room
        vector<int> idx;
        for (int i = 0; i < n; i++)
            if (!rooms[i].cleaned)
                idx.push_back(i);

        int select_idx = idx[get_random(0, idx.size() - 1)];
        Room *selected_room = &rooms[select_idx];

        // do cleaning
        sleep(selected_room->previous_guest);
        selected_room->previous_guest = 0;
        selected_room->cleaned = true;
    }

    pthread_exit(0);
}

int main()
{
    time(NULL);
    cin >> x >> y >> n;
    if (!(y > n && n > x && x > 1))
    {
        cerr << "Wrong Constraints\n";
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i <= y; i++)
    {
        Guest guest(i);
        guests.emplace_back(guest);
    }

    rooms = new Room[n]();
    guest_tid = new pthread_t[y];
    staff_tid = new pthread_t[x];

    for (int i = 0; i < y; i++)
    {
        Guest guest(get_random(1, y));
        guests.push_back(guest);
    }

    for (int i = 0; i < y; i++)
        pthread_create(&guest_tid[i], NULL, guest_thread, (void *)(&guest_tid[i]));
    for (int i = 0; i < x; i++)
        pthread_create(&staff_tid[i], NULL, cleaning_staff_thread, NULL);
    for (int i = 0; i < y; i++)
        pthread_join(guest_tid[i], NULL);
    for (int i = 0; i < x; i++)
        pthread_join(staff_tid[i], NULL);

    exit(EXIT_SUCCESS);
}
