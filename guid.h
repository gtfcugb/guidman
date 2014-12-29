#ifndef _DL_GUID_
#define _DL_GUID_
#include <jemalloc/jemalloc.h>

#include <fcalg/fcalg.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

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
#include <sys/mman.h>

#include "../../def.h"
#include "../../mem.h"
#include "../../log.h"

extern FEvent* feventInstInit(char type,void*data,void*dataExtra,FEventHanle*handle);
extern int timerQueueSchedule(FTimerQueue *pQueue,char type,void*ptr,int interval);
extern int configHashGet(F_Hashtable*configHash,char*key,int *intValue,char**strValue);

typedef struct GuidMapSt
{
    char * pData;
    int  fileSize;
    int  nextPos;
    int  fd;
}GuidMap;


typedef struct GuidEleSt
{
    char*   key;
    int     keyLen;
    long*   pId;
}GuidEle;

int guid_engineDLInit(UpmanData * pUpmanData);
void guid_engineDLInput(UpmanData * pUpmanData,SocketClient*,SocketPacket*);
void guid_engineDLDestroy(UpmanData * pUpmanData);

#endif
