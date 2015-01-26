/*************************************************************************
	> File Name: sock.h
	> Author: 
	> Mail: 
	> Created Time: 2015年01月26日 星期一 09时07分40秒
 ************************************************************************/

#ifndef _SOCK_H
#define _SOCK_H
struct client{
   int port;
   char *addr;
};


struct server{
    int sockfd;
    struct client client;
};

void init_sock(int argc,char **argv);
void exit_sock(void);
void send_sock(unsigned char*sendbuf,int len);


#endif
