#ifndef UPLOAD_INCLUDED
#define UPLOAD_INCLUDED
#include "ledProto.h"
#define FLASH_PAGE_SIZE 256

void UploadTask(void);
int  copyUploadDataToApp(void);
int  clearUploadMagic(void);
#endif

