#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <vector>
using namespace std;

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
};

int x, y, n;
vector<Guest> guests;

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

    exit(EXIT_SUCCESS);
}