#ifndef SERIAL_H
#define SERIAL_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      
#include <termios.h>    
#include <errno.h>     
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>


#define byte unsigned char

#define FALSE 0
#define TRUE 1

typedef struct
{
	byte type;
	byte funcode;
}ID;

int OpenCom(char* device, byte Vtime, byte Vmin);
int CloseCom(void);
void BaudRateChange(int baud);
void DiscardBuffer(void);
int TxData(byte *buffer, int length);
int RxData(byte *buffer, int length);
void Write55(void);


#endif

