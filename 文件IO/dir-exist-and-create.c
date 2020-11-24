#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
struct stat st = {0};
int main()
{
        if(stat("/dev/socket/", &st) == -1)
        {
                printf("no dir and mkdir /dev/socket.\n");
                mkdir("/dev/socket/", 0755);
        }
}
