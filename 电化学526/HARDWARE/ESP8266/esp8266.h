#ifndef __ESP8266_H
#define __ESP8266_H	 
#include "sys.h"  

u8* esp_check_cmd(char *str);
u8* esp_check_cmd2(char *str);
u8 esp_send_cmd_IT(char *cmd,u16 waittime);
u8 esp_send_cmd(char *cmd,char *ack,u16 waittime);
int mqtt_connect(void);
int Publish_MSG(char *topic,char *arr);
void Subscribe_MSG(void);
void Subscribe_MSG_IT(void);


#endif
