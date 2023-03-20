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
    int previous_guest_time;
    int number_of_guests;
    bool cleaned;
    Room()
    {
        number_of_guests = 0;
        previous_guest_time = 0;
        guest = NULL;
    }
};

int x, y, n;
Room *rooms;
sem_t *sem_id, sem_cleaner;
pthread_mutex_t mutex_id;
pthread_cond_t cond_id;
pthread_t *guest_tid, *staff_tid;
vector<Guest> guests;
multiset<pair<int, int>> room_priority_queue;
void display()
{
    cout << "Priority | Room" << endl;
    for (auto &ele : room_priority_queue)
    {
        cout << ele.first << "\t\t\t" << ele.second << endl;
    }
}

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
        cout << endl
             << "Starting queue" << endl;
        display();
        cout << "THIS GUEST IS " << guest->priority << endl;
        cout << "About to get ROOM = " << room_no << " GUEST with Priority = " << guest->priority << endl;

        // rooms[room_no].number_of_guests++;
        // if (rooms[room_no].number_of_guests <= 2)
        //     sem_wait(&sem_cleaner);

        cout << "We are Removing " << room_priority_queue.begin()->first << " from room number = " << room_no << " and ";
        cout << "ADDing " << guest->priority << " from room number = " << room_no << endl;

        if (room_priority_queue.begin()->first != 0)
        {
            // intterupt signal
            cout << "(because queue.first.priority != 0) SEM_POST = " << room_priority_queue.begin()->first << " FROM " << guest->priority << endl;
            sem_post(&sem_id[room_no]);
        }
        room_priority_queue.erase(room_priority_queue.begin());
        room_priority_queue.insert(make_pair(guest->priority, room_no));

        cout << "After Erasing and inserting" << endl;
        display();
        cout << endl;

        rooms[room_no].guest = guest;

        pthread_cond_signal(&cond_id);
        pthread_mutex_unlock(&mutex_id);

        int random_number = get_random(10, 30);
        struct timespec sleep_time;
        clock_gettime(CLOCK_REALTIME, &sleep_time);
        sleep_time.tv_sec += random_number;

        int res = sem_timedwait(&sem_id[room_no], &sleep_time);

        pthread_mutex_lock(&mutex_id);
        if (res == 0)
        {
            // interrupt
            // do nothing
            cout << "GOT INTERRUPT " << guest->priority << " " << endl;
        }
        else
        {
            // not interrupt
            cout << "Peace REM " << guest->priority << " " << room_no << " ADD " << 0 << " " << room_no << endl;
            room_priority_queue.erase(room_priority_queue.find(make_pair(guest->priority, room_no)));
            room_priority_queue.insert(make_pair(0, room_no));
            rooms[room_no].guest = NULL;
            rooms[room_no].previous_guest_time += random_number;
        }

        pthread_cond_signal(&cond_id);
        pthread_mutex_unlock(&mutex_id);
    }
    pthread_exit(0);
}

void *cleaning_staff_thread(void *args)
{
    // while (1)
    // {
    //     // wakes up when required

    //     // select room
    //     vector<int> idx;
    //     for (int i = 0; i < n; i++)
    //         if (!rooms[i].cleaned)
    //             idx.push_back(i);

    //     int select_idx = idx[get_random(0, idx.size() - 1)];
    //     Room *selected_room = &rooms[select_idx];

    //     // do cleaning
    //     sleep(selected_room->previous_guest);
    //     selected_room->previous_guest = 0;
    //     selected_room->cleaned = true;
    // }

    pthread_exit(0);
}

int main()
{
    pthread_mutex_init(&mutex_id, NULL);
    pthread_cond_init(&cond_id, NULL);
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
    sem_id = new sem_t[n];

    sem_init(&sem_cleaner, 0, 2 * n);

    for (int i = 0; i < n; i++)
        sem_init(&sem_id[i], 0, 0);

    for (int i = 0; i < n; i++)
        room_priority_queue.insert(make_pair(0, i));

    for (int i = 0; i < y; i++)
    {
        Guest guest(i + 1);
        guests.push_back(guest);
    }
    display();
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
