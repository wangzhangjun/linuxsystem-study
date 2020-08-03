#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>

#define MAXLINE 10

int main(int argc, char *argv[])
{
    int efd, i;
    int pfd[2];
    pid_t pid;
    char buf[MAXLINE], ch = 'a';
    pipe(pfd);

    pid = fork();
    if (pid == 0) { //子进程
        close(pfd[0]);  //关闭读端
        while (1) {
            //aaaa\n
            for (i = 0; i < MAXLINE/2; i++)
                buf[i] = ch;
            buf[i-1] = '\n';
            ch++;//变成字符b
            //bbbb\n
            for (; i < MAXLINE; i++)
                buf[i] = ch;
            buf[i-1] = '\n';
            ch++;
            //aaaa\nbbbb\n
            write(pfd[1], buf, sizeof(buf));  //写到管道里面
            sleep(5);   //子进程睡眠5s
        }
        close(pfd[1]);
    }
    else if (pid > 0) {
        struct epoll_event event;
        struct epoll_event resevent[10]; //epoll_wait就绪返回event
        int res, len;
        close(pfd[1]);//关闭写端
        efd = epoll_create(10);
        /* ET 边沿触发 ，默认是水平触发 */
        event.events = EPOLLIN | EPOLLET;  //改成边缘是出发,会发现每次打印完aaa，bbb要等5s才会打印
        // event.events = EPOLLIN;  //这句，会发现每次直接把aaa,bbb同时打印出来，因为是水平触发
        event.data.fd = pfd[0];
        epoll_ctl(efd, EPOLL_CTL_ADD, pfd[0], &event);
        while (1) {
            res = epoll_wait(efd, resevent, 10, -1);  //第一次读了5个字节后，又回来，这个时候如果是电平式触发，就会又读一次
            printf("res %d\n", res);
            if (resevent[0].data.fd == pfd[0]) {   //判断是不是管道读端
                len = read(pfd[0], buf, MAXLINE/2);  //读一半，只读5个字节
                write(STDOUT_FILENO, buf, len);
            }
        }
        close(pfd[0]);
        close(efd);
    }
    else {
        perror("fork");
        exit(-1);
    }

    return 0;
}
