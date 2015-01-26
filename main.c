/*************************************************************************
	> File Name: main.c
	> Author: 
	> Mail: 
	> Created Time: 2015年01月24日 星期六 10时43分28秒
 ************************************************************************/

#include <stdio.h>
#include <signal.h>
#include "engine.h"
#include "sock.h"
void signal_handler(int sigm)
{
    static int f_in = 0;
    if(f_in == 0){
        stop_engine();
        f_in++;
    }
}
int usage(int argc,char **argv)
{
    if(argc <= 2){
        printf("Usage:%s [ip] [port]\n",argv[0]);
        printf("example:%s 192.168.1.1 100000\n",argv[0]);
        return -1;
    }
    init_sock(argc,argv);
    return 0;
}
int main(int argc,char **argv)
{
    if(signal(SIGINT,signal_handler) == SIG_ERR)
        fprintf(stderr,"signal send error\n");
    if(!usage(argc,argv)){
        start_engine();
    }
    return 0;
}
