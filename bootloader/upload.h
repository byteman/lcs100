#ifndef UPLOAD_INCLUDED
#define UPLOAD_INCLUDED

typedef struct{
    unsigned int  termID;
    unsigned char groupID;
    unsigned char code;
		unsigned int  data;
}LEDCtrlProtoHead;
enum LEDCtrlCmd{
    CMD_UPLOAD_REQ=1,
    CMD_UPLOAD_DATA,
    CMD_UPLOAD_VERIFY,
    CMD_RESET
};
enum UploadError{
	ERR_OK=0,
	ERR_TOO_LARGE,
	ERR_SESSION,
	ERR_ERASE,
	ERR_WRITE_DATA,
	ERR_CRC,
	ERR_WRITE_CRC,
};
void upload_task(void);
#endif

