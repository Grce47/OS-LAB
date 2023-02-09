#include <stdlib.h>
#include <stdio.h>
int main()
{
    int i = 1;
    while(i++)
    {
        printf("%d\n",i);
        sleep(1); 
    }
    return 0;
}