#ifndef _HTTP_BASE_
#define _HTTP_BASE_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/mman.h>

#define PORT 8080
#define LISTENNUM 10
#define ERRFAIL -1
#define ERROK 0

// 客户端状态 ------ 3 表示连接并发送了 http 状态行
#define DISCONNECT 1
#define CONNECT 2
#define HEAD_CONNECT 3

#define SIZEMAX 10

#define WORDMAX 2048
#define SOCKETOFF -1
#define NETIPADDR 20
#define SERVERON 1
#define SERVEROFF 1

/*  客户端套接字; 端口; 连接状态; ip地址; http版本协议; 下一个客户端  */
typedef struct client_message
{
    int socket;
    int clientport;
    int status;
    char *clientaddr;
    char *version;
    struct client_message* next;
}cmsg, *Cmsg;

typedef struct network_message
{
    int server_socket;  // 服务器套接字
    int port;           // 服务器端口号
    int socket_max;     // 目前出现的最大套接字
    Cmsg client_data;
    /* data */
}netmsg, *Netmsg;



#endif