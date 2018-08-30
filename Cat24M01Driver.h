#ifndef CAT24M01DRIVER_H
#define	CAT24M01DRIVER_H
#include <xc.h>
/** D E F I N E S ***********************************************************/
#define MEMADDRL_R 0b10100001
#define MEMADDRL_W 0b10100000
#define MEMADDRH_R 0b10100011
#define MEMADDRH_W 0b10100010
#define TIMEOUT_VALUE 500

void i2c3Open(void);
int WriteDataMemory(int addr, char data);
int WriteNDataMemory(int addr, char *data,int len);
int ReadDataMemory(int addr, char *data);
int ReadNDataMemory(int addr, char *data,int len);
int isMemoryReady();

/*
exemplo de uso
int send, recv;
char data2[50];  

 while(isMemoryReady()!=1);

 send=WriteNDataMemory(103,data2,5);

while(isMemoryReady()!=1);


recv = ReadNDataMemory(103, data2, 6);

e testar os retornos por erros
*/

#endif	 CAT34M01DRIVER_H

