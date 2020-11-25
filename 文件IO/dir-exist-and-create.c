#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
struct stat st = {0};
const char *dir_name = "/dev/socket/1/2";

static void _mkdir(const char *dir) {
        char tmp[256];
        char *p = NULL;
        size_t len;

        snprintf(tmp, sizeof(tmp),"%s",dir);
        len = strlen(tmp);
        if(tmp[len - 1] == '/')
                tmp[len - 1] = 0;
        for(p = tmp + 1; *p; p++)
                if(*p == '/') {
                        *p = 0;
                        mkdir(tmp, S_IRWXU);
                        *p = '/';
                }
        mkdir(tmp, S_IRWXU);
}

int main()
{
        if(stat(dir_name, &st) == -1)  //目录不存在
        {
                printf("no dir and mkdir %s.\n",dir_name);
                /*int res = mkdir(dir_name, 0755);
                if(res == -1){
                        perror("mkdir");
                }*/
                _mkdir(dir_name);
        }
}
