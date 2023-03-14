#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
    int x;
    printf("%d\n", getpid());
    printf("%d\n",x = open("lock_test.txt", O_RDWR));
    printf("%d\n",flock(x, LOCK_SH));
    fflush(stdout);
    while(1);
    printf("%d\n",flock(x, LOCK_UN));
    return 0;
}
