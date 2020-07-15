#ifndef _HTTP_SERVER_
#define _HTTP_SERVER_

#include "http_base.h"
#include "http_formatprocess.h"

int socket_init(Netmsg* network_message);
Netmsg data_malloc(Netmsg* data_message);
int run_loop(Netmsg network_message);
void socket_close(Netmsg network_message);
int establish_connecting(Netmsg network_message, Cmsg client);
Cmsg set_connect(Cmsg client);

#endif