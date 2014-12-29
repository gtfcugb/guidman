#ifndef _DL_FILE_
#define _DL_FILE_

#include "guid.h"

int  guid_fileInit(F_Hashtable * pHash,char*);
void guid_fileSync();
void guid_fileDestroy();
long guid_fileAddKey(F_Hashtable * pHash,char*,int,long);
#endif
