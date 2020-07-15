#include "http_server.h"


/* 运行代码 */
int run_loop(Netmsg network_message)
{
    // Netmsg network_message = *(Netmsg*)data_message;
    Cmsg client = network_message->client_data;
    Cmsg current_client;

    int ret;
    int flag = SERVERON;
    fd_set readfds, tempfds;
    struct timeval timeout;
	FD_ZERO(&readfds);
    FD_SET(network_message->server_socket, &readfds);

    while (flag) {
        tempfds = readfds;
        current_client = client;
        timeout.tv_sec = 5000;
        ret = select(network_message->socket_max + 1, &tempfds, NULL, NULL, &timeout);
        if (ret < 0) {
            perror("select error");
            return ERRFAIL;
        } else if(ret == 0) {
            //printf("timeout\n");
            continue;
        }
        if (FD_ISSET(network_message->server_socket, &tempfds)) { 
            current_client = set_connect(client);
            ret = establish_connecting(network_message, current_client);
            if(ret < 0) {
                perror("connecting error");
                return ERRFAIL;
            }
            FD_SET(current_client->socket, &readfds);
            network_message->socket_max = \
                        (network_message->socket_max > current_client->socket) ? \
                         network_message->socket_max : current_client->socket;
        }

        current_client = client;
        while (current_client != NULL) {
            if(FD_ISSET(current_client->socket, &tempfds)) {
                ret = recv_msg(current_client);
                if(ret < 0) {
                    perror("recv_msg error");
                    FD_CLR(current_client->socket, &readfds);
                    return ret;
                } else {
                    FD_CLR(current_client->socket, &readfds);
                    close(current_client->socket);
                    current_client->socket = SOCKETOFF;
                    current_client->status = DISCONNECT;  
                }
            }
            current_client = current_client->next;
        }
    }
    return ERROK;
}

/* 服务器建立 */
int socket_init(Netmsg* data_message)
{
    int ret;
    char buf[WORDMAX];
    Netmsg network_message = data_malloc(data_message);
    network_message->server_socket = SOCKETOFF;
    network_message->client_data->socket = SOCKETOFF;

    network_message->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (network_message->server_socket < 0) {
        perror("socket error");
        return ERRFAIL;
    }

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(PORT);

    ret = bind(network_message->server_socket, \
        (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (ret < ERROK) {
        perror("bind error");
        return ret;
    }

    ret = listen(network_message->server_socket, LISTENNUM);
    if (ret < ERROK) {
        perror("listen error");
        return ret;
    }
    snprintf(buf, WORDMAX, "server_socket = %d, ip = %s, port = %d", network_message->server_socket,\
             inet_ntoa(serveraddr.sin_addr), htons(serveraddr.sin_port));
    printf("%s\n", buf);
    network_message->socket_max = network_message->server_socket;

    return ERROK;
}

/* 数据申请内存 */
Netmsg data_malloc(Netmsg* data_message)
{
    Netmsg network_message;  
    network_message = (Netmsg)malloc(sizeof(netmsg));
    if (network_message == NULL) {
        perror("network_message malloc error");
        exit(-1);
    }
    network_message->client_data = (Cmsg)malloc(sizeof(cmsg));
    if ((network_message)->client_data == NULL) {
        perror("client_data malloc error");
        free(network_message);
        network_message = NULL;
        exit(-1);
    }
    network_message->client_data->next = NULL;
    network_message->client_data->status = DISCONNECT;
    network_message->client_data->status = SOCKETOFF;

    *data_message = network_message;
    return network_message;
}

/* 建立连接 */
int establish_connecting(Netmsg network_message, Cmsg client)
{
    // Cmsg client = network_message->client_data;
    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof(clientaddr);
    clientlen = sizeof(clientaddr);
    client->socket = accept(network_message->server_socket, \
                     (struct sockaddr *)&clientaddr, &clientlen);
    if (client->socket < 0) {
        perror("accept error");
        return ERRFAIL;
    }
    client->clientaddr = inet_ntoa(clientaddr.sin_addr);
    client->clientport = htons(clientaddr.sin_port);
    client->status = CONNECT;
    printf("client ip = %s, port = %d\n", client->clientaddr, client->clientport);
    return ERROK;
}


/* 设置已经使用的链表 */
Cmsg set_connect(Cmsg client)
{
    Cmsg current_client = client;  

    while (current_client!= NULL) {
        if(current_client->status == DISCONNECT) {
            return current_client;
        } else if (current_client->next == NULL) {
            break;
        } else {
            current_client = current_client->next;
        }     
    }
    current_client->next = (Cmsg)malloc(sizeof(cmsg));
    if(current_client->next == NULL) {
        perror("current_client malloc error");
        exit(-1);
    }
    current_client = current_client->next;
    current_client->next = NULL; 

    return current_client;
}


void socket_close(Netmsg network_message)
{
    Cmsg client;
    for (client = network_message->client_data; client != NULL;\
                  client = client->next) {
        close(client->socket);
        client->socket = SOCKETOFF;
    }

    if(network_message->server_socket != 0) {
        close(network_message->server_socket);
        network_message->server_socket = SOCKETOFF;
    }

    if(network_message != NULL) {
        free(network_message->client_data);
        free(network_message);
        network_message = NULL;
    }
}