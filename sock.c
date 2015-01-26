/*************************************************************************
	> File Name: sock.c
	> Author: 
	> Mail: 
	> Created Time: 2015年01月26日 星期一 09时13分46秒
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <malloc.h>
#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include "sock.h"
struct server *server;
void init_sock(int argc,char **argv)
{
    struct sockaddr_in remote;
    int len = sizeof(remote);

    server = (struct server*)malloc(sizeof(struct server));
    server->client.port = atoi(argv[2]);
    server->client.addr = strdup(argv[1]);

    remote.sin_family = AF_INET;
    remote.sin_port = htons(atoi(argv[2]));
    remote.sin_addr.s_addr=inet_addr(argv[1]);

    server->sockfd = socket(AF_INET,SOCK_DGRAM,0);
    connect(server->sockfd,(const struct sockaddr*)&remote,len);

}
void send_sock(unsigned char *sendbuf,int len)
{
    int ret=0;
    ret=send(server->sockfd,sendbuf,len,0);
    if(ret < 0){
        printf("send error\n");
        exit(1);
    }
}



