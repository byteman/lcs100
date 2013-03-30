#ifndef BY_PROTOCOL_INCLUDE
#define BY_PROTOCOL_INCLUDE

#ifdef __cplusplus
extern "C"{
#endif


typedef unsigned char (*DataProcType) (unsigned char* context, unsigned int len);

void protoParserInit(DataProcType cbProc);
///分析接收到的字符，如果满足了规定的协议则返回1，否则0
unsigned char  parseChar(unsigned char rxChar);
///读取已经接收到的最后一次的数据包
unsigned char* readPacket(unsigned int* pktLen);   
unsigned int   buildPacket(unsigned char* context, unsigned int contextSize,
						  unsigned char* packet, unsigned int pktSize);



#ifdef __cplusplus
}
#endif

#endif
