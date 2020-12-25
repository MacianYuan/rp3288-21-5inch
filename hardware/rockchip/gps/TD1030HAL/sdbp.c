#include "sdbp.h"
#include "tdgnss.h"



int BaudsArray[8] = {4800,4800,9600,19200,38400,57600,115200,230400};

byte sync_head[2] = {0x23,0x3E};

ID ackID 		= { 0x01, 0x01 };
ID nackID 		= { 0x01, 0x02 };
ID enterID 		= { 0xA0, 0x01 };
ID confirmID 	= { 0xA0, 0x02 };
ID eraseID 		= { 0xA0, 0x03 };
ID writeID 		= { 0xA0, 0x04 };
ID rebootID 	= { 0xA0, 0x05 };
ID uartID 		= { 0x03, 0x21 };
ID verID 		= { 0x05, 0x01 };

static void InputNoData(ID id)
{

	byte buf[10];
	int len;
    int fc = 0;
    buf[0] = 0x23;
    buf[1] = 0x3E;
    buf[2] = id.type;
    buf[3] = id.funcode;
    buf[4] = 0x00;
    buf[5] = 0x00;
    fc = FletCher16_sdbp(buf, 2, 4);
    buf[6] = (byte)fc;
    buf[7] = (byte)(fc >> 8);
    len = 8;
	TxData(buf, len);    
}

int FletCher16_sdbp(byte* buffer, int offset, int count)
{
    byte fc1=0,fc2=0;
    int i;
    for (i = 0; i < count; i++)
    {
        fc1 += buffer[offset+i];
        fc2 += fc1;
    }
    return (fc2 << 8) + (fc1 & 0xFF);
}
//check the buf is a correct SDBP message or not 
int GetFirstSyncFrame(byte* bufRead, int len, int* head, int* tail)
{
    int i;
    int len_f =0 ;
    int fc = 0;

    if (len <0)
        return FALSE;
    
    for (i = 0; i <= len; i++)
    {
         if(bufRead[i] == sync_head[0] && (bufRead[i+1] == sync_head[1]))
         {
            len_f = bufRead[i+4] +  (bufRead[i+5] <<8);
            fc=bufRead[i+6+len_f] +  (bufRead[i+6+len_f+1] <<8);
            if(FletCher16_sdbp((byte*)bufRead,i+2,len_f+4) == fc)                        
            {
            	*head = i;
				*tail = i+6+len_f+2;
                return TRUE;
            }
         }
    }

    return FALSE;
}

byte CheckImgFile(const byte *filedatas, int startpoint, int filelen, byte *type)
{
    (void)type;
	if(filedatas == NULL || startpoint < 0 || filelen < IMGFILE_HEADREAR_LEN)
		return FALSE;

	if(filedatas[startpoint] != 0x14 || filedatas[startpoint + 1] != 0x56 || filedatas[startpoint + 2] != 0x10  || filedatas[startpoint + 3] != 0x70)
	{	
		return FALSE;
	}
	
	
	return TRUE;
}

/*
 *
 * SDBP FUNC
 *
 */
int SDBP_PUB_ACK(byte* bufRead, ID id,int idx)
{
	if(idx <0)
		return FALSE;
	if(bufRead[idx+2] != 0x01 || bufRead[idx+3] != 0x01)
		return FALSE;
	if(bufRead[idx+6] == id.type && bufRead[idx+7] == id.funcode)
		return TRUE;

	return FALSE;
}

int SDBP_PUB_NACK(byte* bufRead, ID id,int idx)
{
	if(idx <0)
		return FALSE;
	if(bufRead[idx+2] != nackID.type || bufRead[idx+3] != nackID.funcode)
		return FALSE;
	if(bufRead[idx+6] == id.type && bufRead[idx+7] == id.funcode)
		return TRUE;
	return FALSE;
}

int SDBP_NACK(byte* bufRead, int idx)
{
	if (idx < 0)
		return FALSE;
	if (bufRead[idx + 2] == nackID.type || bufRead[idx + 3] == nackID.funcode)
		return TRUE;
	return FALSE;
}


void SDBP_UPD_ENTER(void)
{
    InputNoData(enterID);

    usleep(100000);//100 ms
}

void SDBP_UPD_CONFIRM_I(void)
{
	InputNoData(confirmID);
}


