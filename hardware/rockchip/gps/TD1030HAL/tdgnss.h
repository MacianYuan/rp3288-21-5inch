#ifndef __TDGNSS_H__
#define __TDGNSS_H__

#define  LOG_TAG  "TD1030DBG"

#include <cutils/log.h>

#include <hardware/gps.h>


#define  D(...)   		ALOGE(__VA_ARGS__)

#define DEFAULT_BDPRNADD 		200
#define DEFAULT_UART_BAUD_RATE 	9600
#define DEFAULT_AGPS_PORT		7275
#define DEFAULT_AGPS_SERVER		"211.151.53.216"
#define DEFAULT_GNSS_MODE		3
#define TIME_MASK_RENEW        (10*60)
#define TIME_QUE_AGPS_CHIP      (60)
#define DEFAULT_UART_DRIVER 	"/dev/ttyS4"

#define DEFAULT_PROJECT       	"TD1030"

#define  CMD_BD_COLD	 	"$CCSIR,1,1*48\r\n"
#define  CMD_GPS_COLD 		"$CCSIR,2,1*4B\r\n"
#define  CMD_BD_GP_COLD 	"$CCSIR,3,1*4A\r\n"
#define  CMD_GL_COLD 		"$CCSIR,4,1*4D\r\n"
#define  CMD_BD_GL_COLD 	"$CCSIR,5,1*4C\r\n"
#define  CMD_GP_GL_COLD 	"$CCSIR,6,1*4F\r\n"

#define  CMD_BD_HOT	 		"$CCSIR,1,3*4A\r\n"
#define  CMD_GPS_HOT 		"$CCSIR,2,3*49\r\n"
#define  CMD_BD_GP_HOT 		"$CCSIR,3,3*48\r\n"
#define  CMD_GL_HOT 		"$CCSIR,4,3*4F\r\n"
#define  CMD_BD_GL_HOT 	    "$CCSIR,5,3*4E\r\n"
#define  CMD_GP_GL_HOT 	    "$CCSIR,6,3*4D\r\n"

#define  CMD_AGPS_QUE       "$CCQUE,5*58\r\n"
#define  CMD_AGPS_ING		"$CCAING,"
#define  CMD_AGPS_QUE_LEN   		13
#define  QUE_TDVER_TIMES           	7

#define TD_SDBP_HEADER    0x40
#define TD_SDBP_ACK_FLAG1 0xc0
#define TD_SDBP_TAIL0     '\r'
#define TD_SDBP_TAIL1     '\n'
#define TD_CMD_ACK_TIME_MAX 40

#define  BAUD_9600			9600
#define  BAUD_115200        115200  
#define AGPS_DATA_CONNECTION_CLOSED   0
#define AGPS_DATA_CONNECTION_OPENING  1
#define AGPS_DATA_CONNECTION_OPEN     2
#define AGPS_GPRS_LEN 20

#define TD_ORIGINAL_MEASURE_PATH "/data/"

#define  SET_CMD_TDMODULE_LEN      15
#define TD_BDGPS_CONFIGFILE_PATH  "/system/etc/tdgnss.conf"

#define  	TD_CONFIGFILE_NUM              25
#define 	TD_OFF   						0
#define 	TD_ON                           1
#define    	TRUE		1
#define 	FALSE		0
#define 	AGPS_TIMEOUT      40

#define VCC_OFF  		0x0
#define VCC_ON			0x1
#define POWER_ON    	0x2
#define POWER_OFF    	0x3 
#define RST          	0X4    
#define RST_HIG         1 
#define RST_LOW         2
#define AGPS_ON			1
#define AGPS_OFF        0

struct boot_check_t{
	char get_ver;
	char check;
	int  boot_baud;
}boot_check;
/*
struct AP_CHIP_AGPS{
	unsigned int mask_chip_first;
	unsigned int mask_chip_second;
	int agps_time_cnt;
	char agps_get_status;
	char write_agps_first;
	char agps_flag;
	char mask_flag;
	int time_cnt;
	time_t  time_lac;
	char receive_rutxt_cnt;
};
*/
struct configfile_item
{
	char *name;
	char *value;
};

