#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <vector>
#include <assert.h>
using namespace std;

int get_random(int l, int r)
{
    return l + (rand() % (r - l + 1));
}

class Guest
{
private:
    int priority;

public:
    Guest(int _priority)
    {
        priority = _priority;
    }
    int get_priority()
    {
        return this->priority;
    }
};

class Room
{
public:
    Guest *guest;
    Room()
    {
        guest = NULL;
    }
};

int x, y, n;
vector<Guest> guests;
Room *rooms;
pthread_t *guest_tid, *staff_tid;

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

    pthread_exit(0);
}

void *cleaning_staff_thread(void *args)
{
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
        pthread_create(&guest_tid[i], NULL, guest_thread, (void *)(&guest_tid[i]));
    for (int i = 0; i < x; i++)
        pthread_create(&staff_tid[i], NULL, cleaning_staff_thread, NULL);
    for (int i = 0; i < y; i++)
        pthread_join(guest_tid[i], NULL);
    for (int i = 0; i < x; i++)
        pthread_join(staff_tid[i], NULL);
    

    exit(EXIT_SUCCESS);
}