int SDBP_UPD_CONFIRM_O(byte* bufRead, int idx)
{
	if(idx <0)
		return FALSE;
	if(bufRead[idx+2] != confirmID.type || bufRead[idx+3] != confirmID.funcode)
		return FALSE;

	if(bufRead[idx+6])
		return FALSE;
	else
		return TRUE;
}

void SDBP_UPD_ERASE_I1(void)
{
	InputNoData(eraseID);
}

void SDBP_UPD_ERASE_I2(int num)
{
	int iLenToWrite;
	int fc = 0;
	bufWrite[0] = 0x23;
	bufWrite[1] = 0x3E;
	bufWrite[2] = eraseID.type;
	bufWrite[3] = eraseID.funcode;
	bufWrite[4] = 0x02;
	bufWrite[5] = 0x00;
	bufWrite[6] = (byte)num;
	bufWrite[7] =  (byte)(num>>8);
	fc = FletCher16_sdbp(bufWrite, 2,6);
	bufWrite[8] = (byte)fc;
	bufWrite[9] = (byte)(fc >> 8);
	iLenToWrite = 10;
	TxData(bufWrite, iLenToWrite);
}

int SDBP_UPD_ERASE_O1(byte* bufRead, int idx)
{
	if (idx < 0)
		return FALSE;
	if (bufRead[idx + 2] != eraseID.type || bufRead[idx + 3] != eraseID.funcode)
		return FALSE;

	if(bufRead[idx + 6])
		return FALSE;
	else
		return TRUE;
}
int SDBP_UPD_ERASE_O2(byte* bufRead, int idx, int num)
{
    (void)num;
	if (idx < 0)
		return -1;
	if (bufRead[idx + 2] != eraseID.type || bufRead[idx + 3] != eraseID.funcode)
		return -1;
	num = bufRead[idx + 7]  + (bufRead[idx + 8]<<8);
	return bufRead[idx + 6];
}

int SDBP_UPD_WRITE_I(byte* datas,int offset,int datalen)
{
	int sum = 0;
	int i;
	int iLenToWrite;
	for (i = 0; i < datalen; i++)
	{
		sum+=datas[i+offset] ;
	}
	if (sum == datalen * 0xff)
	{
		return FALSE;
	}

	int fc = 0;
	bufWrite[0] = 0x23;
	bufWrite[1] = 0x3E;
	bufWrite[2] = writeID.type ;
	bufWrite[3] = writeID.funcode;
	bufWrite[4] = (byte)(datalen+6);
	bufWrite[5] = (byte)((datalen+6) >> 8);
	bufWrite[6] = (byte)offset;
	bufWrite[7] = (byte)(offset >> 8);
	bufWrite[8] = (byte)(offset>>16);
	bufWrite[9] = (byte)(offset >>24);
	bufWrite[10] = (byte)datalen;
	bufWrite[11] = (byte)(datalen >> 8);
//	Array.Copy(datas, offset, bufWrite, 12,datalen);
	memcpy(&bufWrite[12],&datas[offset],datalen);
	fc = FletCher16_sdbp(bufWrite, 2, datalen+10);
	bufWrite[datalen + 12] = (byte)fc;
	bufWrite[datalen + 13] = (byte)(fc >> 8);
	iLenToWrite = datalen+14;

	TxData(bufWrite, iLenToWrite);
	return TRUE;
}

int SDBP_UPD_WRITE_O(byte* bufRead, int idx)
{
	if (idx < 0)
		return -1;
	if (bufRead[idx + 2] != writeID.type || bufRead[idx + 3] != writeID.funcode)
		return -1;
	if(bufRead[idx + 6])
		return FALSE;
	else
		return TRUE;
}

void SDBP_CFG_UART_I(UARTPara uartpara,byte autosave)
{
	int iLenToWrite;
	int fc = 0;
	bufWrite[0] = 0x23;
	bufWrite[1] = 0x3E;
	bufWrite[2] = 0x03;
	bufWrite[3] = 0x21;
	bufWrite[4] = 0x06;
	bufWrite[5] = 0x00;
	bufWrite[6] = uartpara.com;
	bufWrite[7] = uartpara.baudrate;
	bufWrite[8] = uartpara.databit;
	bufWrite[9] = uartpara.stopbit;
	bufWrite[10] = uartpara.checkbit;
	bufWrite[11] = autosave;
	fc = FletCher16_sdbp(bufWrite, 2, 10);
	bufWrite[12] = (byte)fc;
	bufWrite[13] = (byte)(fc >> 8);
	iLenToWrite = 14;
	TxData(bufWrite, iLenToWrite);
}