enum
{
	RECV_GNSS_OFF = 0,
	RECV_BD_GP_ON = 1,
	RECV_BD_GL_ON = 2,
	RECV_GP_GL_ON = 3,
	//RECV_GP=2,
	//RECV_BD=3,
	RECV_GP_VISIBILITY_SA = 4,
	RECV_GP_NO_VISIBILITY_SA = 5,
	RECV_GP_GSA = 6,
	RECV_BD_GSA = 7,
};
enum
{
	MODE_OFF = 0,
	MODE_GPS = 1,
	MODE_BD = 2,
	MODE_BD_GPS = 3,
	MODE_GL = 4,
	MODE_BD_GL = 5,
	MODE_GP_GL = 6,
};

enum
{
	RECV_NO = 0,
	RECV_GP = 1,
	RECV_BD = 2,
	RECV_GL = 3,
};
/* commands sent to the gps thread */
enum
{
	CMD_QUIT  = 0,
	CMD_START = 1,
	CMD_STOP  = 2,
	CMD_ENABLE_GPS,
	CMD_ENABLE_BD,
	CMD_ENABLE_GL,
	CMD_ENABLE_BD_GP,
	CMD_ENABLE_GP_GL,
	CMD_ENABLE_BD_GL,
	CMD_START_AGPS,
	CMD_AGPS,
	BD_GP_COLD,
	GP_GL_COLD,
	BD_GL_COLD,
	BD_COLD,
	GPS_COLD,
	GL_COLD,
	CMD_UPDATE_TD,
	CMD_UPDATE_RTCM,
	CMD_ORIGIN_ON,
	CMD_ORIGIN_OFF,
	CMD_SENT_TO_CHIP,
};

enum nmea_state
{
	NMEA_OFF = 0,
	NMEA_BDRMC,
	NMEA_GPRMC,
	NMEA_GNRMC,
	NMEA_GLRMC,
	NMEA_1ST_GNGSA,
	NMEA_2ND_GNGSA,
	NMEA_OTHER,
};

enum debug_mask_bit
{
  D_TOKEN = 0,
  D_GSV,
  D_GSA,
  D_NMEA,
  D_FIX,
  D_FUCTION,
  D_CONFIGFILE,
};

/* this is the state of our connection to the hardware_gpsd daemon */
typedef struct
{
	int                     init;
	int                     fd;
	GpsCallbacks            callbacks;
	GpsStatus               status;
	pthread_t               thread;
	int                     control[2];
} GpsState;

typedef struct
{
	int                     agps_mode;
	int                     nv_mode;
	int						work;
	int						agps_que_count;
	int                     agps_on;
	int                     deleaid;
	int                     on_need;
} TDMODE;

struct __updatefileinfo{
	unsigned char  	updatefiletype;
};


struct __querytdmodeinfo
{
	unsigned char  runstate;
	unsigned char  boot_ver_maj;
	unsigned char  boot_ver_min;
	unsigned char  app_ver_maj;
	unsigned char  app_ver_min;
	unsigned char  app_ver_extend;
};
struct __tdsoftinfo{
	unsigned char  boot_ver_maj;
	unsigned char  boot_ver_min;
	unsigned char  app_ver_maj;
	unsigned char  app_ver_min;
	unsigned char  app_ver_extend;
};
struct __strconfigfile{
	char projectname[100];
	char uartpath[100];
	char pmugpiopath[100];
	char agps_server[100];
	char cts[100];
};
typedef struct _TD1030Config
{
	struct __strconfigfile strconfigfile;
	struct __tdsoftinfo tdsoftinfo;
	struct __updatefileinfo updatefileinfo;
	struct __querytdmodeinfo querytdmodeinfo;
	unsigned short agps_port;
	int gnss_mode;
	int uartboadrate;
	int bdprnaddr;
	int toallprn;
	unsigned char last_gsv;
	unsigned char last_gsa;
	unsigned char power_flag;
	unsigned char position_mode;
	unsigned char gnssflag;
	unsigned char recvgpgsv;
	unsigned char recvgpgsa;
	int cts_fd;
	unsigned int debugmask;
} TD1030Config;

struct utc_t{	
	int year;	
	int month;	
	int day;	
	int hour;	
	int min;	
	int sec;
};

int TdUartOpen(void);
void SetApUartbaud(int baud);


extern TD1030Config tdconfig;
unsigned int GetDebugMaskBit(enum debug_mask_bit bit);
void Creat_update_thread(void);
void SendResultToApp(int res);


#endif
