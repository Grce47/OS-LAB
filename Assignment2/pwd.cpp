#include <unistd.h>
#include <dirent.h>
#include <iostream> 
#include <signal.h>

using namespace std;

void pwd()
{
    char buf[1024];
    getcwd(buf, sizeof(buf));
    cout << buf << endl;
}

int main(int argc, char *argv[])
{
    pwd();
    return 0; 
}