void SDBP_UPD_REBOOT()
{
	InputNoData(rebootID);
}

void SDBP_DBG_FLASH_I(int offset,int num)
{
	int iLenToWrite;
	int fc = 0;
	bufWrite[0] = 0x23;
	bufWrite[1] = 0x3E;
	bufWrite[2] = 0xBE;
	bufWrite[3] = 0x08;
	bufWrite[4] = 0x08;
	bufWrite[5] = 0x00;
	bufWrite[6] =0x47;
	bufWrite[7] =0x44;
	bufWrite[8] = (byte)(offset);
	bufWrite[9] = (byte)(offset>>8);
	bufWrite[10] = (byte)(offset>>16);
	bufWrite[11] = (byte)(offset >> 24);
	bufWrite[12] = (byte)(num);
	bufWrite[13] = (byte)(num>>8);
	fc = FletCher16_sdbp(bufWrite, 2, 12);
	bufWrite[14] = (byte)fc;
	bufWrite[15] = (byte)(fc >> 8);
	iLenToWrite = 16;
	TxData(bufWrite, iLenToWrite);
}


int SDBP_DBG_FLASH_O(byte* bufRead, int idx,byte* datas,int offset, int len)
{
	if (idx < 0)
		return -1;
	int offset1 = bufRead[idx + 6] + (bufRead[idx + 7] << 8) +
						  (bufRead[idx + 8] << 16) + (bufRead[idx + 9] << 24);
	int len1 = bufRead[idx +10] + (bufRead[idx + 11] << 8);
	if (offset1 != offset || len != len1)
		return -1;
//	Array.Copy(bufRead, idx + 12, datas, offset, len);
	memcpy(&datas[offset],&bufRead[idx + 12],len);
	return len;
}

void SDBP_QUE_VER_I(void)
{
    InputNoData(verID);
}

int SDBP_QUE_VER_O(byte* bufRead, int idx)
{
	int len;

    if (idx < 0)
		return FALSE;
    if (bufRead[idx + 2] != verID.type || bufRead[idx + 3] != verID.funcode)
		return FALSE;
    len = bufRead[idx +4]  + (bufRead[idx +5] <<8);
    if(len>150)
		return FALSE;

    return TRUE;

}

//*****************************************
//DO NOT USE
//
//
//
//*****************************************
unsigned char CRC16TabH[256] =
{
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

unsigned char CRC16TabL[256] =
{
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2,
	0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04,
	0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
	0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8,
	0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
	0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
	0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6,
	0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10,
	0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
	0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
	0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE,
	0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
	0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA,
	0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C,
	0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0,
	0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62,
	0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
	0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE,
	0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
	0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
	0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C,
	0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76,
	0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
	0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
	0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,
	0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
	0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,
	0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A,
	0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86,
	0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

unsigned short CalcCRC16(const char *datas, int startpoint, int len)
{
	unsigned short crc = 0;
	unsigned char crc_h = 0;
	unsigned char crc_l = 0;
	unsigned char tmp;
	int len_all = len + startpoint;
	int i;

	if(startpoint < 0 )
		return 0;
	

	for (i = startpoint; i < len_all; i++)
	{
		tmp = crc_h ^ datas[i];
		crc_h = crc_l ^ CRC16TabH[tmp];
		crc_l = CRC16TabL[tmp];
	}

	crc = (((unsigned short) crc_h) << 8) + (unsigned short)crc_l;

	return crc;
}

unsigned char CheckCRC16(const char *datas, int startpoint, int iLen, unsigned short crc)
{
	unsigned short crc_calc = 0;

	if(startpoint < 0 )
		return FALSE;

	crc_calc = CalcCRC16(datas, startpoint, iLen);

	if(crc_calc == crc )
	{
		return TRUE;
	}

	return FALSE;
}
int GetVersionCmd(unsigned char *arr, int startpoint)
{
    (void)arr;
	(void)startpoint;
	return 0;
}

unsigned char GetVersionRes(unsigned char *datas, int startpoint, int len, pVersionInfo info)
{	
    (void)datas;
	(void)startpoint;
	(void)len;
	(void)info;
	return FALSE;
}

