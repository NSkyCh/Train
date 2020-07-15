#include "http_formatprocess.h"


httptype file_type[HTTP_NUMMAX] = {
    {".html",    "text/html"  },
    {".gif",     "image/gif"  },
    {".jpeg",    "image/jpeg" },
    {".png",     "image/png"  },
    { NULL,      "text/plain"  },
};

statusline errormsg[HTTP_NUMMAX] = {
    {"400",  "Bad Request"  , "Server don't Request"},
    {"404",  "Not Found" , "Server don't find file"},
    {"403",  "Forbidden"  , "Server don't run the CGI program"},
    {"501",  "Not Implemented" , "Server don't implement this method" },
    { NULL,      NULL},
};


/* 解析 uri */
int parse_uri(char *uri, char* filename) 
{
    const char* src = "ntskych.html";
    strncpy(filename, ".", 1);                           
	strncat(filename, uri, strlen(uri)); 
    if (uri[strlen(uri)-1] == '/') {
        strncat(filename, src, strlen(src));
        if(access(filename, F_OK) == ERRFAIL) {
            printf("%s don't find!!!\n", filename);
            return ERRFAIL;
        }
    }
    return ERROK;
}

/* 获取文件类型 */
int get_filetype(char *filename) 
{
    int filetype;

    for(filetype = 0; filetype >= (HTTP_NUMMAX - 1); filetype++) 
    {
        if (strstr(filename, file_type[filetype].suffix)) {
            break;
        }
    }

    return filetype;
}  

/* 发送文本处理 */
char* http_packethandle(char *buf, int filesize, int ttype, char *version)
{
    if(buf == NULL) {
        perror("buf is NULL");
        return NULL;
    }

    snprintf(buf, WORDMAX,  "%s 200 OK\r\n"
                            "Server: NtSkyCh's Web Server\r\n"
                            "Connection: close\r\n"
                            "Content-length: %d\r\n"
                            "Content-type: %s\r\n\r\n", \
                             version, filesize, file_type[ttype].type);
   
    return buf;
}


/* 发送http格式信息 */
int http_responsehandle_send(Cmsg client, const char *filename, int ttype)
{
    
    int filesize = 0;
    char *filebuf, *headpacket;
    int fliefd;
    printf("%s\n", filename);

    filesize = get_filesize(filename);
    headpacket = (char *)malloc(WORDMAX);
    if (headpacket == NULL) {
        perror("headpacket malloc");
        return ERRFAIL;
    }
    headpacket =  http_packethandle(headpacket, filesize, ttype, client->version);
    if (headpacket == NULL) {
        perror("http_packethandle");
        return ERRFAIL;
    }
   
    printf("Response headers:\n%s", headpacket);

    fliefd = open(filename, O_RDONLY, 0);
    if (fliefd < 0) {
        perror("open file");
        return ERRFAIL;
    }
    filebuf = mmap(0, filesize, PROT_READ, MAP_PRIVATE, fliefd, 0);
    close(fliefd);

    http_send(client, headpacket, 0);
    http_send(client, filebuf, filesize);
    munmap(filebuf, filesize);
    free(headpacket);

    return ERROK;
}

int get_filesize(const char *filename)
{
    FILE *file_fp; //, *fp;
    int filesize;

    file_fp = fopen(filename, "r+");
    if (file_fp == NULL) {
        perror("fopen");
        return ERRFAIL;
    }
 
    fseek(file_fp, 0, SEEK_END);
    filesize = ftell(file_fp);
    fseek(file_fp, 0, 0);
    printf("filesize = %d\n", filesize);
    fclose(file_fp);
    return filesize;
}


/* 发送信息 */
int http_send(Cmsg client, char* buf, unsigned int size)
{
    int ret;
    if (buf == NULL) {
        printf("buf = NULL");
        return ERRFAIL;
    }

    if (size == 0) {
        ret = send(client->socket, buf, strlen(buf), 0);
        if (ret <= ERRFAIL) {
            perror("send");
            return ret;
        }
    } else {
        ret = send(client->socket, buf, size, 0);
        if (ret <= ERRFAIL) {
            perror("send");
            return ret;
        }
    }
    
    
    return ERROK;
}

/* 发送错误信息 */
void http_errsend(Cmsg client, char *uri, int etype, char *version)
{
    char headbuf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    snprintf(body, MAXBUF,  "<html><title>Server Error</title>"
                            "<body bgcolor=""ffffff"">\r\n"
                            "%s: %s\r\n<p>%s: %s\r\n"
                            "<hr><em>The Web server</em>\r\n", \
                            errormsg[etype].StatusCode, errormsg[etype].StatusMsg, \
                            errormsg[etype].errtext, uri);

    /* Print the HTTP response */
    snprintf(headbuf, MAXBUF, "%s %s %s\r\n"
                              "Content-type: text/html\r\n"
                              "Content-length: %d\r\n\r\n", \
                               version, errormsg[etype].StatusCode, \
                               errormsg[etype].StatusMsg, (int)strlen(body));

    http_send(client, headbuf, 0);
    http_send(client, body, 0);
}

/* 客户端信息接受 */
int recv_msg(Cmsg client)
{
    ssize_t recvtype;
    int ret, ttype = 0, etype = 1;
    char filename[WORDMAX];
    char method[WORDMAX], uri[WORDMAX], version[WORDMAX];
    char buf[WORDMAX];

    memset(filename, 0, WORDMAX);
    memset(method, 0, WORDMAX);
    memset(uri, 0, WORDMAX);
    memset(version, 0, WORDMAX);
    memset(buf, 0, WORDMAX);

    recvtype = recv(client->socket, buf, WORDMAX, 0);
    if (recvtype < 0) {
        perror("recv data error");
        return ERRFAIL;
    } else if (recvtype == 0) {
        printf("%d is disconnect, ip = %s, port = %d\n", client->socket, \
               client->clientaddr, client->clientport);
        return DISCONNECT;
    } else {
        sscanf(buf, "%s %s %s", method, uri, version);
        client->version = version;      
        printf("%s", buf);

        if (strcasecmp(method, "GET")) {                     
            http_errsend(client, uri, etype, version);
            return ERRFAIL;
        }
 
        ret = parse_uri(uri, filename);
        if (ret < 0) {
            perror("parse_uri");
            return ERRFAIL;
        }
        ttype = get_filetype(filename); 
      
        http_responsehandle_send(client, filename, ttype);
    }

    return ERROK;
}
