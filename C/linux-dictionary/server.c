/******************************************
	#	FileName  : server.c
	#	Author	  : OYJS 
	#	QQ	      : 3014067790 
	#	Email	  : 3014067790@qq.com
	#	Created   : Wed 14 Aug 2019 08:05:33 PM CST
 ****************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <linux/in.h>
#include <time.h>
#include <unistd.h>
#include <sqlite3.h>



#define N 256
#define M 32

typedef struct message
{
	char type; 
	char name[M]; 
	char data[N]; // password or word 
}msg,*Msg;

int init_start(int,const char **);
int ep_init(int);
void recv_info(int,int);
void add_connect(int,int);
void register_data(int,msg);
void load_data(int ,msg);
void find_word(int,msg);
void record_insert(msg);
char *time_paket();
void record_word(int);
//int callback(void *arg,int f_num,char **f_value,char **f_name);



int main(int argc, const char *argv[])
{
	int sockfd,acceptfd;
	char buf[N];

	sockfd = init_start(argc,argv);

	int epfd,epct,i;
	struct epoll_event events[20];

	epfd=ep_init(sockfd);
	while(1)
	{
		epct = epoll_wait(epfd,events,20,-1);
		for(i=0;i<epct;i++)
		{
			if(events[i].data.fd == sockfd)
			{
				add_connect(epfd,events[i].data.fd);
			}
			else
			{
				recv_info(epfd,events[i].data.fd);
			}
		}
	}
	return 0;
}

int init_start(int argc,const char **argv)
{
	if(argc < 3)
	{
		printf("please use %s <IP> <PORT>.\n",argv[0]);
		exit(-1);
	}

	int sockfd;
	struct sockaddr_in serveraddr;
	int serverlen = sizeof(serveraddr);
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd<0)
	{
		perror("sockfd");
		exit(-1);
	}

	printf("sockfd ok!\n");

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(atoi(argv[2]));
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);

	int val=1;
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));

	if(bind(sockfd,(struct sockaddr *)&serveraddr,serverlen)<0)
	{
		perror("Failed to bind");
		exit(-1);
	}
	printf("bind success\n");
	if(listen(sockfd,10)<0)
	{
		perror("Failed to listen");
		exit(-1);
	}
	printf("listen success\n");
	return sockfd;
}

int ep_init(int sockfd)
{
	struct epoll_event event;
	int epfd;
	epfd=epoll_create(1);

	event.data.fd = sockfd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&event);
	return epfd;
}

void add_connect(int epfd,int fd)
{	
	struct epoll_event event;
	struct sockaddr_in clientaddr;
	
	int acceptfd;
	char buf[N];
	memset(buf,0,sizeof(buf));

	int clientlen = sizeof(clientaddr);

	acceptfd = accept(fd,(struct sockaddr *)&clientaddr,&clientlen);
	printf("");
//	time_paket(buf);

	event.data.fd = acceptfd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epfd,EPOLL_CTL_ADD,acceptfd,&event);
}


void recv_info(int epfd,int fd)
{	
	struct epoll_event event;
	char buf[N];
	ssize_t recvtype;
	msg redata;

//	memset(&redata,0,sizeof(redata));
	memset(buf,0,sizeof(buf));
	recvtype = recv(fd,&redata,sizeof(msg),0);
	
	if(recvtype < 0)
	{
		perror("recv");
	}
	else if(recvtype == 0)
	{
		printf("%c,%s,%s\n",redata.type,redata.name,redata.data);
		if(redata.type == 0)
		{
			printf("a visitor is disconnect\n",redata.name);
		}
		else
		{
			printf("%s is disconnect\n",redata.name);
		}
		close(fd);
		epoll_ctl(epfd,EPOLL_CTL_DEL,fd,&event);
	}
	else
	{
		printf("%c,%s,%s\n",redata.type,redata.name,redata.data);
	//	printf("yse\n");
		switch(redata.type)
		{
			case 'r':
			{
				register_data(fd,redata);
				memset(&redata,0,sizeof(redata));
				break;
			}
			case 'l':
			{
				load_data(fd,redata);
				break;
			}
			case 'f':
			{
				find_word(fd,redata);
				break;
			}
			case 'h':
			{
				record_word(fd);
				break;
			}
			case 'q':
			{
			//	client_quit();
				printf("%s is close\n",redata.name);
				memset(&redata,0,sizeof(redata));
				break;
			}
		}
	}
}


void register_data(int fd,msg redata)
{
//	printf("%c,%s,%s\n",redata.type,redata.name,redata.data);
	
	sqlite3 *db;
	if(sqlite3_open("test.db",&db) != 0)
	{
		fprintf(stderr,"sqlite3_open failed: %s\n",sqlite3_errmsg(db));
	}
	printf("register_open ok.\n");

	char *errmsg = NULL;
	int re;
	re = sqlite3_exec(db,"select * from usrdata;",NULL,NULL,&errmsg);
	if(re != 0)
	{
		if(sqlite3_exec(db,\
					"create table usrdata(usrname char primary key,password char);",\
					NULL,NULL,&errmsg) != 0)
		{
			fprintf(stderr,"create failed %s\n",errmsg);
			exit(-1);
		}
		printf("register create ok.\n");
	}
	else
	{
		char buf[N];
		msg send_data;
		memset(buf,0,sizeof(buf));
		memset(&send_data,0,sizeof(msg));

		sprintf(buf,"insert into usrdata values(\"%s\",\"%s\");",\
				redata.name,redata.data);
		if(sqlite3_exec(db,buf,NULL,NULL,&errmsg) != 0)
		{
			send_data.type='e'; //e = error
			sprintf(send_data.data,"username:%s already exists",redata.name);
			send(fd,&send_data,sizeof(msg),0);
		}
		else
		{
			send_data.type='s';//s = success
			strcpy(send_data.data,"registration success");
			send(fd,&send_data,sizeof(msg),0);
			printf("register insert ok.\n");
		}
	}
	sqlite3_close(db);
}

void load_data(int fd,msg redata)
{
	sqlite3 *db;
	if(sqlite3_open("test.db",&db) != 0)
	{
		fprintf(stderr,"sqlite3_open failed: %s\n",sqlite3_errmsg(db));
	}
	printf("load_open ok.\n");


	char *errmsg = NULL;
	char buf[N];
	msg send_data;
	memset(buf,0,sizeof(buf));
	memset(&send_data,0,sizeof(msg));

	sprintf(buf,"select * from usrdata where usrname=\"%s\" and password=\"%s\";"\
			,redata.name,redata.data);

	if(sqlite3_exec(db,buf,NULL,NULL,&errmsg) != 0)
	{	
		send_data.type='e'; //e = error
		strcpy(send_data.data,"Username or password is incorrect");
		send(fd,&send_data,sizeof(msg),0);
	}
	else
	{
		send_data.type='s'; //s = success
		strcpy(send_data.data,"load success");
		send(fd,&send_data,sizeof(msg),0);
	}
	sqlite3_close(db);
}


void find_word(int fd,msg redata)
{

	record_insert(redata);

	FILE *fp;
	fp = fopen("dict.txt","r");
	if(fp == NULL)
	{
		perror("error");
		exit(-1);
	}
	int num;
	char buf[N],exp[N];
	char temp[N]="Failed to find the word";
	msg send_data;
	memset(&send_data,0,sizeof(msg));
	while(1)
	{
		num=fscanf(fp,"%s ",buf);
	//	printf("%s\n",buf);
		fgets(exp,N,fp);
		exp[strlen(exp)-1]='\0';
	//	printf("%s\n",exp);
		if(num<=0)
		{
			strcpy(send_data.data,temp);
			send(fd,&send_data,sizeof(msg),0);
			break;
		}
		else
		{
			if(strcmp(redata.data,buf) ==0 )
			{
				sprintf(send_data.data,"%s %s",buf,exp);
				printf("%s\n",send_data.data);
				send(fd,&send_data,sizeof(msg),0);
				break;
			}
			
		}
	}
	fclose(fp);
}
void record_insert(msg redata)
{
	sqlite3 *db;
	if(sqlite3_open("test.db",&db) != 0)
	{
		fprintf(stderr,"sqlite3_open failed: %s\n",sqlite3_errmsg(db));
	}
	printf("records_open ok.\n");

	char *errmsg = NULL;
	int re;
	re = sqlite3_exec(db,"select * from records;",NULL,NULL,&errmsg);
	if(re != 0)
	{
		if(sqlite3_exec(db,\
					"create table records(usrname char,word char,time char);",\
					NULL,NULL,&errmsg) != 0)
		{
			fprintf(stderr,"create failed %s\n",errmsg);
			exit(-1);
		}
		printf("records create ok.\n");
	}
	else
	{
		char buf[N];
		char *find_time;
		msg send_data;
		memset(buf,0,sizeof(buf));
		memset(&send_data,0,sizeof(msg));
		
		find_time = time_paket();
		printf("%s\n",find_time);
		sprintf(buf,"insert into records values(\"%s\",\"%s\",\"%s\");",\
				redata.name,redata.data,find_time);
		if(sqlite3_exec(db,buf,NULL,NULL,&errmsg) != 0)
		{
			fprintf(stderr,"insert failed %s\n",errmsg);
			exit(-1);
		}
		else
		{
			free(find_time);
			printf("records insert ok.\n");
		}
	}
	sqlite3_close(db);
}

char* time_paket()
{
	time_t t;
	t=time(NULL);
	struct tm* tt;
	tt=localtime(&t);

	char *find_time1=NULL;
	find_time1 = (char *)malloc(20);
	
	memset(find_time1,0,20);

	sprintf(find_time1,"%04d-%02d-%02d %02d:%02d:%02d",\
			tt->tm_year+1900,tt->tm_mon+1,tt->tm_mday,\
			tt->tm_hour,tt->tm_min,tt->tm_sec);
//	printf("%s\n",find_time1);
	return find_time1;
}

int callback(void *arg,int f_num,char **f_value,char **f_name)
{
//	printf("%d\n",*((int *)arg));
//	printf("%d\n",*((int *)arg+1));
//	int num = *((int *)arg);
	int fd = *((int *)arg+1);

//	printf("%d\n",num);
//	printf("%d\n",fd);
	
	int i;
	char buf[N],temp[N];
	memset(buf,0,sizeof(buf));
	memset(temp,0,sizeof(temp));
	msg send_data;
	memset(&send_data,0,sizeof(msg));

	if( *((int *)arg) > 0 )
	{
		for(i = 0;i < f_num;i++)
		{
			if(i == 0)
			{
				sprintf(buf,"%s    \t",f_name[i]);
				strcpy(temp,buf);
			}
			else if(i == f_num-1)
			{
				sprintf(buf,"%s%s",temp,f_name[i]);
			//	strcpy(temp,buf);
			}
			else
			{
				sprintf(buf,"%s%s    \t",temp,f_name[i]);
				strcpy(temp,buf);
			}
		//	printf("%s    \t",f_name[i]);
		}
		*((int *)arg)=-1;
		printf("%s\n",buf);
	}
	else
	{
		for(i = 0;i < f_num;i++)
		{
			if(i == 0)
			{
				sprintf(buf,"%s\t\t",f_value[i]);
				strcpy(temp,buf);
			}
			else if(i == f_num-1)
			{
				sprintf(buf,"%s%s",temp,f_value[i]);
			//	strcpy(temp,buf);
			}
			else
			{
				sprintf(buf,"%s%s\t\t",temp,f_value[i]);
				strcpy(temp,buf);
			}
		//	printf("%s\t\t",f_value[i]);
		}
		printf("%s\n",buf);
	}
	
	strcpy(send_data.data,buf);
	send_data.type = 'c'; //c = continue
	send(fd,&send_data,sizeof(msg),0);
	return 0;
}


void record_word(int fd)
{
	sqlite3 *db;
	if(sqlite3_open("test.db",&db) != 0)
	{
		fprintf(stderr,"sqlite3_open failed: %s\n",sqlite3_errmsg(db));
	}
	printf("records_open ok.\n");

	char *errmsg = NULL;
	int re;
	msg send_data;
	char buf[N];
	char temp[]="no historical records";
	int num[2]={1,fd};
	memset(&send_data,0,sizeof(msg));

	re = sqlite3_exec(db,"select * from records;",callback,&num,&errmsg);
	if(re != 0)
	{
		strcpy(send_data.data,temp);
		send_data.type='e';
		send(fd,&send_data,sizeof(msg),0);
	}
	else
	{
		sleep(1);
		send_data.type='s';
		send(fd,&send_data,sizeof(msg),0);
	}
	
}
