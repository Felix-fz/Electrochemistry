#ifndef __MYMSG_H__
#define __MYMSG_H__

#include "sys.h"


enum MessageID{    
	SetDAMsagID = 3,   //set DA
	SetDAAckMsgID = 4, //set DAAck
	GetADValueID = 5,  //get ADValue
	GetADAckMsgID = 6, //get ADAck
};

typedef struct{
	int msgID;
	int msgLen;
	int msgBody;
}setMessage;

typedef struct{
	int msgID;
	int msgLen;
	char msgBody;
}ackMessage;


int msgArray(int msgid,int msgbody,uint8_t *sendBuffer);
//u16 DecodeMSG(u8 *usartRecBuffer);
void ClearArrayData(unsigned char array[],unsigned char length);
#endif
