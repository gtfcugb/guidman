#include "guid.h"
#include "file.h"
#include <sys/mman.h>

/*this global vars , make it running as a single instance*/
int GUID_FILESIZE_MAX   =   102400;
int GUID_MMAP_SYSC      =   10;
int GUID_KEY_LEN_MAX    =   32;

/*ID STEP*/
static int sIdStep      =   1;

static unsigned int hash_string(  void *p_key, unsigned int key_len , unsigned int bucket_num ){
    char	*psz	= (char *)p_key;;
    unsigned int	hash_value = 0;;
    unsigned int	ret	= 0;;
    unsigned int	i= 0;
    while ( key_len-- ){
    if ( i == 5 ){
        i = 0;
        ret += hash_value;
        hash_value = 0;
    }
    hash_value += hash_value << 3;
    hash_value += (unsigned int)tolower( *psz );
    psz++;
    i++;
    }
    ret += hash_value;
    return ret % bucket_num;
}

static int compareEle(void *data_one,void*data_two){
    char* key1      =   ((GuidEle *)data_one)->key;
    int key1Len     =   ((GuidEle *)data_one)->keyLen;
    char* key2      =   ((GuidEle *)data_two)->key;
    int key2Len     =   ((GuidEle *)data_two)->keyLen;
    
    int ret         =   memcmp(key1,key2,key1Len>key2Len?key2Len:key1Len);
    if(ret == 0){
        return key1Len - key2Len;
    }
	return ret;
}

/**这里的hash需要检测*/
static unsigned int hashEle(void *p_key,unsigned int bucket_num){
	unsigned int ret = hash_string( ((GuidEle *)p_key)->key, ((GuidEle *)p_key)->keyLen,bucket_num);
    return ret;
}


static F_Hashtable*sGuids=NULL;
/*we need syc the mmap file timely*/
static void guid_engineProcessForSync(EventLoop *eLoop,FEvent*pEvent){
    guid_fileSync();
    /*next*/
    FEvent * e = feventInstInit(EVENT_UPMAN,NULL,NULL,guid_engineProcessForSync);
    timerQueueSchedule(eLoop->pTimerQueue,TIMER_SYSTEM1,e,GUID_MMAP_SYSC);
}

int guid_engineDLInit(UpmanData * pUpmanData){
    sGuids   =      f_hashtable_create(1024,NULL,FMalloc,FRealloc,FFree,compareEle,hashEle);
    char*path=      pUpmanData->pUpmanConfig->SCRIPT_PARAM;

    configHashGet(pUpmanData->pUpmanConfig->EXTRA_HASH,"GUID_FILESIZE_MAX",&GUID_FILESIZE_MAX,NULL);
    configHashGet(pUpmanData->pUpmanConfig->EXTRA_HASH,"GUID_MMAP_SYSC",&GUID_MMAP_SYSC,NULL);
    configHashGet(pUpmanData->pUpmanConfig->EXTRA_HASH,"GUID_KEY_LEN_MAX",&GUID_KEY_LEN_MAX,NULL);
    configHashGet(pUpmanData->pUpmanConfig->EXTRA_HASH,"GUID_ID_STEP",&sIdStep,NULL);


    if(guid_fileInit(sGuids,path) != 0){
        return -1;
    }
    guid_engineProcessForSync(&(gServer.eLoop),NULL);
    return 0;
}

void guid_engineDLInput(UpmanData * pUpmanData,SocketClient*pSock,SocketPacket*pPacket){
    char* debug =pPacket->debug;
    int cGet    = 0;
    int cCheck  = 0;
    int cList   = 0;
    if(strcmp(pPacket->type,"get") == 0){
        cGet    = 1;
    }
    else if(strcmp(pPacket->type,"check") == 0){
        cCheck    = 1;
    }
    else if(strcmp(pPacket->type,"list") == 0){
        cList    = 1;
    }
    if(cGet || cCheck){
        /*get  we incre the guid by the value which is configed ,default is 1 ,then return it  */
        /*check  we just return the guid */
        GuidEle findMem;
        findMem.key         =   pPacket->rawBuffer + pPacket->ocuLen + pPacket->headLen;
        findMem.keyLen      =   pPacket->bodyLen;
        
        GuidEle*pMem        =   (GuidEle*)f_hashtable_find(sGuids,&findMem);
        /*1024 is redunctant ,but that's ok*/
        char response[1024];
        long retId          =   0;

        if(pMem == NULL){
            if(cGet){
                retId   = guid_fileAddKey(sGuids,findMem.key,findMem.keyLen,sIdStep);
            }
        }
        else{
            if(cGet){
                *pMem->pId  +=sIdStep;
            }
            retId    =   *pMem->pId;
        }
        
         /*64 is redunctant ,but that's ok*/
        char tmpId[64];
        sprintf(tmpId,"%ld",retId);

        sprintf(response,"%c%c%s%c%s%c%d%c%s%c%s",RESPONSE_TYPE_SOA,PACKET_SEP_CHAR,pPacket->mark,PACKET_SEP_CHAR,pPacket->type,PACKET_SEP_CHAR,(int)strlen(tmpId),PACKET_SEP_CHAR,debug,PACKET_HEAD_SEP,tmpId);

        send(pSock->fd,response,strlen(response),0);
    }
    else if(cList){
        /*dump all guid 10240 is magic num  */
        int infoSize        =   10240;
        char*   info        =   (char*)FMalloc(infoSize);
        if(info == NULL){
            
        }
        else{
            char    pos             =   0;
            F_H_Iter iter = f_hashtable_begin(sGuids);
            while(f_hashtable_end(sGuids,iter) != true){
                GuidEle *pMem       = (GuidEle*)f_hashtable_def(iter);
                char tmpId[64];
                sprintf(tmpId,"\n%ld\n",*pMem->pId);
                int idLen           = strlen(tmpId);

                if(pos + pMem->keyLen + idLen> infoSize){
                    infoSize *=2;
                    char *newInfo    =   (char*)FRealloc(info,infoSize);
                    if(newInfo==NULL){
                        break;
                    }
                    else{
                        info = newInfo;
                    }
                }
                
                memcpy(info+pos,pMem->key,pMem->keyLen);
                pos+=pMem->keyLen;
                memcpy(info+pos,tmpId,idLen);
                pos +=idLen;

                iter                = f_hashtable_next(sGuids,iter);
            }
            char head[1024];
            sprintf(head,"%c%c%s%c%s%c%d%c%s%c",RESPONSE_TYPE_SOA,PACKET_SEP_CHAR,pPacket->mark,PACKET_SEP_CHAR,pPacket->type,PACKET_SEP_CHAR,pos,PACKET_SEP_CHAR,debug,PACKET_HEAD_SEP);
            send(pSock->fd,head,strlen(head),0);
            send(pSock->fd,info,pos,0);
            FFree(info);
        }
    }
}

void guid_engineDLDestroy(UpmanData * pUpmanData){
    guid_fileDestroy();
    F_H_Iter iter = f_hashtable_begin(sGuids);
    while(f_hashtable_end(sGuids,iter) != true){
        GuidEle *pMem     = (GuidEle*)f_hashtable_def(iter);
        FFree(pMem);
        iter                    = f_hashtable_next(sGuids,iter);
    }
    f_hashtable_destroy(sGuids);
}
