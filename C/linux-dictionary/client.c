/******************************************
	#	FileName  : client.c
	#	Author	  : OYJS 
	#	QQ	      : 3014067790 
	#	Email	  : 3014067790@qq.com
	#	Created   : Wed 14 Aug 2019 06:47:43 PM CST
 ****************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <sys/epoll.h>
#include <linux/in.h>
//#include <time.h>
//#include <unistd.h>

#define N 256
#define M 32



typedef struct message
{ 
	char type;
	char name[M]; 
	char data[N]; // password or word 
} msg,*Msg;

int init_start(int ,const char **);
void interface();
void option(int);
void register_send(int);
void load_send(int);
void find_interface();
int find_option(char *usrname,int sockfd);
void find_send(char *usrname,int sockfd);
void record_send(char *usrname,int sockfd);
void quit_send(char *usrname,int sockfd);


int main(int argc, const char *argv[])
{
	int sockfd;	
	pid_t pid;
	char name[32],buf[N];
	ssize_t recvtype;
	
	sockfd=init_start(argc,argv);
	while(1)
	{
		interface();
		option(sockfd);
	}

    return 0;
}

int init_start(int argc,const char **argv)
{
	int sockfd;
	struct sockaddr_in serveraddr;
	ssize_t recvtype;

	if(argc < 3)
	{
		printf("please use %s <IP> <port>.\n",argv[0]);
		exit(-1);
	}
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		perror("fail to socket");
		exit(-1);
	}

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);		
	serveraddr.sin_port = htons(atoi(argv[2]));
	socklen_t serverlen=sizeof(serveraddr);

	if(connect(sockfd,(struct sockaddr *)&serveraddr,serverlen) < 0)
	{
		perror("connect failed.");
		exit(-1);
	}
	printf("connect ok.\n");

	return sockfd;
}

void interface()
{
	printf("\e[1;31m*****************************************************\033[0m\n");
	printf("\e[1;31m****\t\t\t\t\t\t ****\033[0m\n");

	printf("\e[1;31m****\033[0m");
	printf("\e[1;33m	1.register\t2.load\t   3.quit	 \033[0m");
	printf("\e[1;31m****\n\033[0m");

	printf("\e[1;31m****\t\t\t\t\t\t ****\033[0m\n");
	printf("\e[1;31m*****************************************************\033[0m\n");
}

void option(int sockfd)
{
	int op;
	char buf[N];
	printf("input number(1,2 or 3) you need >> ");
	scanf("%d",&op);
	switch(op)
	{
	case 1:
		{
			register_send(sockfd);
			break;
		}
	case 2:
		{
			load_send(sockfd);
			break;
		}
	case 3:
		{
			exit(-1);
		}
	default:
		{
			printf("input error,please input num;\n");
			break;
		}
	}	
}

msg account_packet(char op)
{
	char buf[N];
	msg packet_data;

	memset(buf,0,sizeof(buf));
	memset(&packet_data,0,sizeof(msg));

	packet_data.type = op;
//	usleep(10000);	
	getchar();

	printf("please input your usename >> ");
	fgets(buf,sizeof(buf),stdin);
	buf[strlen(buf)-1]='\0';
	strncpy(packet_data.name,buf,M);

	memset(buf,0,sizeof(buf));
	printf("please input your password >> ");
	fgets(buf,sizeof(buf),stdin);
	buf[strlen(buf)-1]='\0';
	strcpy(packet_data.data,buf);

//	printf("%c,%s,%s\n",packet_data.type,packet_data.name\
			,packet_data.data);
	return packet_data;
}

void register_send(int sockfd)
{
	msg register_data,recv_data;
	char op = 'r';
	register_data = account_packet(op);

	printf("%c,%s,%s\n",register_data.type,register_data.name\
			,register_data.data);

	send(sockfd,&register_data,sizeof(msg),0);
	usleep(10000);

	recv(sockfd,&recv_data,sizeof(msg),0);
	printf("%c,%s\n",recv_data.type,recv_data.data);

}


void load_send(int sockfd)
{
	msg load_data,recv_data;
	char op = 'l';
	load_data = account_packet(op);
	char usrname[M];
	
	send(sockfd,&load_data,sizeof(msg),0);
	usleep(10000);

	recv(sockfd,&recv_data,sizeof(msg),0);
	printf("%c,%s\n",recv_data.type,recv_data.data);

	int flag;
	if(recv_data.type = 's')
	{
		strcpy(usrname,load_data.name);
		while(1)
		{
			find_interface();
			flag=find_option(usrname,sockfd);
			if(flag ==1 )
				break;
		}
	}

}


void find_interface()
{
	printf("\e[1;31m**************************************************************\033[0m\n");
	printf("\e[1;31m****\t\t\t\t\t\t\t  ****\033[0m\n");

	printf("\e[1;31m****\033[0m");
	printf("\e[1;33m\t 1.find word\t2.historical records\t3.quit\t\033[0m");
	printf("\e[1;31m  ****\n\033[0m");

	printf("\e[1;31m****\t\t\t\t\t\t\t  ****\033[0m\n");
	printf("\e[1;31m**************************************************************\033[0m\n");
}


int find_option(char *usrname,int sockfd)
{
	int op;
	int flag=0;
	char buf[N];
	printf("input number(1,2 or 3) you need >> ");
	scanf("%d",&op);
	switch(op)
	{
		case 1:
		{
			find_send(usrname,sockfd);
			break;
		}
		case 2:
		{
			record_send(usrname,sockfd);
			break;
		}
		case 3:
		{
			quit_send(usrname,sockfd);
			flag=1;
			break;
		}
		default:
		{
			printf("input error,please input num;\n");
			break;
		}
	}	
	return flag;
}

void find_send(char *usrname,int sockfd)
{
	char buf[N];
	msg find_data,recv_data;
	getchar();
	memset(buf,0,sizeof(buf));
	
	find_data.type = 'f';	//find word
	strcpy(find_data.name,usrname);

	printf("please input your word your want to find >> ");
	fgets(buf,sizeof(buf),stdin);
	buf[strlen(buf)-1]='\0';
	strcpy(find_data.data,buf);
	
	
	send(sockfd,&find_data,sizeof(msg),0);

	recv(sockfd,&recv_data,sizeof(msg),0);
	printf("%s\n",recv_data.data);
}

void record_send(char *usrname,int sockfd)
{
	char buf[N];
	msg record_data,recv_data;
	getchar();
	memset(buf,0,sizeof(buf));
	memset(&record_data,0,sizeof(msg));
	
	record_data.type = 'h';		//historical records
	strcpy(record_data.name,usrname);
	send(sockfd,&record_data,sizeof(msg),0);

	while(1)
	{
		memset(&recv_data,0,sizeof(msg));
		recv(sockfd,&recv_data,sizeof(msg),0);
		if(recv_data.type == 's'||recv_data.type == 'e')	//'q' = quit = end
		{
			break;
		}
		else
		{
			printf("%s\n",recv_data.data);
		}
	}
}
void quit_send(char *usrname,int sockfd)
{
	msg quit_data;
	quit_data.type = 'q';
	strcpy(quit_data.name,usrname);
	char buf[N]="quit";
	strcpy(quit_data.data,buf);
	send(sockfd,&quit_data,sizeof(msg),0);
}
