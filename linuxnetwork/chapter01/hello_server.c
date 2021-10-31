/*
gcc hello_server.c -o server
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
    int serv_socket;
    int clnt_socket;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    char message[] = "hello world";

    if(argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    serv_socket = socket(AF_INET, SOCK_STREAM, 0);  //创建套接字
    if(serv_socket == -1) {
        error_handling("socket() error");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {  //bind函数分配ip地址和端口号
        error_handling("bind() error");
    }

    if(listen(serv_socket, 5) == -1) {   //把套接字转为可接收连接的状态
        error_handling("listen() error");    
    }

    clnt_addr_size = sizeof(clnt_addr);
    clnt_socket = accept(serv_socket, (struct sockaddr *)&clnt_addr, &clnt_addr_size);  // 受理连接请求，如果没有连接请求的情况下，不会返回，直到有请求为止
    if(clnt_socket == -1) {
        error_handling("accept() error");
    }

    write(clnt_socket, message, sizeof(message));
    close(clnt_socket);
    close(serv_socket);
    return 0;
}