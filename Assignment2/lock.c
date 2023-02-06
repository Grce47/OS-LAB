#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
    int x;
    printf("%d",x = open("test.txt", O_RDWR));
    // while(1);
    printf("%d",flock(x, LOCK_SH));
    while(1);
    printf("%d",flock(x, LOCK_UN));
    // while(1);
    return 0;
}