/*
gcc hello_client.c -o client
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in serv_addr;
    char message[30] = {0};
    int str_len;

    if(argc != 3) {
        printf("Usage:%s <ip> <port>\n", argv[0]);
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        error_handling("socket() error");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {  //向服务器端发送连接请求。
        error_handling("connect()");
    }

    str_len = read(sockfd, message, sizeof(message)-1);
    if(str_len == -1) {
        error_handling("read()");
    }

    printf("Message from server:%s\n", message);
    close(sockfd);

    return 0;
}