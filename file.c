#include "file.h"
static GuidMap     sMMap;
extern int GUID_FILESIZE_MAX ;
extern int GUID_MMAP_SYSC    ;
extern int GUID_KEY_LEN_MAX  ;
int guid_fileInit(F_Hashtable * pHash,char*dataPath){
    int     fd      =   open(dataPath ,  O_RDWR | O_CREAT );
    if(!fd){
        FLOG_ERROR("GUID-FILE_INIT %s %s \n","cant open file by ",dataPath);
        return -1;
    }
    struct stat statbuf;
    if(fstat(fd,&statbuf) < 0){
        FLOG_ERROR("GUID-FILE_INIT %s %s \n","cant fstat file by ",dataPath);
        return -1;
    }
    
    sMMap.fileSize  =   statbuf.st_size;
    sMMap.fd        =   fd;
    sMMap.pData     =   (char*)mmap(0,GUID_FILESIZE_MAX,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);

    if(sMMap.pData == MAP_FAILED ){
        FLOG_ERROR("GUID-FILE_INIT %s %s \n","cant mmap file by ",dataPath);
        return -1;
    }

    sMMap.nextPos   =   statbuf.st_size;
    char*buf        =   sMMap.pData;
    int readRet     =   statbuf.st_size;
    int curPos  = 0;
    while(1){
        void* pos = memchr(buf+curPos,'\n',readRet - curPos);
        if(pos == NULL){
            /*ok we reach the end of the file*/
            break;
        }
        char *key   =   buf+curPos;
        int keyLen  =   ((char*)pos - buf) - curPos;

        //utilPrintVoid(key,keyLen);
        curPos      +=  keyLen;
        assert(buf[curPos] == '\n');
        ++curPos;
        long* id     =  (long*)(buf+curPos);
        curPos      +=  sizeof(long); 
        //FLOG_DEBUG("\n%ld \n",*id);
        if(buf[curPos] != '\n'){
            FLOG_ERROR("GUID-BADFILE %s %s \n","it is a bad file ,please repair it first by ",dataPath);
            return -1;
        }
        ++curPos;
        
        GuidEle*pMem        =   (GuidEle*)FMalloc(sizeof(GuidEle));
        pMem->key   =   key;
        pMem->keyLen=   keyLen;
        pMem->pId   =   id;
        f_hashtable_insert(pHash,pMem);
    }
    return 0;
}

static int guidMapAdd(GuidMap*pIdMap,char*key,int keyLen,long id , char** pKey, long**pId ){
    *pKey   = pIdMap->pData+pIdMap->nextPos;
    memcpy(pIdMap->pData+pIdMap->nextPos,key,keyLen);
    pIdMap->nextPos+=    keyLen;
    ((char*)(pIdMap->pData))[pIdMap->nextPos]   ='\n';
    ++pIdMap->nextPos;
    *pId    =  (long*)( pIdMap->pData+pIdMap->nextPos);
    *((long*)(pIdMap->pData+pIdMap->nextPos)) = id;
    pIdMap->nextPos+=    sizeof(long);
    ((char*)(pIdMap->pData))[pIdMap->nextPos]   ='\n';
    ++pIdMap->nextPos;
    return 0;
}

long guid_fileAddKey(F_Hashtable * pHash,char*key,int keyLen,long start){
    if(memchr(key,'\n',keyLen)||memchr(key,'\r',keyLen)){
        FLOG_ERROR("GUID-ADD invalid key  \n");
        return -1;
    }
    if(keyLen > GUID_KEY_LEN_MAX){
        FLOG_ERROR("GUID-ADD invalid keyLen by %d \n",keyLen);
        return -1;
    }
    int addSize =   keyLen+1+sizeof(long)+1;
    if(GUID_FILESIZE_MAX <= sMMap.fileSize + addSize){
        FLOG_ERROR("GUID-ADD reach guid_file_max by %d \n",GUID_FILESIZE_MAX);
        return -1;
    }
    sMMap.fileSize  +=  addSize;
    ftruncate(sMMap.fd, sMMap.fileSize);
    long id         =   start;
    GuidEle*pMem    =   (GuidEle*)FMalloc(sizeof(GuidEle));
    pMem->keyLen    =   keyLen;

    guidMapAdd(&sMMap,key,keyLen,id,&pMem->key,&pMem->pId);
    
    /*we assume it is ok except out of memery*/
    f_hashtable_insert(pHash,pMem);

    return id;
}

void guid_fileSync(){
    /*we assume it is ok*/
    msync(sMMap.pData,sMMap.fileSize,MS_ASYNC);
}

void guid_fileDestroy(UpmanData * pUpmanData){
    guid_fileSync();
    munmap(sMMap.pData , sMMap.fileSize);
    close(sMMap.fd);
}

/*
S*guid*get*0*5
memca
S*guid*get*0*8
sdseffff
S*guid*get*0*5
uyiwe
*/
