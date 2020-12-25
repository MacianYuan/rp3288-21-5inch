#ifndef SDBP_H
#define SDBP_H

#include "serial.h"

typedef struct UARTPara
{
    byte com;
    byte baudrate;
    byte databit;
    byte stopbit;
    byte checkbit;
}UARTPara;

/*
 *
 * sdbp const def
 *
 */
#define FRM_MAX_LEN 960;
#define FRM_DATA_MAX_LEN 952;

extern ID ackID;
extern ID nackID;
extern ID enterID;
extern ID confirmID;
extern ID eraseID;
extern ID writeID;
extern ID rebootID;
extern ID uartID;
extern ID verID;


byte bufWrite[960];

byte CheckImgFile(const byte *filedatas, int startpoint, int filelen, byte *type);

int GetFirstSyncFrame(byte* bufRead, int len, int* head, int* tail);
int SDBP_PUB_ACK(byte* bufRead, ID id,int idx);
int SDBP_PUB_NACK(byte* bufRead, ID id,int idx);
int SDBP_NACK(byte* bufRead, int idx);
void SDBP_UPD_ENTER(void);
void SDBP_UPD_CONFIRM_I(void);
int SDBP_UPD_CONFIRM_O(byte* bufRead, int idx);
void SDBP_UPD_ERASE_I1(void);
void SDBP_UPD_ERASE_I2(int num);
int SDBP_UPD_ERASE_O1(byte* bufRead, int idx);
int SDBP_UPD_ERASE_O2(byte* bufRead, int idx, int num);
int SDBP_UPD_WRITE_I(byte* datas,int offset,int datalen);
int SDBP_UPD_WRITE_O(byte* bufRead, int idx);
void SDBP_CFG_UART_I(UARTPara uartpara,byte autosave);
void SDBP_UPD_REBOOT();
void SDBP_DBG_FLASH_I(int offset,int num);
int SDBP_DBG_FLASH_O(byte* bufRead, int idx,byte* datas,int offset, int len);
void SDBP_QUE_VER_I(void);
int SDBP_QUE_VER_O(byte* bufRead, int idx);
int FletCher16_sdbp(byte* buffer, int offset, int count);

//DO NOT USE
//#define bool unsigned char
#define IMGFILE_HEAD_LEN    15
#define IMGFILE_HEADREAR_LEN    4
#define OG_MS_CMD_LEN 4
typedef struct
{
	unsigned char  runstate;
	unsigned char  boot_ver_maj;
	unsigned char  boot_ver_min;
	unsigned char  app_ver_maj;
	unsigned char  app_ver_min;
	unsigned char  app_ver_extend;
} VersionInfo, *pVersionInfo;

int GetVersionCmd(unsigned char *arr, int startpoint);
unsigned char GetVersionRes(unsigned char *datas, int startpoint, int len, pVersionInfo info);

unsigned short CalcCRC16(const char *datas, int startpoint, int len);
unsigned char CheckCRC16(const char *datas, int startpoint, int iLen, unsigned short crc);


#endif

