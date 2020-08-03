/* server.c */
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>

#define MAXLINE 10
#define SERV_PORT 8000

int main(void)
{
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    int listenfd, connfd;
    char buf[MAXLINE+50];
    char str[INET_ADDRSTRLEN];
    int i, efd;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    listen(listenfd, 20);

    struct epoll_event event;
    struct epoll_event resevent[10];
    int res, len;

    efd = epoll_create(10);
    /* ET 边沿触发 ，默认是水平触发 */
    //event.events = EPOLLIN | EPOLLET;
    event.events = EPOLLIN;
    printf("Accepting connections ...\n");
    cliaddr_len = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
    printf("received from %s at PORT %d\n",
            inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
            ntohs(cliaddr.sin_port));

    event.data.fd = connfd;
    epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &event);
    while (1) {
        res = epoll_wait(efd, resevent, 10, -1);
        printf("res %d\n", res);
        if (resevent[0].data.fd == connfd) {
            //这里是为了演示阻塞的场景，当read读了很多次后，会阻塞。对于一个服务器来说，就不能响应别的客户端了。有没有办法编程非阻塞的read
            len = read(connfd, buf, sizeof(buf));
            write(STDOUT_FILENO, buf, len);
            len = read(connfd, buf, sizeof(buf));
            write(STDOUT_FILENO, buf, len);
            len = read(connfd, buf, sizeof(buf));
            write(STDOUT_FILENO, buf, len);
            len = read(connfd, buf, sizeof(buf));
            write(STDOUT_FILENO, buf, len);
        }
    }
    return 0;
}
