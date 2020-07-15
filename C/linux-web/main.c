#include "http_server.h"

int main(int argc, const char *argv[])
{
    Netmsg network_message;
    int ret;
    ret = socket_init(&network_message);
    if (ret < ERROK) {
        perror("socket_init error");
        socket_close(network_message);
        return ERRFAIL;
    }
    printf("socket_init ok\n");

    ret = run_loop(network_message);
    if (ret < ERROK) {
        perror("run_loop error");
        socket_close(network_message);
        return 0;
    }
   
    socket_close(network_message);
    return 0;
}