#include <unistd.h>
#include <dirent.h>
#include <iostream> 
#include <signal.h>

using namespace std;

// pwd() - print working directory
void pwd()
{
    char buf[1024];
    // getcwd() - get current working directory
    getcwd(buf, sizeof(buf));
    cout << buf << endl;
}

int main(int argc, char *argv[])
{
    // print working directory
    pwd();
    return 0; 
}
