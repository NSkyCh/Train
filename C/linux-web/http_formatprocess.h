#ifndef _HTTP_FORMATPROCESS_
#define _HTTP_FORMATPROCESS_

#include "http_base.h"

#define HTTP_NUMMAX 5
#define HTTP_HEADMAX 50
#define MAXLINE 2024
#define MAXBUF 8096


typedef struct http_type
{
    char *suffix; // 文件后缀
    char *type;   // 类型文件

}httptype, *HttpType;

typedef struct StatusLine
{
    char *StatusCode;    // 状态码
    char *StatusMsg; // 状态消息
    char *errtext;

}statusline, *StatusLine;

int parse_uri(char *uri, char* filename);
int recv_msg(Cmsg client);
int http_responsehandle_send(Cmsg client, const char *filename, int ttype);
int get_filesize(const char *filename);
int http_send(Cmsg client, char* buf, unsigned int size);
void http_errsend(Cmsg client, char *uri, int etype, char *version);
int get_filetype(char *filename);
char* http_packethandle(char *buf, int filesize, int ttype, char *version);

#endif