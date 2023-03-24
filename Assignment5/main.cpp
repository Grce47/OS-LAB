#include <iostream>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <set>
#include <time.h>
#include <vector>
#include <assert.h>
#include <semaphore.h>
#include <sys/types.h>
#include <fcntl.h>
#include <fstream>

using namespace std;

// returns a random number between l, r
int get_random(int l, int r)
{
    return l + (rand() % (r - l + 1));
}

// Class of guest
class Guest
{
public:
    int priority; // stores priority of guest
    int id;         // stores id of guest

    // constructor of guest
    Guest(int _prior, int _id)
    {
        priority = _prior;
        id = _id;
    }
};


// Room Class
class Room
{
public:
    Guest *guest; // pointer to guest which has occupied the room
    int previous_guest_time; // stores the time for which room was occupied
    int number_of_guests; // stores the number of guests who stayd in the room
    bool cleaned;   // stores whether room is cleaned or not
    Room()
    {
        number_of_guests = 0;
        previous_guest_time = 0;
        guest = NULL;
        cleaned = true;
    }
};

int x, y, n;
Room *rooms;
sem_t *sem_id, sem_cleaner, sem_clean_wait;
pthread_mutex_t mutex_id;
pthread_cond_t cond_id;
pthread_t *guest_tid, *staff_tid;
vector<Guest> guests;
multiset<pair<int, int>> room_priority_queue;
int uncleaned_rooms;
ofstream out;
 // including guest thread
#include "guest.cpp"

// including cleaner thread
#include "cleaning_staff.cpp"

int main()
{
    // intializing all the vriables
    uncleaned_rooms = 0;
    pthread_mutex_init(&mutex_id, NULL);
    pthread_cond_init(&cond_id, NULL);
    time(NULL);
    cout<<"Enter the number of cleaners: ";
    cin>>x;
    cout<<"Enter the number of guests: ";
    cin>>y;
    cout<<"Enter the number of rooms: ";
    cin>>n;

    out.open("sns.log");
    if (!(y > n && n > x && x > 1))
    {
        cerr << "Wrong Constraints\n";
        exit(EXIT_FAILURE);
    }

    // for (int i = 1; i <= y; i++)
    // {
    //     Guest guest(get_random(1,y), i);
    //     guests.emplace_back(guest);
    // }

    rooms = new Room[n]();
    guest_tid = new pthread_t[y];
    staff_tid = new pthread_t[x];
    sem_id = new sem_t[y];

    sem_init(&sem_cleaner, 0, 1);
    sem_init(&sem_clean_wait, 0, 0);

    for (int i = 0; i < y; i++)
        sem_init(&sem_id[i], 0, 0);

    for (int i = 0; i < n; i++)
        room_priority_queue.insert(make_pair(0, i));

    for (int i = 0; i < y; i++)
    {
        Guest guest(i + 1, i);
        guests.push_back(guest);
    }

    // creating all the threads
    for (int i = 0; i < y; i++)
        pthread_create(&guest_tid[i], NULL, guest_thread, (void *)(&guest_tid[i]));
    for (int i = 0; i < x; i++)
        pthread_create(&staff_tid[i], NULL, cleaning_staff_thread, NULL);
    for (int i = 0; i < y; i++)
        pthread_join(guest_tid[i], NULL);
    for (int i = 0; i < x; i++)
        pthread_join(staff_tid[i], NULL);

    pthread_mutex_destroy(&mutex_id);
    pthread_cond_destroy(&cond_id);

    exit(EXIT_SUCCESS);
}
