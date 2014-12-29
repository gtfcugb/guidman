#include <assert.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/sem.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/syscall.h>
#include <syslog.h>
#include <dlfcn.h>

#define IP "192.168.3.188"
#define PORT 5000

void *processGo(void*arg){
    int conn_fd;
	struct sockaddr_in serv_addr_in;
    /*必须清0*/
    memset(&serv_addr_in, 0, sizeof(struct sockaddr_in));
    serv_addr_in.sin_family = AF_INET;
    /*字节转换*/
    serv_addr_in.sin_port = htons(PORT);
    inet_aton(IP, &serv_addr_in.sin_addr);
    conn_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(connect(conn_fd, (struct sockaddr *) &serv_addr_in, sizeof(serv_addr_in)) <0){
        printf("连接失败 %s %d \n",IP,PORT);
        exit(0);
    }
    char sendMsg[1024];
    char key[1024];
    char value[1024];
    int i =0;
    
    struct timeval tv;
	struct timeval tv1;

    gettimeofday(&tv,0);
    for(i =0;i < 1000000;i++){
        sprintf(value,"testkey_%d",1);
        sprintf(sendMsg,"S*guid*get*0*%d\n%s",strlen(value),value);
        //printf("%s\n",sendMsg);
        send(conn_fd,sendMsg,strlen(sendMsg),0);
        recv(conn_fd,sendMsg,1024,0);
        //printf("%s\n",sendMsg);
    }

    gettimeofday(&tv1,0);
    printf("time %ld  \n",(tv1.tv_sec-tv.tv_sec) * 1000000 + (tv1.tv_usec - tv.tv_usec)); 
    return NULL;
}

int i=0;
int main(){
    for(i=0;i< 10;i++){
        int tid;
        pthread_create(&tid, NULL, (void *)processGo, NULL);
    }
    printf("main thread go to sleep \n");
    sleep(10000);
    return 0;
}
