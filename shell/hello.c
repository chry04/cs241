#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) 
{
    int n = atoi(argv[1]);

    for (int i = 0; i < n; i++)
    {
        sleep(3);
        printf("hello\n");
    }

    return 0;
}
