#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include "../common/wrap.h"
#define MAXLINE 80
#define SERV_PORT 8000
#define OPEN_MAX 1024
int main(int argc, char *argv[])
{
    int i, j, maxi, listenfd, connfd, sockfd;
    int nready, efd, res;
    ssize_t n;
    char buf[MAXLINE], str[INET_ADDRSTRLEN];
    socklen_t clilen;
    int client[OPEN_MAX];
    struct sockaddr_in cliaddr, servaddr;
    struct epoll_event tep, ep[OPEN_MAX];  //ep是接受epoll_wait返回的存储结构体的数组，tep是epoll_ctl插入时需要构造的
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    Bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    Listen(listenfd, 20);
    
    for (i = 0; i < OPEN_MAX; i++)
        client[i] = -1;
    maxi = -1;
    efd = epoll_create(OPEN_MAX);//创建根节点
    if (efd == -1)
        perr_exit("epoll_create");
    tep.events = EPOLLIN; tep.data.fd = listenfd; //
    res = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep);//把lfd添加到树
    if (res == -1)
        perr_exit("epoll_ctl");
    for ( ; ; ) {
        nready = epoll_wait(efd, ep, OPEN_MAX, -1); /* 阻塞监听 */
        if (nready == -1)
            perr_exit("epoll_wait");
        for (i = 0; i < nready; i++) {
            if (!(ep[i].events & EPOLLIN))//如果是写操作，就continue掉，从ep数组总获取第
                continue;
            if (ep[i].data.fd == listenfd) {
                clilen = sizeof(cliaddr);
                connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
                printf("received from %s at PORT %d\n", inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)), ntohs(cliaddr.sin_port));
                for (j = 0; j < OPEN_MAX; j++)
                    if (client[j] < 0) {
                        client[j] = connfd; /* save descriptor */
                        break;
                    }
            if (j == OPEN_MAX)
                perr_exit("too many clients");
            if (j > maxi)
                maxi = j; /* max index in client[] array */
            tep.events = EPOLLIN; tep.data.fd = connfd;
            res = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep);  //把新的fd添加到树中
            if (res == -1)
                perr_exit("epoll_ctl");
            }
            else {  //不是lfd，剩下的都是读
                sockfd = ep[i].data.fd;
                n = Read(sockfd, buf, MAXLINE);
                if (n == 0) {  //表示已经关闭
                    for (j = 0; j <= maxi; j++) {
                        if (client[j] == sockfd) {
                            client[j] = -1;
                            break;
                        }
                    }
                    res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);  //删除
                    if (res == -1)
                        perr_exit("epoll_ctl");
                    Close(sockfd);
                    printf("client[%d] closed connection\n", j);
                }else if(n < 0){
                    perror("read err");
                    res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
                    Close(sockfd);
                }
                else {
                    for (j = 0; j < n; j++)
                        buf[j] = toupper(buf[j]);
                    Writen(sockfd, buf, n);
                }
            }
        }
    }
    Close(listenfd);
    Close(efd);
    return 0;
}