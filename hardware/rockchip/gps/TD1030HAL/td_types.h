#ifndef TD_TYPES_H
#define TD_TYPES_H

#include <hardware/gps.h>

#define tTD_VOID void

typedef unsigned long int       tTD_BOOL;
typedef unsigned char           tTD_UCHAR;

typedef char                    tTD_CHAR;

typedef signed   char           tTD_INT8;
typedef unsigned char           tTD_UINT8;

typedef signed   short int      tTD_INT16;
typedef unsigned short int      tTD_UINT16;

typedef signed long int         tTD_INT32;
typedef unsigned long int       tTD_UINT32;

typedef double                  tTD_DOUBLE;
typedef float                   tTD_FLOAT;

typedef void                   *tTD_HANDLE;

typedef long long tTD_INT64;
typedef unsigned long long tTD_UINT64;

#define TD_TRUE            1
#define TD_FALSE           0

#define ID_LENGTH       8

typedef enum

{
	LSM_NO_LOGGING = 0,

	LSM_SINGLE_SESSION_LOGGING = 1,

	LSM_ALL_SESSION_LOGGING = 2,

}	eLSM_LOGGING_TYPE;

typedef enum

{
	TD_INVALID_NT_TYPE,
	TD_GSM,
	TD_WCDMA
} NET_TYPE;


typedef enum

{

	TD_NO_AIDING				   = 0x00,


	TD_LOCAL_AIDING 			   = 0x01,


	TD_NETWORK_AIDING			   = 0x02,


	TD_LOCAL_AIDING_PREFERED	   = 0x03,

}  eLSM_AIDING_TYPE;


typedef struct

{
	tTD_UINT16 mcc;
	tTD_UINT16 mnc;
	tTD_UINT16 lac;
	tTD_UINT32 cid;

}  NetGsmCellID;



typedef struct

{
	tTD_UINT16 mcc;
	tTD_UINT16 mnc;
	tTD_UINT32 ucid;

} NetWcdmaCellID;


typedef struct

{
	tTD_BOOL  CellInfoFlag;
	AGpsRefLocationCellID m;

}  NetCellID;

typedef enum

{
	TD_IMSI,
	TD_MSISDN,
	TD_INVALID_SETID_TYPE
} SET_IDENTIFICATION;


typedef struct

{
	tTD_BOOL  IdSetFlag;
	SET_IDENTIFICATION IdType;
	tTD_UINT8 IdValue[ID_LENGTH];
	char setid[16];

} IdSetInfo;


#endif




