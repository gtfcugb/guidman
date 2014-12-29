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
#include <mysql/mysql.h>

void *processGo(void*arg){
    char*sql = "INSERT INTO s_id_generator (`name`) VALUES('testkey_1') ON DUPLICATE KEY UPDATE `id` = LAST_INSERT_ID(`id` + 1)"; 
    int sqlLen=strlen(sql);
    MYSQL   *link=NULL;
    link=mysql_init(link);

    link = mysql_real_connect(link,"localhost","root","","test",3306,NULL,0);
    assert(link != NULL);
    
    
    int i =0;
    
    struct timeval tv;
	struct timeval tv1;

    gettimeofday(&tv,0);
    for(i =0;i < 100000;i++){
        assert(!mysql_real_query(link,sql,sqlLen));
        MYSQL_RES * results = mysql_store_result(link);
        if(results){
            mysql_free_result(results);
        }
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
