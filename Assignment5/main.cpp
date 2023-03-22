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

int get_random(int l, int r)
{
    return l + (rand() % (r - l + 1));
}

class Guest
{
public:
    int priority;
    int id;
    Guest(int _prior, int _id)
    {
        priority = _prior;
        id = _id;
    }
};

class Room
{
public:
    Guest *guest;
    int previous_guest_time;
    int number_of_guests;
    bool cleaned;
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
pthread_mutex_t mutex_id, cleaner_lock;
pthread_cond_t cond_id, cleaner_cond;
pthread_t *guest_tid, *staff_tid;
vector<Guest> guests;
multiset<pair<int, int>> room_priority_queue;
int uncleaned_rooms;
int room_full;
ofstream out;

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

        pthread_mutex_lock(&mutex_id);

        while (room_priority_queue.begin()->first >= guest->priority)
        {
            pthread_cond_wait(&cond_id, &mutex_id);
        }
        int room_no = room_priority_queue.begin()->second;

        rooms[room_no].number_of_guests++;

        if (room_priority_queue.begin()->first != 0)
        {
            // intterupt signal
            Guest *guest_ = rooms[room_no].guest;
            sem_post(&sem_id[guest_->id]);
            out<<"Removing the Guest: "<<guest_->id<<" with priority "<<room_priority_queue.begin()->first<<" from Room "<<room_no<<" and "<<"Adding the guest: "<<guest_index<<" with priority "<<guest->priority<<endl;
        }else{
            out<<"Adding the Guest: "<<guest_index<<" with priority "<<guest->priority<<" to the empty room "<<room_no<<endl;
        }

        room_priority_queue.erase(room_priority_queue.begin());
        if (rooms[room_no].number_of_guests < 2)
        {
            room_priority_queue.insert(make_pair(guest->priority, room_no));
        }
        else
        {
            room_priority_queue.insert(make_pair(y + 1, room_no));
        }

        rooms[room_no].guest = guest;

        int random_number = get_random(10, 30);
        struct timespec sleep_time;
        clock_gettime(CLOCK_REALTIME, &sleep_time);
        sleep_time.tv_sec += random_number;

        pthread_mutex_unlock(&mutex_id);
        pthread_cond_broadcast(&cond_id);

        int res = -1;
        res = sem_timedwait(&sem_id[guest->id], &sleep_time);

        pthread_mutex_lock(&mutex_id);
        if (res == 0)
        {
            // interrupt
            // do nothing
        }
        else
        {
            // not interrupt
            out<< "Guest: "<<guest_index<<" with priority: "<<guest->priority<<" left the room: "<<room_no<<" after compeleting its stay"<<endl;
            if (rooms[room_no].number_of_guests < 2)
            {
                room_priority_queue.erase(room_priority_queue.find(make_pair(guest->priority, room_no)));
                room_priority_queue.insert(make_pair(0, room_no));
            }
            else
            {
                uncleaned_rooms++;
                rooms[room_no].cleaned = false;
                if (uncleaned_rooms == n){
                    for(int i=0;i<n;i++){
                        sem_post(&sem_clean_wait);
                    }
                }
            }
            rooms[room_no].guest = NULL;
            rooms[room_no].previous_guest_time += random_number;
        }

        pthread_mutex_unlock(&mutex_id);
        pthread_cond_broadcast(&cond_id);
    }
    pthread_exit(0);
}

void *cleaning_staff_thread(void *args)
{
    while (1)
    {
        // wakes up when required
        sem_wait(&sem_clean_wait);

        // select room
        sem_wait(&sem_cleaner);
        vector<int> idx;
        for (int i = 0; i < n; i++)
            if (!rooms[i].cleaned)
                idx.push_back(i);

        int select_idx = idx[get_random(0, idx.size() - 1)];
        Room *selected_room = &rooms[select_idx];
        selected_room->cleaned = true;
        out<<"Cleaning Room No. "<<select_idx<<endl;
        sem_post(&sem_cleaner);

        // do cleaning
        sleep(selected_room->previous_guest_time);
        selected_room->previous_guest_time = 0;
        selected_room->guest = NULL;
        selected_room->number_of_guests = 0;

        sem_wait(&sem_cleaner);
        uncleaned_rooms--;
        if(uncleaned_rooms == 0){
            room_full = 0;
            auto itr = room_priority_queue.end();
            itr--;
            while (itr->first == y+1)
            {
                auto temp = *itr;
                room_priority_queue.erase(itr);
                room_priority_queue.insert({0,temp.second});
                itr = room_priority_queue.end();
                itr--;
            }     
            pthread_cond_broadcast(&cond_id);  
        }
        sem_post(&sem_cleaner);
    }

    pthread_exit(0);
}

int main()
{
    room_full = 0;
    uncleaned_rooms = 0;
    pthread_mutex_init(&mutex_id, NULL);
    pthread_mutex_init(&cleaner_lock, NULL);
    pthread_cond_init(&cond_id, NULL);
    pthread_cond_init(&cleaner_cond, NULL);
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
