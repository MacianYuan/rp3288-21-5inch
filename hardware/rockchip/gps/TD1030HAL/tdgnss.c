#include <errno.h>
#include <pthread.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <math.h>
#include <time.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <cutils/sockets.h>
#include <hardware/gps.h>
#include <cutils/properties.h>
#include <dirent.h>
#include <sys/stat.h>
#include "tdgnss.h"
#include "td_types.h"
#include "sdbp.h"
#include "update.h"
#include "agps.h"

time_t start,end,end_agps;
int time_agps=0;
//int poweroff_flag=0;
static struct configfile_item item_list[TD_CONFIGFILE_NUM];

//struct AP_CHIP_AGPS ap_chip_agps;
unsigned char chip_standby[9]={0x23,0x3E,0x02,0x04,0x01,0x00,0x01,0x08,0x1E};
unsigned char      chip_rst[9]={0x23,0x3E,0x02,0x01,0x01,0x00,0x01,0x05,0x12};
unsigned char      fix_dop100[15]={0x23,0x3E,0x03,0x34,0x07,0x00,0x64,0x64,0x10,0x27,0x10,0x27,0x01,0x75,0xE7};
unsigned char      gnssmode_bds[15]={0x24,0x43,0x43,0x53,0x49,0x52,0x2C,0x31,0x2C,0x30,0x2A,0x34,0x39,0x0D,0x0A};
unsigned char      gnssmode_gps[15]={0x24,0x43,0x43,0x53,0x49,0x52,0x2C,0x32,0x2C,0x30,0x2A,0x34,0x41,0x0D,0x0A};
unsigned char      gnssmode_gps_bds[15]={0x24,0x43,0x43,0x53,0x49,0x52,0x2C,0x33,0x2C,0x30,0x2A,0x34,0x42,0x0D,0x0A};
unsigned char      gnssmode_gl[15]={0x24,0x43,0x43,0x53,0x49,0x52,0x2C,0x34,0x2C,0x30,0x2A,0x34,0x43,0x0D,0x0A};
unsigned char      gnssmode_bds_gl[15]={0x24,0x43,0x43,0x53,0x49,0x52,0x2C,0x35,0x2C,0x30,0x2A,0x34,0x44,0x0D,0x0A};
unsigned char      gnssmode_gps_gl[15]={0x24,0x43,0x43,0x53,0x49,0x52,0x2C,0x36,0x2C,0x30,0x2A,0x34,0x45,0x0D,0x0A};
unsigned char cold_start[9]={0x23,0x3E,0x02,0x01,0x01,0x00,0x01,0x05,0x12};


bool fixed[256];

enum nmea_state nmeastate;
extern pthread_mutex_t AgpsDataMutex;
extern TD_INFO td_agps_info;

//static pthread_mutex_t PowerMutex   	  = PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t StatusEventMutex   = PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t SessionMutex       = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t RilMutex           = PTHREAD_COND_INITIALIZER;
pthread_mutex_t UartWriteMutex     = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t TDMODEMutex     	  = PTHREAD_COND_INITIALIZER;
//static pthread_mutex_t TDMODEWROKINGMutex    	  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t  QueagpsMutex    = PTHREAD_MUTEX_INITIALIZER;

#define AUTO_UPDATE_STATE_IDLE			0
#define AUTO_UPDATE_STATE_CHECK_VERSION	1

struct auto_update_version_check{
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	VersionInfo ver;
	int state;
};
pthread_mutex_t version_check_struct_lock = PTHREAD_MUTEX_INITIALIZER;
struct auto_update_version_check version_check;

//end

#define MAX_RTCM_DATA_LEN 2048
static char rtcm_data[MAX_RTCM_DATA_LEN] = {0};
static int rtcm_data_len = 0;

//#define PRINT_RTCM_DATA
//#define PRINT_ORIGIN_DATA
//#define PRINT_LONG_DATA
#ifdef PRINT_LONG_DATA
static char rtcm_tmp[2048] = {0};
#define MAX_PRINTF 300
#endif

int origin_flag = 0;
#define OG_MS_FRAME_HEAD_LEN 6
#define OG_MS_FRAME_TAIL_LEN 2
#define OG_MS_VALID_DATA_LEN 48  //512
#define OG_MS_FRAME_LEN (OG_MS_FRAME_HEAD_LEN + OG_MS_VALID_DATA_LEN + OG_MS_FRAME_TAIL_LEN)
#define OG_MS_FRAME_TAIL_H 0xCA //'\r'
#define OG_MS_FRAME_TAIL_L 0xAC //'\n'
char origin_data[OG_MS_FRAME_LEN + 1];


#ifdef WRITE_ORIGIN_SDCARD
int glb_og_fd = -1;
#endif
static char origin_data_tmp[2048] = {'$', 'G', 'P', 'B', 'I', 'N',0};

#define CMD_MAX_LEN 4096
static char cmd_buff[CMD_MAX_LEN] = {0};
static int cmd_length = 0;
static volatile int cmd_to_chip_flag = 0;

static char ack_buff[CMD_MAX_LEN] = {0};
static pthread_t tdchip_ack_timer_thread_id;

struct tdchip_ack_timer_sync{
	pthread_mutex_t mutex;
	pthread_cond_t cond;
};
struct tdchip_ack_timer_sync tdchip_ack_sync = {
	.mutex = PTHREAD_COND_INITIALIZER,
	.cond = PTHREAD_MUTEX_INITIALIZER
};

//static int change_baud_9600_flag = 0;
//end

TD1030Config tdconfig;

static int DataConnectionStatus = AGPS_DATA_CONNECTION_CLOSED;
static unsigned RilRequestPending = 0;
char gprs_interface[AGPS_GPRS_LEN];

static IdSetInfo  TdIdInfo;
static NetCellID   TdCellID;
static tTD_BOOL  NetAccess;
TDMODE td_mode = {0,0,0,0,0,0,0};
typedef struct
{
	const char  *p;
	const char  *end;
} Token;

#define  MAX_NMEA_TOKENS  32

typedef struct
{
	int     count;
	Token   tokens[ MAX_NMEA_TOKENS ];
} NmeaTokenizer;

static char *GetItemValue(char *name);
static char *remove_cr_lf(char *buf, char ch);
static int set_itemvalue(char *name, char *value);
static void intercept(char *line);
static int load_properties(const char *path);
static void LoadProjectName(void);
static void LoadDriverPath(void);
static void LoadUartBaudRate(void);
static void LoadBeiDouPrnAdd(void);
static void FreeItemList(void);
static int LoadBdGpsConfigFile(void);
static void StartAgps(void);

void update_nmea(const char  *p, const char  *end );
void PowerOn(void);
void PowerOff(void);
int UartTxData(unsigned char *buffer, int length);
//void VccOn(void);
//void VccOff(void);
//static void agps_deal(void);
//static void rutxt_deal( NmeaTokenizer *tzer);
//static void cmp_renew_agps(unsigned int *mask1,unsigned int *mask2);
//static void cmp_mk_wrdata(unsigned int *mask);
//static bool get_mask(NmeaTokenizer  *tzer,unsigned int *mask);
//static void write_ccaing(int fd,unsigned int mask);
unsigned int CrcSum(char *s);

void update_gps_location(GpsLocation *location);
void update_gps_status(GpsStatusValue value);
void update_bdgps_svstatus(GpsSvStatus *svstatus);
static  int _LibNMEA_Xor(char *p_chk_str);
static int td_set_update_mode(void);
void RstTdChip(void);
//static void judge_cmd_change_baud(void);
void update_origin_data(const char * data, const int data_len);

int update_mode = 0;


struct __GSAPRN
{
	int   fixprnnum;
	int   fixprn[GPS_MAX_SVS];
//	unsigned  char fixprnflag;
} GsaPrn;
struct supl_agps_data td_agps_data;

GpsState  _gps_state[1];


static char *GetItemValue(char *name)
{
	int i;

	for (i = 0; i < TD_CONFIGFILE_NUM; i++)
	{
		if (!item_list[i].name)
		{
			continue;
		}

		if (strcmp(item_list[i].name, name) == 0)
		{
			return item_list[i].value;
		}
	}

	return NULL;
}
static char *remove_cr_lf(char *buf, char ch)
{
	int len = strlen(buf);
	int i;

	if (len == 0)
	{
		return NULL;
	}

	for (i = len - 1; i >= 0; i--)
	{
		if (buf[i] == ch)
		{
			return &buf[i];
		}
	}

	return NULL;
}

static int set_itemvalue(char *name, char *value)
{
	int i;

	for (i = 0; i < TD_CONFIGFILE_NUM; i++)
	{
		char *n, *v;

		if (item_list[i].name && strcmp(item_list[i].name, name))
			continue;

		if (item_list[i].name == NULL)
		{
			n = malloc(strlen(name) + 1);

			if (!n)
			{
				return -1;
			}

			memset(n, 0x00, strlen(name) + 1);

			v = malloc(strlen(value) + 1);

			if (!v)
			{
				free(n);
				return -1;
			}

			memset(v, 0x00, strlen(value) + 1);

			strcpy(n, name);
		}
		else
		{
			n = item_list[i].name;
			v = item_list[i].value;

			if (strlen(v) < strlen(value))
			{
				v = malloc(strlen(value) + 1);

				if (!v)
				{
					return -1;
				}

				memset(v, 0x00, strlen(value) + 1);

				free(item_list[i].value);
			}
		}

		strcpy(v, value);

		item_list[i].name = n;
		item_list[i].value = v;

		return 0;
	}

	return -1;
}

static void intercept(char *line)
{
	char *buf = line;
	char *name, *value;

	if (line[0] == '#')
		return;

	name = strsep(&buf, "=");
	value = strsep(&buf, "=");

	set_itemvalue(name, value);

	return ;
}

#define PROPERTY_FILE_MAX_SIZE 2048

static int add_properties(char *buff, char *tag, char *value)
{
	char line[256];
	int tag_len, value_len;
	tag_len = strlen(tag);
	value_len = strlen(value);
	if((tag_len + value_len) > 250)
		return -1;
	sprintf(line, "%s=%s\r\n", tag, value);

	if((strlen(line) + strlen(buff)) > PROPERTY_FILE_MAX_SIZE)
		return -1;
	strcat(buff, line);
	return 0;
}

static int add_properties_int(char *buff, char *tag, int value)
{
	char line[256];
	int tag_len;
	tag_len = strlen(tag);
	if((tag_len) > 200)
		return -1;
	
	sprintf(line, "%s=%d\r\n", tag, value);

	if((strlen(line) + strlen(buff)) > PROPERTY_FILE_MAX_SIZE)
		return -1;
	strcat(buff, line);
	return 0;
}

static int add_properties_blank_pad(char *buff)
{
	char line[256];
	memset(line, ' ', 256);
	line[255] = 0;	
	strcat(buff, line);	
	return 0;
}

static int save_properties()
{
	char *p_buff;
	const char *path = TD_BDGPS_CONFIGFILE_PATH;
	FILE *fp;
	int file_len;

	D("%s", __FUNCTION__);
	p_buff = (char*)calloc(PROPERTY_FILE_MAX_SIZE, 1);
	if(!p_buff)
		return -1;

	add_properties(p_buff, "PROJECT", tdconfig.strconfigfile.projectname);
	add_properties(p_buff, "UART_DRIVER", tdconfig.strconfigfile.uartpath);
	add_properties_int(p_buff, "UART_BAUD_RATE", tdconfig.uartboadrate);
	add_properties_int(p_buff, "BDPRNADD", tdconfig.bdprnaddr);
	add_properties(p_buff, "AGPS_SERVER", tdconfig.strconfigfile.agps_server);
	add_properties_int(p_buff, "AGPS_PORT", tdconfig.agps_port);
	add_properties_int(p_buff, "GNSS_MODE", tdconfig.gnss_mode);

    add_properties_int(p_buff, "TD_DEBUG_TOKEN", 0);
	add_properties_int(p_buff, "TD_DEBUG_GSV", 0);
	add_properties_int(p_buff, "TD_DEBUG_GSA", 0);
	add_properties_int(p_buff, "TD_DEBUG_NMEA", 0);
	add_properties_int(p_buff, "TD_DEBUG_FIX", 0);
	add_properties_int(p_buff, "TD_DEBUG_FUCTION", 1);
	add_properties_int(p_buff, "TD_DEBUG_CONFIGFILE", 1);
	
	add_properties_blank_pad(p_buff);

	fp = fopen(path, "w");

	if(fp != NULL)
		if (GetDebugMaskBit(D_CONFIGFILE) == 1)
		{
		D("%s: fopen ok\n", __FUNCTION__);
		}

	if (!fp)
	{
		if (GetDebugMaskBit(D_CONFIGFILE) == 1)
		{

		D("%s: can not to open %s (%d)\n", __FUNCTION__, path, errno);
		}
		free(p_buff);
		return -1;
	}
	file_len = strlen(p_buff);
	fseek(fp, 0, SEEK_SET);
	if(fwrite(p_buff, 1, (size_t)file_len, fp) != (size_t)file_len)
	{
		D("write property file err");
	}
	
	fclose(fp);
	
	free(p_buff);

	D("write property ok, len:%d", file_len);

	return 0;
}

static int load_default_properties()
{
	D("%s", __FUNCTION__);
	//memset(tdconfig.projectname,0,sizeof(tdconfig.projectname));
	strcat(tdconfig.strconfigfile.projectname, DEFAULT_PROJECT);
	D("%s",tdconfig.strconfigfile.projectname);
	strcat(tdconfig.strconfigfile.uartpath, DEFAULT_UART_DRIVER);
	D("%s",tdconfig.strconfigfile.uartpath);
	strcat(tdconfig.strconfigfile.agps_server, DEFAULT_AGPS_SERVER);
	//sprintf(tdconfig.strconfigfile.bdprnaddr,"%d",DEFAULT_BDPRNADD);
	tdconfig.bdprnaddr = DEFAULT_BDPRNADD;
	tdconfig.uartboadrate = DEFAULT_UART_BAUD_RATE;
	tdconfig.agps_port = DEFAULT_AGPS_PORT;
	tdconfig.gnss_mode = DEFAULT_GNSS_MODE;
	tdconfig.debugmask = 0x60;
	memset(&tdconfig.tdsoftinfo,0,sizeof(tdconfig.tdsoftinfo));
	return 0;
}



static int load_properties(const char *path)
{
	char line[256];
	FILE *fp;
	int i = 0;

	fp = fopen(path, "r");

	if(fp != NULL)
		if (GetDebugMaskBit(D_CONFIGFILE) == 1)
		{
		D("%s: fopen ok\n", __FUNCTION__);
		}

	if (!fp)
	{
		if (GetDebugMaskBit(D_CONFIGFILE) == 1)
		{

		D("%s: can not to open %s (%d)\n", __FUNCTION__, path, errno);
		}
		return -1;
	}

	while (fgets(line, sizeof(line), fp))
	{
		i++;
		char *nl;
		if((line[0] == 0)||(line[0]==0xa)||(line[0]==' '))
			break;
		//D("fgets1:%s--len:%d \n",line,strlen(line));
		while ((nl = remove_cr_lf(line, '\n')) || (nl = remove_cr_lf(line, '\r')) )
		{
			*nl = '\0';
		}

		if (GetDebugMaskBit(D_CONFIGFILE) == 1)
		{
		D("%s: item open %s  \n", __FUNCTION__, line);
        }
		//D("fgets2:%s --len:%d\n",line,strlen(line));
		intercept(line);
	}

	fclose(fp);

	return 0;
}
static void LoadProjectName(void)
{
	char *projectname = GetItemValue("PROJECT");

	if (projectname)
	{
		strcpy(tdconfig.strconfigfile.projectname, projectname);
		if (GetDebugMaskBit(D_CONFIGFILE) == 1)
		{
		D("[PROJECT_NAME : %s]", tdconfig.strconfigfile.projectname);
		}
	}
	else
	{
		if (GetDebugMaskBit(D_CONFIGFILE) == 1)
		{

		D("[PROJECT_NAME : no]");
		}
	}

	return;
}
static void LoadDriverPath(void)
{
	char *uartpath = GetItemValue("UART_DRIVER");

	if (!uartpath)
	{
		D("Uart Path is NULL!");
	}else{
		memset(tdconfig.strconfigfile.uartpath,0,sizeof(tdconfig.strconfigfile.uartpath));
		strcpy(tdconfig.strconfigfile.uartpath, uartpath);
		D("Uart Path : %s", tdconfig.strconfigfile.uartpath);
	}
	
	return;
}
/*
static void LoadSoftVerInfo(void)
{
	char *boot_ver_major = GetItemValue("TDBOOT_VER_MAJOR");
	char *boot_ver_minor = GetItemValue("TDBOOT_VER_MINOR");
	char *app_ver_major = GetItemValue("TDAPP_VER_MAJOR");
	char *app_ver_minor = GetItemValue("TDAPP_VER_MINOR");
	char *app_ver_extend = GetItemValue("TDAPP_VER_EXTEND");

	//memset(tdconfig.updatetdpath,0,sizeof(tdconfig.updatetdpath));
	if (!boot_ver_major)
	{
		//D("boot_ver_major is NULL!");
	}else{
		tdconfig.tdsoftinfo.boot_ver_maj=atoi(boot_ver_major);
		//D("boot_ver_major : %d",tdconfig.tdsoftinfo.boot_ver_maj);
	}
	if (!boot_ver_minor)
	{
		//D("boot_ver_minor is NULL!");
	}else{
		tdconfig.tdsoftinfo.boot_ver_min=atoi(boot_ver_minor);
		//D("boot_ver_minor : %d",tdconfig.tdsoftinfo.boot_ver_min);
	}
	if (!app_ver_major)
	{
		//D("app_ver_major is NULL!");
	}else{
		tdconfig.tdsoftinfo.app_ver_maj=atoi(app_ver_major);
		//D("app_ver_major : %d", tdconfig.tdsoftinfo.app_ver_maj);
	}
	if (!app_ver_minor)
	{
		//D("app_ver_minor is NULL!");
	}else{
		tdconfig.tdsoftinfo.app_ver_min = atoi(app_ver_minor);
		//D("tdconfig.tdsoftinfo.app_ver_maj : %d",tdconfig.tdsoftinfo.app_ver_min);
	}
	if (!app_ver_extend)
	{
		//D("app_ver_extend is NULL!");
	}else{
		tdconfig.tdsoftinfo.app_ver_extend = atoi(app_ver_extend);
		//D("app_ver_extend : %d", tdconfig.tdsoftinfo.app_ver_extend);
	}	
	return;
}
*/

static void LoadUartBaudRate(void)
{
	char *baudrate = GetItemValue("UART_BAUD_RATE");
	int selectbaudrate = 0;

	if (baudrate)
	{
		selectbaudrate = atoi(baudrate);
	}
	else
	{
		selectbaudrate = DEFAULT_UART_BAUD_RATE;
	}


	if((selectbaudrate != 9600) && (selectbaudrate != 115200) && (selectbaudrate != 38400)
			&& (selectbaudrate != 57600))
	{

		selectbaudrate = DEFAULT_UART_BAUD_RATE;
	}

	tdconfig.uartboadrate = selectbaudrate;
	if (GetDebugMaskBit(D_CONFIGFILE) == 1)
	{
	D("Uart Baudrate : %d", selectbaudrate);
	}


	return;
}
static void LoadBeiDouPrnAdd(void)
{
	char *beidouprn = GetItemValue("BDPRNADD");
	int bdprnaddr = 0;

	if (beidouprn)
	{
		bdprnaddr = atoi(beidouprn);
	}
	else
	{
		bdprnaddr = DEFAULT_BDPRNADD;
	}

	if(bdprnaddr <= 0)
		bdprnaddr = DEFAULT_BDPRNADD;

	tdconfig.bdprnaddr = bdprnaddr;

	return;
}
static void LoadAgpsPort(void)
{
	char *p_ch = GetItemValue("AGPS_PORT");
	int port = 0;

	if (p_ch)
	{
		port = atoi(p_ch);
	}
	else
	{
		port = DEFAULT_AGPS_PORT;
	}

	tdconfig.agps_port= port;
	D("AGPS_PORT:%d",tdconfig.agps_port);
	return;
}

static void LoadAgpsServer(void)
{
	char *p_ch = GetItemValue("AGPS_SERVER");
	memset(tdconfig.strconfigfile.agps_server,0,sizeof(tdconfig.strconfigfile.agps_server));
	if (p_ch)
	{
		strcpy(tdconfig.strconfigfile.agps_server, p_ch);
		if (GetDebugMaskBit(D_CONFIGFILE) == 1)
		{
		D("[AGPS_SERVER : %s]", tdconfig.strconfigfile.agps_server);
		}
	}
	else
	{
		if (GetDebugMaskBit(D_CONFIGFILE) == 1)
		{

		D("[AGPS_SERVER : no]");
		}
		strcpy(tdconfig.strconfigfile.agps_server, DEFAULT_AGPS_SERVER);
	}

	return;
}
static void LoadGnssMode(void)
{
	char *p_ch = GetItemValue("GNSS_MODE");
	int mode = 3;

	if (p_ch)
	{
		mode = atoi(p_ch);
	}
	else
	{
		mode = DEFAULT_GNSS_MODE;
	}
	if((mode < 1) || (mode > 6))
		mode = DEFAULT_GNSS_MODE;

	tdconfig.gnss_mode= mode;
	D("GNSS_MODE:%d",tdconfig.gnss_mode);
	return;
}
static void LoadDebugSetting(void)
{
	char *p_ch;
	unsigned int mask = 0;
    p_ch = GetItemValue("TD_DEBUG_TOKEN");

	if (p_ch)
	{
		mask |= atoi(p_ch)<<D_TOKEN;
	}
	else
	{
		mask |= 0<<D_TOKEN;
	}

    p_ch = GetItemValue("TD_DEBUG_GSV");

	if (p_ch)
	{
		mask |= atoi(p_ch)<<D_GSV;
	}
	else
	{
		mask |= 0<<D_GSV;
	}
	
    p_ch = GetItemValue("TD_DEBUG_GSA");

	if (p_ch)
	{
		mask |= atoi(p_ch)<<D_GSA;
	}
	else
	{
		mask |= 0<<D_GSA;
	}

    p_ch = GetItemValue("TD_DEBUG_NMEA");

	if (p_ch)
	{
		mask |= atoi(p_ch)<<D_NMEA;
	}
	else
	{
		mask |= 0<<D_NMEA;
	}	
    p_ch = GetItemValue("TD_DEBUG_FIX");

	if (p_ch)
	{
		mask |= atoi(p_ch)<<D_FIX;
	}
	else
	{
		mask |= 0<<D_FIX;
	}
    p_ch = GetItemValue("TD_DEBUG_FUCTION");

	if (p_ch)
	{
		mask |= atoi(p_ch)<<D_FUCTION;
	}
	else
	{
		mask |= 0<<D_FUCTION;
	}

    p_ch = GetItemValue("TD_DEBUG_CONFIGFILE");

	if (p_ch)
	{
		mask |= atoi(p_ch)<<D_CONFIGFILE;
	}
	else
	{
		mask |= 0<<D_CONFIGFILE;
	}
	
	tdconfig.debugmask= mask;
	
	if (GetDebugMaskBit(D_CONFIGFILE) == 1)
	{
	D("[DebugMask:0x%x]",tdconfig.debugmask);
	}

	return;

}

unsigned int GetDebugMaskBit(enum debug_mask_bit bit)
{
  return (tdconfig.debugmask>>bit)&0x01;
}

static void FreeItemList(void)
{
	int i;

	for (i = 0; i < TD_CONFIGFILE_NUM; i++)
	{
		if (!item_list[i].name)
		{
			continue;
		}

		free(item_list[i].name);
		free(item_list[i].value);

		item_list[i].name = NULL;
		item_list[i].value = NULL;
	}

	return ;
}

static int LoadBdGpsConfigFile(void)
{
	char *path = TD_BDGPS_CONFIGFILE_PATH;
	int n;

	n = load_properties(path);

	if (n < 0)
	{
		if (GetDebugMaskBit(D_CONFIGFILE) == 1)
		{
		D("%s:Load config error", __FUNCTION__);
		D("Create a tdgnss.conf!!!");
		}
		load_default_properties();
		save_properties();
		return -1;
	}
	LoadDebugSetting();
	LoadProjectName();
	LoadDriverPath();
	LoadUartBaudRate();
	LoadBeiDouPrnAdd();
	LoadAgpsPort();
	LoadAgpsServer();
	LoadGnssMode();

	FreeItemList();
	return 0;
}

int TdUartOpen(void)
{
	int fd;
	fd = open(tdconfig.strconfigfile.uartpath,  O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);	//O_RDONLY

	if (fd < 0)
	{
		perror("SERIAL ERROR : no gps_bd serial detected ");

		D("ERROR : Uart Open %s", strerror(errno));
		
		return (-1);
	}


	D("Succeed : open uart %s boadrate = %d\n", tdconfig.strconfigfile.uartpath, tdconfig.uartboadrate);

	if ( isatty( fd ) )
	{
		struct termios  ios;
		tcgetattr( fd, &ios );
		ios.c_lflag = 0;  /* disable ECHO, ICANON, etc... */
		ios.c_oflag = 0;
		ios.c_iflag = 0;

		ios.c_cflag &= ~CSTOPB;	//stop bit 1
		ios.c_cflag |= CS8;		//data bit 8
		ios.c_cflag &= ~PARENB;	//parity non
		ios.c_cflag &= ~CRTSCTS;        //no flow control, need update kernel


		switch(tdconfig.uartboadrate)
		{
			case 9600:
				cfsetispeed(&ios, B9600);
				cfsetospeed(&ios, B9600);
				break;

			case 115200:
				cfsetispeed(&ios, B115200);
				cfsetospeed(&ios, B115200);
				break;

			case 38400:
				cfsetispeed(&ios, B38400);
				cfsetospeed(&ios, B38400);
				break;

			case 57600:
				cfsetispeed(&ios, B57600);
				cfsetospeed(&ios, B57600);
				break;

			default:
				cfsetispeed(&ios, B9600);
				cfsetospeed(&ios, B9600);
				break;
		}

		//cfsetispeed(&ios, B115200);
		//cfsetospeed(&ios, B115200);
		tcsetattr( fd, TCSANOW, &ios );
	}

	return fd;
}

void SetApUartbaud(int baud)
{

	D("AP baud=%d\n",baud);
	tcflush(_gps_state->fd,TCIOFLUSH);
	struct termios options;
		options.c_lflag = 0;  /* disable ECHO, ICANON, etc... */
		options.c_oflag = 0;
		options.c_iflag = 0;


		options.c_cflag &= ~CSTOPB;	//stop bit 1
		options.c_cflag |= CS8;		//data bit 8
		options.c_cflag &= ~PARENB;	//parity non
		options.c_cflag &= ~CRTSCTS;  
	if(baud==9600){
		cfsetispeed(&options, B9600);
		cfsetospeed(&options, B9600);
	}
	else if(baud==115200)
	{
		cfsetispeed(&options, B115200);
		cfsetospeed(&options, B115200);

	}	
	//options.c_cflag |= (CLOCAL | CREAD);
	
	tcsetattr(_gps_state->fd, TCSANOW, &options);

	
}

/********** AGPS*******************/
AGpsCallbacks      agpsCallbacks;
GpsNiCallbacks     gpsNiCallbacks;
AGpsRilCallbacks   gpsRilCallbacks;

/* AGPSInterface!!!!!!!!!!!!!!!!! */
static void  td_agps_init( AGpsCallbacks *callbacks )
{
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);

	if(callbacks != NULL)
	{
		agpsCallbacks.status_cb = callbacks->status_cb;
	}
}

static int  td_agps_data_conn_open( const char *apn )
{
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);

	pthread_mutex_lock(&RilMutex);
	DataConnectionStatus = AGPS_DATA_CONNECTION_OPEN;
	pthread_mutex_unlock(&RilMutex);
	D("con open recvd with apn = %s", apn);

	if(RilRequestPending)
	{
		RilRequestPending = 0;
	}
	else
	{
	}

	return 0;
}

static int  td_agps_data_conn_closed(const int connId)
{
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s(%d)", __FUNCTION__,connId);

	pthread_mutex_lock(&RilMutex);
	DataConnectionStatus = AGPS_DATA_CONNECTION_CLOSED;
	pthread_mutex_unlock(&RilMutex);
	D("con_closed from ril");

	return 0;
}

static int  td_agps_data_conn_failed(void)
{
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);

	pthread_mutex_lock(&RilMutex);
	DataConnectionStatus = AGPS_DATA_CONNECTION_CLOSED;
	pthread_mutex_unlock(&RilMutex);
	D("Data conn failed");

	if(RilRequestPending)
	{
		RilRequestPending = 0;

	}

	return 0;
}

static int  td_agps_set_server(AGpsType type, const char *hostname, int port)
{
	if (0)D("Enter : %s(%d,%s,%d)", __FUNCTION__,type,hostname,port);
	//D("td_agps_set_server - Type : %d, hostname = %s, port = %d", type, hostname, port);

	if(hostname == NULL)
	{

		return -1;
	}

	return 0;
}
static int td_data_conn_open_with_apn_ip_type(const char* apn,ApnIpType apnIpType)
{
  
  if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s(%s,%d)", __FUNCTION__,apn,apnIpType);
  return 0;
}

static const AGpsInterface tdAGpsInterface =
{
	sizeof(AGpsInterface),
	td_agps_init,
	td_agps_data_conn_open,
	td_agps_data_conn_closed,
	td_agps_data_conn_failed,
	td_agps_set_server,
	td_data_conn_open_with_apn_ip_type,
};
/* GpsNiInterface!!!!!!!!!!!!!!!!!! */
static void td_gps_ni_init (GpsNiCallbacks *callbacks)
{
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);

	if(callbacks != NULL)
	{
		gpsNiCallbacks.notify_cb = callbacks->notify_cb;
	}

	return;
}


static void td_gps_ni_respond (int notif_id, GpsUserResponseType user_response)
{
	//tTD_RESULT result;

	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s(%d,%d)", __FUNCTION__,notif_id,user_response);

	return;
}

static const GpsNiInterface tdGpsNiInterface =
{
	sizeof(GpsNiInterface),
	td_gps_ni_init,
	td_gps_ni_respond
};
/*agpsrilinterface*/

static void td_agps_ril_init( AGpsRilCallbacks *callbacks )
{
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);

	if(callbacks == NULL)
	{
		D("ERROR : AGpsRilCallbacks NULL");
		return;
	}

	gpsRilCallbacks.request_setid = callbacks->request_setid;
	gpsRilCallbacks.request_refloc = callbacks->request_refloc;

	return;
}


static void td_agps_ril_set_ref_location (const AGpsRefLocation *agps_reflocation,size_t sz_struct)
{
	int type, mcc, mnc, lac, cid = 0;
	AGpsRefLocation *pRefLoc = NULL;
	//tTD_BOOL result = TD_SUCCESS;

	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s(%d)", __FUNCTION__,sz_struct);

	pRefLoc = (AGpsRefLocation *) agps_reflocation;

	if(pRefLoc == NULL)
	{
		D("ERROR : RefLocation NULL !!!!");
		return;
	}

	type = pRefLoc->type;

	mcc = pRefLoc->u.cellID.mcc;
	mnc = pRefLoc->u.cellID.mnc;
	lac = pRefLoc->u.cellID.lac;
	cid = pRefLoc->u.cellID.cid;

	if(cid != -1)
	{
		TdCellID.CellInfoFlag = TD_TRUE;
	}
	else
	{
		TdCellID.CellInfoFlag = TD_FALSE;
		D("Cell ID is not available");
	}

	if(type == AGPS_REF_LOCATION_TYPE_GSM_CELLID)
	{
		TdCellID.m.type = TD_GSM;
		TdCellID.m.mcc = mcc;
		TdCellID.m.mnc = mnc;
		TdCellID.m.lac = lac;
		TdCellID.m.cid = cid;
		D("GSM_CELLID : mcc = %u, mnc = %u, lac = %u, cid = %x", mcc, mnc, lac, cid);
	}
	else if (type == AGPS_REF_LOCATION_TYPE_UMTS_CELLID)
	{
		TdCellID.m.type = TD_WCDMA;
		TdCellID.m.mcc = mcc;
		TdCellID.m.mnc = mnc;
		TdCellID.m.cid = cid;
		D("WCDMA_CELLID : mcc = %u, mnc = %u, lac = %u, cid = %u", mcc, mnc, lac, cid);
	}

	if(TdCellID.CellInfoFlag == TD_TRUE)
		CP_SendCellInfo(&TdCellID);

	return;
}

static void td_agps_ril_set_set_id(AGpsSetIDType type, const char *setid)
{
	tTD_UINT8 SETidValue[ID_LENGTH];
	tTD_UINT8 setid_converted[16];
	int i = 0;
	int error = 0;
	int set_id_length = 0;

	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s(%s)", __FUNCTION__,setid);


	if(setid == NULL)
	{
		D("ERROR : ID null");
		error = 1;
	}

	set_id_length = strlen(setid);
	
	memset(setid_converted, 0, sizeof(setid_converted));
	memcpy(TdIdInfo.setid, setid, 15);
	TdIdInfo.setid[15] = 0;

	if(!error)
	{
		for(i = 0; i < set_id_length; i++)
		{
			if( ((*(setid + i)) >= '0') && ((*(setid + i)) <= '9'))
			{
				setid_converted[i] = (*(setid + i)) - '0';
			}
			else
			{
				setid_converted[i] = (*(setid + i)) - 'A' + 10;
			}
		}

		for( i = 0 ; i < 8; i++)
		{
			SETidValue[i] = (setid_converted[i * 2] << 4) | setid_converted[i * 2 + 1];
		}

		TdIdInfo.IdSetFlag = TD_TRUE;
		TdIdInfo.IdType = type;
		memcpy(TdIdInfo.IdValue, SETidValue, ID_LENGTH);
		//D("id convert %s",SETidValue);

        CP_SendSETIDInfo(&TdIdInfo);
		D("Succeed : SETID Info Valid : CP_SendSETIDInfo Success");
	}

	return;
}

static void td_agps_ril_ni_message (uint8_t *msg, size_t len)
{
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);

    if(len == 0 || msg == 0)return;
	
	return;
}
void set_gprs_interface(char *s)
{
	gprs_interface[0] = '\0';

	if (s != NULL)
	{
		D("gprs_interface from Location Manger %s", s);
		strncpy(gprs_interface, s, AGPS_GPRS_LEN - 1/*strlen(s) + 1*/);
		D("gprs_interface in PAL %s", gprs_interface);
	}
}

//when net change call the next function
static void td_agps_ril_update_network_state(int connected, int type, int roaming, const char *extra_info)
{
	char ifname[AGPS_GPRS_LEN];

	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s(%d,%d,%d,%s)", __FUNCTION__, connected, type, roaming, extra_info);

	ifname[0] = '\0';

	if (connected)
	{
		if (type == AGPS_RIL_NETWORK_TYPE_MOBILE)
		{
			snprintf(ifname, AGPS_GPRS_LEN - 1, "ccinet0");
		}

		if (roaming)
		{
			ifname[0] = '\0';
		}

		NetAccess = TD_TRUE;
	}
	else
		NetAccess = TD_FALSE;

	if (strlen(ifname) > 0)
	{
		set_gprs_interface(ifname);
	}

	return;
}

static void td_agps_ril_network_availability(int avaiable, const char *apn)
{
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s(%d,%s)", __FUNCTION__, avaiable, apn);
	return;
}

static const AGpsRilInterface tdAGpsRilInterface =
{
	sizeof(AGpsRilInterface),
	td_agps_ril_init,
	td_agps_ril_set_ref_location,
	td_agps_ril_set_set_id,
	td_agps_ril_ni_message,
	td_agps_ril_update_network_state,
	td_agps_ril_network_availability,
};

static int RequestRefLocation(void)
{
	//start = time(NULL);
	if(gpsRilCallbacks.request_refloc)
	{
		D("Framework : Request_refloc to Framework");
		gpsRilCallbacks.request_refloc(AGPS_RIL_REQUEST_REFLOC_CELLID);
	}
	else
	{
		D("ERROR : No request_refloc");
		return -1;
	}
	td_mode.agps_que_count=0;
	return 0;
}

static int RequestSetId(void)
{
	if(gpsRilCallbacks.request_setid)
	{
		D("Framework : Send RequestSetId to Android Framework");
		gpsRilCallbacks.request_setid(AGPS_RIL_REQUEST_SETID_IMSI);
		//gpsRilCallbacks.request_setid(AGPS_RIL_REQUEST_SETID_MSISDN);
	}
	else
	{
		D("ERROR : No request_setid");
		return -1;
	}

	return 0;
}
/*
static int AgpsStatusCallback(void)
{
	AGpsStatus agpsstatus;
	agpsstatus.size = sizeof(AGpsStatus);
	agpsstatus.type = AGPS_TYPE_SUPL;
	agpsstatus.status = GPS_RELEASE_AGPS_DATA_CONN;

	if(agpsCallbacks.status_cb != NULL)
	{
		agpsCallbacks.status_cb(&agpsstatus);
		D("DATA Connection Req - Type : %d, Status : %d", agpsstatus.type, agpsstatus.status);
	}
	else
	{
		return -1;
	}

	return 0;
}*/























/******************************************************************************************************/
static int
nmea_tokenizer_init( NmeaTokenizer  *t, const char  *p, const char  *end )
{
	int    count = 0;


	//char buff[300];
	// the initial '$' is optional
	if (p < end && p[0] == '$')
		p += 1;

	// remove trailing newline
	if (end > p && end[-1] == '\n')
	{
		end -= 1;

		if (end > p && end[-1] == '\r')
			end -= 1;
	}

	// get rid of checksum at the end of the sentecne
	if (end >= p + 3 && end[-3] == '*')
	{
		end -= 3;
	}

	while (p < end)
	{
		const char  *q = p;

		q = memchr(p, ',', end - p);

		if (q == NULL)
			q = end;

		if (q >= p)
		{
			if (count < MAX_NMEA_TOKENS)
			{
				t->tokens[count].p   = p;
				t->tokens[count].end = q;
				count += 1;
			}
		}

		if (q < end)
			q += 1;

		p = q;
	}

	t->count = count;
	return count;
}

static Token
nmea_tokenizer_get( NmeaTokenizer  *t, int  index )
{
	Token  tok;
	static const char  *dummy = "";
	static const char  *changestring = "00";

	if (index < 0 || index >= t->count)
	{
		tok.p = tok.end = dummy;
	}
	else if(t->tokens[index].p == t->tokens[index].end)
	{
		tok.p = changestring;
		tok.end = changestring + 1;
	}
	else
		tok = t->tokens[index];

	return tok;
}


static int
str2int( const char  *p, const char  *end )
{
	int   result = 0;
	int   len    = end - p;

	for ( ; len > 0; len--, p++ )
	{
		int  c;

		if (p >= end)
			goto Fail;

		c = *p - '0';

		if ((unsigned)c >= 10)
			goto Fail;

		result = result * 10 + c;
	}

	return  result;

Fail:
	return -1;
}

static double
str2float( const char  *p, const char  *end )
{
	int   len    = end - p;
	char  temp[16];

	if (len >= (int)sizeof(temp))
		return 0.;

	memcpy( temp, p, len );
	temp[len] = 0;
	return strtod( temp, NULL );
}

/*****************************************************************/
/*****                                                       *****/
/*****       N M E A   P A R S E R                           *****/
/*****                                                       *****/
/*****************************************************************/

#define  NMEA_MAX_SIZE  255

typedef struct
{
	int     pos;
	int     overflow;
	int     utc_year;
	int     utc_mon;
	int     utc_day;
	int     utc_diff;
	GpsLocation  fix;
	GpsSvStatus  sv_status;
	int     sv_status_changed;
	gps_location_callback  callback;
	char    in[ NMEA_MAX_SIZE + 1 ];
} NmeaReader;

/*
static void
nmea_reader_update_utc_diff( NmeaReader  *r )
{
	time_t         now = time(NULL);
	struct tm      tm_local;
	struct tm      tm_utc;
	long           time_local, time_utc;

	gmtime_r( &now, &tm_utc );
	localtime_r( &now, &tm_local );

	time_local = tm_local.tm_sec +
				 60 * (tm_local.tm_min +
					   60 * (tm_local.tm_hour +
							 24 * (tm_local.tm_yday +
								   365 * tm_local.tm_year)));

	time_utc = tm_utc.tm_sec +
			   60 * (tm_utc.tm_min +
					 60 * (tm_utc.tm_hour +
						   24 * (tm_utc.tm_yday +
								 365 * tm_utc.tm_year)));

	r->utc_diff = time_utc - time_local;
	D("UTC_DIFF:%d",r->utc_diff);
}*/


static void
nmea_reader_init( NmeaReader  *r )
{
	memset( r, 0, sizeof(*r) );

	r->pos      = 0;
	r->overflow = 0;
	r->utc_year = -1;
	r->utc_mon  = -1;
	r->utc_day  = -1;
	r->callback = NULL;
	r->fix.size = sizeof(r->fix);
	r->sv_status.size = sizeof(GpsSvStatus);
	//nmea_reader_update_utc_diff( r );
}


static void
nmea_reader_set_callback( NmeaReader  *r, gps_location_callback  cb )
{
	r->callback = cb;

	if (cb != NULL && r->fix.flags != 0)
	{
		//D("%s: sending latest fix to new callback", __FUNCTION__);
		// r->callback( &r->fix );
		r->fix.flags = 0;
	}
}
/*
static void UTC2BTC(struct utc_t *GPS)   
{             
    GPS->hour+=8; 
    if(GPS->hour>23)   
    {   
    	GPS->hour-=24;   
        GPS->day+=1;   
        if(GPS->month==2 ||GPS->month==4 ||GPS->month==6 ||GPS->month==9 ||GPS->month==11 ){   
        	if(GPS->day>30){   
            	GPS->day=1;   
                GPS->month++;   
                }   
            }   
            else{   
                if(GPS->day>31){   
                    GPS->day=1;   
                    GPS->month++;   
                }   
            }
			if( ((GPS->year % 4 == 0)&&(GPS->year % 100 != 0))
             || (GPS->year%400==0) ){ 
                if(GPS->day > 29 && GPS->month ==2){    
                    GPS->day=1;   
                    GPS->month++;   
                }   
            }   
            else{   
                if(GPS->day>28 &&GPS->month ==2){   
                    GPS->day=1;   
                    GPS->month++;   
                }   
            }   
            if(GPS->month>12){   
                GPS->month-=12;   
                GPS->year++;   
            }          
        }   
}
*/

static int
nmea_reader_update_time( NmeaReader  *r, Token  tok )
{
	int        hour, minute;
	double     seconds;
	struct tm  tm;
	time_t     fix_time;
  time_t now;
	struct tm *local_tm;
	struct tm *utc_tm;
	long local_time;
	long utc_time;
	long delta_time;

	if (tok.p + 6 > tok.end)
		return -1;
	
	hour    = str2int(tok.p,   tok.p + 2);
	minute  = str2int(tok.p + 2, tok.p + 4);
	seconds = str2float(tok.p + 4, tok.end);
	struct utc_t GPS;
	GPS.year=r->utc_year;
	GPS.month=r->utc_mon;
	GPS.day=r->utc_day;
	GPS.hour=hour;
	GPS.min=minute;
	GPS.sec=seconds;
//	UTC2BTC(&GPS); 

	tm.tm_hour  = GPS.hour;
	tm.tm_min   = GPS.min;
	tm.tm_sec   = (int) seconds;
	tm.tm_year  = GPS.year - 1900;
	tm.tm_mon   = GPS.month - 1;
	tm.tm_mday  = GPS.day;
	tm.tm_isdst = -1;
	
	fix_time = mktime( &tm );
//	if(r->utc_year==2000){
//		r->fix.flags=0;
//		return 0;
//	}	
	
	now = time(NULL);
	local_tm = localtime(&now);
	local_time =	local_tm->tm_sec + 60*(local_tm->tm_min + 60*(local_tm->tm_hour + 24*(local_tm->tm_yday + 365*local_tm->tm_year)));
	utc_tm = gmtime(&now);
	utc_time =	utc_tm->tm_sec + 60*(utc_tm->tm_min + 60*(utc_tm->tm_hour + 24*(utc_tm->tm_yday + 365*utc_tm->tm_year)));
	delta_time = local_time - utc_time;

	r->fix.timestamp = (long long)(fix_time + delta_time) * 1000 + (int)(seconds * 1000) % 1000;
	return 0;
}




static int
nmea_reader_update_date( NmeaReader  *r, Token  date, Token  time )
{
	Token  tok = date;
	int    day, mon, year;

	if (tok.p + 6 != tok.end)
	{
		D("date not properly formatted: '%.*s'", tok.end - tok.p, tok.p);
		return -1;
	}

	day  = str2int(tok.p, tok.p + 2);
	mon  = str2int(tok.p + 2, tok.p + 4);
	year = str2int(tok.p + 4, tok.p + 6) + 2000;

	if ((day | mon | year) < 0)
	{
		D("date not properly formatted: '%.*s'", tok.end - tok.p, tok.p);
		return -1;
	}

	r->utc_year  = year;
	r->utc_mon   = mon;
	r->utc_day   = day;

	return nmea_reader_update_time( r, time );
}


static double
convert_from_hhmm( Token  tok )
{
	double  val     = str2float(tok.p, tok.end);
	int     degrees = (int)(floor(val) / 100);
	double  minutes = val - degrees * 100.;
	double  dcoord  = degrees + minutes / 60.0;
	return dcoord;
}

static int
nmea_reader_update_latlong( NmeaReader  *r,
							Token        latitude,
							char         latitudeHemi,
							Token        longitude,
							char         longitudeHemi )
{
	double   lat, lon;
	Token    tok;
	tok = latitude;
	char temp1[30] = {0}, temp2[30] = {0};
	int len1, len2;
	len1 = latitude.end - latitude.p;
	len2 = longitude.end - longitude.p;
	memcpy(temp1, latitude.p, len1);
	temp1[len1] = 0;
	memcpy(temp2, longitude.p, len2);
	temp2[len2] = 0;
	//D("nmea latitude : %s  longitude : %s ", temp1,temp2);


	if (tok.p + 6 > tok.end)
	{
		 //D("latitude is too short0: '%.*s'", tok.end-tok.p, tok.p);
		 r->fix.flags    &= ~(GPS_LOCATION_HAS_LAT_LONG);
		return -1;
	}

	lat = convert_from_hhmm(tok);

	if (latitudeHemi == 'S')
		lat = -lat;

	tok = longitude;

	if (tok.p + 6 > tok.end)
	{
		//D("longitude is too short1: '%.*s'", tok.end-tok.p, tok.p);
		r->fix.flags    &= ~(GPS_LOCATION_HAS_LAT_LONG);
		return -1;
	}

	lon = convert_from_hhmm(tok);

	if (longitudeHemi == 'W')
		lon = -lon;

	if((lat == 0) && (lon == 0))
	{	
		r->fix.flags    &= ~(GPS_LOCATION_HAS_LAT_LONG);
		return -1;
	}
	else
	{
		r->fix.latitude = lat;
		r->fix.longitude = lon;
		r->fix.flags    |= GPS_LOCATION_HAS_LAT_LONG;
		if (GetDebugMaskBit(D_FIX) == 1)
		{
		D("latitude : %f  longitude : %f ",  r->fix.latitude, r->fix.longitude);
		}
	}

	return 0;
}


static int nmea_reader_update_altitude( NmeaReader *r,Token altitude)
{
	Token   tok = altitude;
	char temp1[30] = {0};
	int len1;
	len1 = altitude.end - altitude.p;
	memcpy(temp1, altitude.p, len1);
	temp1[len1] = 0;
	//D("nmea altitude : %s ",temp1);


	if (tok.p >= tok.end){
		r->fix.flags   &= ~(GPS_LOCATION_HAS_ALTITUDE);
		return -1;
	}	
	if((r->fix.flags)&(GPS_LOCATION_HAS_LAT_LONG))
		r->fix.flags   |= GPS_LOCATION_HAS_ALTITUDE;
	else
		r->fix.flags   &= ~(GPS_LOCATION_HAS_ALTITUDE);
	r->fix.altitude = str2float(altitude.p, altitude.end);
	if (GetDebugMaskBit(D_FIX) == 1)
	{
	D("altitude : %f ", r->fix.altitude);
	}

	return 0;
}


static int
nmea_reader_update_accuracy( NmeaReader  *r,

							 Token        accuracy )

{


	Token   tok = accuracy;
	char temp1[30] = {0};
	int len1;
	len1 = accuracy.end - accuracy.p;
	memcpy(temp1, accuracy.p, len1);
	temp1[len1] = 0;
	//D("nmea accuracy : %s ",temp1);


	if (tok.p >= tok.end){
		r->fix.flags   &= ~(GPS_LOCATION_HAS_ACCURACY);
		return -1;
	}	

	r->fix.accuracy = str2float(tok.p, tok.end);
/*
	if (r->fix.accuracy == 99.99)
	{
		r->fix.flags   &= ~(GPS_LOCATION_HAS_ACCURACY);
		return 0;

	}
*/
	if((r->fix.flags)&(GPS_LOCATION_HAS_LAT_LONG))
		r->fix.flags   |= GPS_LOCATION_HAS_ACCURACY;
	else
		r->fix.flags   &= ~(GPS_LOCATION_HAS_ACCURACY);
	//r->fix.flags   |= GPS_LOCATION_HAS_ACCURACY;
	if (GetDebugMaskBit(D_FIX) == 1)
	{
	D("accuracy : %f", r->fix.accuracy);
	}

	return 0;

}



static int
nmea_reader_update_bearing( NmeaReader  *r,
							Token        bearing )
{
	Token   tok = bearing;
	char temp1[30] = {0};
	int len1;
	len1 = bearing.end - bearing.p;
	memcpy(temp1, bearing.p, len1);
	temp1[len1] = 0;
	//D("nmea bearing : %s ", temp1);


	if (tok.p >= tok.end){
		r->fix.flags   &= (~GPS_LOCATION_HAS_BEARING);
		//D("bearing- : r->fix.flags 0x%x ", r->fix.flags);
		return -1;
	}	

	//r->fix.flags   |= GPS_LOCATION_HAS_BEARING;
	
	if((r->fix.flags)&(GPS_LOCATION_HAS_LAT_LONG)){
		//D("bearing0 : r->fix.flags 0x%x ", r->fix.flags);
		r->fix.flags   |= GPS_LOCATION_HAS_BEARING;
	}	
	else
		r->fix.flags   &= ~(GPS_LOCATION_HAS_BEARING);
	r->fix.bearing  = str2float(tok.p, tok.end);
	//D("bearing : r->fix.flags 0x%x ", r->fix.flags);
	if (GetDebugMaskBit(D_FIX) == 1)
	{
	D("bearing : %f ", r->fix.bearing);
	}
	return 0;
}


static int
nmea_reader_update_speed( NmeaReader  *r,
						  Token        speed )
{
	Token   tok = speed;
	char temp1[30] = {0};
	int len1;
	len1 = speed.end - speed.p;
	memcpy(temp1, speed.p, len1);
	temp1[len1] = 0;
	//D("nmea speed : %s ",temp1);


	if (tok.p >= tok.end){
		r->fix.flags   &= (~GPS_LOCATION_HAS_SPEED);
		return -1;
	}	

	if((r->fix.flags)&(GPS_LOCATION_HAS_LAT_LONG)){
		//D("speed : r->fix.flags0 0x%x ", r->fix.flags);
		r->fix.flags   |= GPS_LOCATION_HAS_SPEED;
	}	
	else
		r->fix.flags   &= ~(GPS_LOCATION_HAS_SPEED);
	//D("speed : r->fix.flags 0x%x ", r->fix.flags);
	//r->fix.flags   |= GPS_LOCATION_HAS_SPEED;
	r->fix.speed    = (float)str2float(tok.p, tok.end) * 1.852 / 3.6;
	if (GetDebugMaskBit(D_FIX) == 1)
	{
	D("speed : %f ", r->fix.speed);
	}

	return 0;
}


void update_gps_svstatus(GpsSvStatus *svstatus);


static int NmeaCheck(NmeaReader  *r)
{
	unsigned char buff[500];
	char *p, *end;

	int res;
	int ret;
	p = r->in;
	end = r->in + r->pos;
	memset(buff, 0, sizeof(buff));
	memcpy(buff, p, (end - p));
	/*if(!memcmp(buff,"$GNRMC",6))
		D("%s",buff);
	if(!memcmp(buff,"$GNGGA",6))
		D("%s",buff);*/
	/*if(buff[0]==0x40){	
		for(i=0;i<end-p+1;i++)
			D("nmea que v buff[%d]=%X",i,buff[i]);
	}*/
	/*if(!memcmp(buff,"$RUTXT",6))
		D("%s",buff);*/
	pthread_mutex_lock(&version_check_struct_lock);
	if((version_check.state == AUTO_UPDATE_STATE_CHECK_VERSION)&&(buff[0]==0x40))
	{
		D("111");
		if((unsigned int)(r->pos)+2<sizeof(buff))
		{
			buff[end-p]=0x0d;
			buff[end-p+1]=0x0a;
		}
		if(GetVersionRes(buff, 0, r->pos+2, &version_check.ver) == 0)
		{
			D("get version nak");
		}else{
			D("get version");
			pthread_mutex_lock(&version_check.mutex);
			ret = pthread_cond_signal(&version_check.cond);
			pthread_mutex_unlock(&version_check.mutex);
			D("get version222 time:%d  signa:ret=%d",(int)time(NULL),ret);
		}
	}else if (buff[0]==0x40)
	{
		D("222");
		D("version_check.state=%d",version_check.state);
	}
	pthread_mutex_unlock(&version_check_struct_lock);

	update_nmea(r->in, r->in + r->pos);
	res = _LibNMEA_Xor((char *)buff);
	return res;
}

static int _LibNMEA_Xor(char *p_chk_str)
{
	unsigned int check_sum = 0;
	int n, i;
	char crcbuf[4];
	memset(crcbuf, 0, sizeof(crcbuf));
	p_chk_str++;
	n = strlen(p_chk_str) - 5;

	if(n < 0)
		return -1;

	for(i = 0; i < n; i++)
	{
		check_sum ^= (*p_chk_str);
		//D("sum:0x%x : %c",check_sum,*p_chk_str);
		p_chk_str++;

		if(n == 500)
			return -1;
	}

	if(check_sum < 0x10)
		sprintf(crcbuf, "0%X", check_sum);
	else
		sprintf(crcbuf, "%X", check_sum);

	p_chk_str++;

	if(memcmp(crcbuf, p_chk_str, 2) == 0)
	{
		return 0;
	}
	else
	{
		//D("nmea crc f : %s", p_chk_str);
		return -1;
	}
}

#if 0
static void FixSattileCheck(int prn, NmeaReader  *r)
{
	int i;

	if(GsaPrn.fixprnnum == 0)
		return;

	for(i = 0; i < GsaPrn.fixprnnum; i++)
	{
		if(prn == GsaPrn.fixprn[i])
		{	
			int basenum = 0;
			
			int posinint = 0;

			if(prn < 33)//GPS
			{
              basenum = 0;
			  posinint = prn - 1;
			}
			else if(prn < 65)//SBAS
			{
              basenum = 1;
			  posinint = prn - 32 - 1;
			}
			else if(prn < 97)//GLO
			{
              basenum = 2;
			  posinint = prn - 64 - 1;
			}
			else if(prn < 129)//GLO
			{
              basenum = 3;
			  posinint = prn - 96 - 1;
			}			
			else if(prn > 200 && prn < 233)//BDS
			{
              basenum = 4;
			  posinint = prn - 200 - 1;
			}
			else if(prn < 265)//BDS
			{
              basenum = 5;
			  posinint = prn - 232 - 1;
			}
			else if(prn > 400 && prn < 433)//GA
			{
              basenum = 6;
			  posinint = prn - 400 - 1;
			}
			else if(prn > 432)//GA
			{
              basenum = 7;
			  posinint = prn - 432 - 1;
			}


			
			unsigned  int prnshift = (1 << posinint);
			r->sv_status.used_in_fix_mask[basenum] |= prnshift;

			return;
		}
	}
}
#endif
static void FixSattileCheck(int prn, NmeaReader  *r)
{
	int i;

	if(GsaPrn.fixprnnum == 0)
		return;

	if((prn<=0) ||(prn>256))
		return;

	for(i = 0; i < GsaPrn.fixprnnum; i++)
	{
		if((prn == GsaPrn.fixprn[i]) || (fixed[prn] == 1))
		{
			fixed[prn] = 1;
			r->sv_status.used_in_fix_mask |= (1 << (r->sv_status.num_svs));
			return;
		}
	}
}

void up_svstatus(NmeaReader  *r )
{
	//if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);
	if(r->callback!=NULL)
		update_bdgps_svstatus(&r->sv_status);
	memset(&GsaPrn, 0, sizeof(GsaPrn));
	memset(&r->sv_status, 0, sizeof(r->sv_status));
	return;
}
/*
static void QueAgpsMaskAndWriteData(NmeaTokenizer  *tzer)
{
	Token  tok_agps_cmd   =  nmea_tokenizer_get(tzer, 3);
	GpsState *s = (GpsState *)_gps_state;
	static int agps_que_counter = 0;
	int agps_cmd = str2int(tok_agps_cmd.p, tok_agps_cmd.end);
	int len;
	int k=0;
	//memset(prn,0,sizeof(prn));
	if((agps_cmd == 5) && (agps_que_counter < 4))
	{
		unsigned char que_eph_mask_buff[9];
		unsigned int que_mask_mask = 0, check_mask = 0;
		int i, j;
		char str_num[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
		memset(que_eph_mask_buff, 0, sizeof(que_eph_mask_buff));
		Token tok_agps_mask = nmea_tokenizer_get(tzer, 4);
		memcpy(que_eph_mask_buff, tok_agps_mask.p, 8);
		D("nmea mask:%s", que_eph_mask_buff);

		for(i = 0; i < 8; i++)
		{
			if((que_eph_mask_buff[i] >= '0') && (que_eph_mask_buff[i] <= 'f'))
			{
				for(j = 0; j < 16; j++)
				{
					if(que_eph_mask_buff[i] == str_num[j])
					{
						que_mask_mask = que_mask_mask * 16 + j;
						break;
					}
				}
			}
			else
			{
				D("nmea $RUTXT mask error!agpsmask:%d", que_mask_mask);
				return;
			}
		}

		D("AGPS MASK:0X%X", que_mask_mask);

		if(que_mask_mask == 0)
		{
			agps_que_counter = 0;
			D("AGPS Send all prn!");
			return;
		}

		check_mask = que_mask_mask & td_agps_data.ehp_new.eph_mask;

		if(check_mask == 0)
		{
			D("EPH prn Send ok!,counter=%d", agps_que_counter);
			ap_chip_agps.write_agps_first=TRUE;
			agps_que_counter = 0;
			return;
		}

		for(i = 0; i < EPH_MAX_LINES; i++)
		{
			if((check_mask & 0x01) == 0x01)
			{
				len = strlen(td_agps_data.ehp_new.line[i]);			
				pthread_mutex_lock(&UartWriteMutex);
				write(s->fd, &td_agps_data.ehp_new.line[i], len);			
				pthread_mutex_unlock(&UartWriteMutex);
				usleep(50000);
				k++;
				//D("QUE Write $CCGEP data! prn:%d", i + 1);
			}

			check_mask = check_mask >> 1;
		}
		D("Que EPH write num:i=%d", k);
		if(i > 0)
		{				
			pthread_mutex_lock(&UartWriteMutex);
			write(s->fd, CMD_AGPS_QUE, CMD_AGPS_QUE_LEN);			
			pthread_mutex_unlock(&UartWriteMutex);
			D("Que EPH prn_num:i=%d", i);
			agps_que_counter++;
		}
	}

	return;

}*/

static void ClearSv( NmeaReader  *r )
{
	memset(&r->sv_status.used_in_fix_mask, 0, sizeof(r->sv_status.used_in_fix_mask));   // fix bug 20170601
	memset(&r->sv_status, 0, sizeof(r->sv_status));
	memset(&GsaPrn, 0, sizeof(GsaPrn));
	r->sv_status.size = sizeof(GpsSvStatus);
	return;
}
/*
static bool time_cnt_one(void)
{
	time_t tm;
	tm=time(NULL);
	if(ap_chip_agps.time_lac==0){
		ap_chip_agps.time_lac=tm;
		return FALSE;
	}	
	else{
		if((tm-ap_chip_agps.time_lac)>=1)
		{
			ap_chip_agps.time_lac=tm;
			return TRUE;
		}
	}
	return FALSE;
}*/
/*
int writen(int fd,const void *buffer, int length)
{
	int len = 0;
	if(fd != -1)
	{

		do
		{
			pthread_mutex_lock(&UartWriteMutex);
			len = write(fd, buffer, length );
			pthread_mutex_unlock(&UartWriteMutex);
			D("write:len=%d errno:%d  error:%s",len, errno,strerror(errno));
		}
		while (len < 0 && (errno == EINTR || errno == EAGAIN));

		return len;
	}
	else
	{
		return 0;
	}
}

static void agps_deal(void)
{
	if(!(time_cnt_one()))
		return;
	GpsState *s = (GpsState *)_gps_state;
	ap_chip_agps.time_cnt++;
	if(ap_chip_agps.agps_get_status==FALSE)
	{
		if(ap_chip_agps.time_cnt==AGPS_TIMEOUT){
			D("AGPS TIMEOUT request again!!");
			RequestSetId();
			RequestRefLocation();
			ap_chip_agps.time_cnt=0;
			ap_chip_agps.agps_flag=AGPS_ON;
		}
	}else{
		if(ap_chip_agps.write_agps_first==TRUE){
			ap_chip_agps.agps_time_cnt++;
			D("time cnt:%d",ap_chip_agps.agps_time_cnt);
			if(!(ap_chip_agps.agps_time_cnt%TIME_QUE_AGPS_CHIP)){					
				if(ap_chip_agps.agps_time_cnt==TIME_MASK_RENEW)
					ap_chip_agps.mask_flag=TRUE;
				D("send que_cmd!");
				writen(s->fd,CMD_AGPS_QUE,CMD_AGPS_QUE_LEN);
				//write(s->fd, CMD_AGPS_QUE, CMD_AGPS_QUE_LEN);				
			}			
		}
	}
}
*/
/*
static void rutxt_deal(NmeaTokenizer  *tzer)
{
	D("$RUTXT");
	unsigned int mask;
	bool ret;
	if(ap_chip_agps.write_agps_first==FALSE){
		td_mode.agps_que_count++;
		if(td_mode.agps_que_count<5)
			QueAgpsMaskAndWriteData(tzer);
	}else{
		if(get_mask(tzer,&mask)){
			ap_chip_agps.receive_rutxt_cnt++;
			if(ap_chip_agps.receive_rutxt_cnt == 1){
				ap_chip_agps.mask_chip_first=mask;
				cmp_mk_wrdata(&mask);
			}else if(ap_chip_agps.receive_rutxt_cnt==10){				
				ap_chip_agps.mask_chip_second=mask;
				ap_chip_agps.receive_rutxt_cnt=0;
				cmp_renew_agps(&ap_chip_agps.mask_chip_first,&ap_chip_agps.mask_chip_second);
				
			}else{
				cmp_mk_wrdata(&mask);				
			}				
		}
	}
}

static void cmp_renew_agps(unsigned int *mask1,unsigned int *mask2)
{
	unsigned int mk1,mk2,check_mask;
	mk1 = *mask1;
	mk2 = *mask2;
	check_mask=mk1&mk2&td_agps_data.ehp_new.eph_mask; 
	D("renew:mask_f:%X mask_s:%X,check_mask:%X",*mask1,*mask2,check_mask);
	if((check_mask)!=0){
		D("Renew agps data!");
		memset(&ap_chip_agps,0,sizeof(ap_chip_agps));
		ap_chip_agps.agps_flag=AGPS_ON;
		RequestSetId();
		RequestRefLocation();	
	}else{
		ap_chip_agps.mask_chip_first=ap_chip_agps.mask_chip_second;
		ap_chip_agps.mask_chip_second=0;
		ap_chip_agps.receive_rutxt_cnt=0;
	}
}

static void write_ccaing(int fd,unsigned int mask)
{
	int i=0;
	int len;
	unsigned int temp_eph_mask;
	char aing[50];
	char s1[50];
	int counter_gep=0;
	unsigned int check_sum = 0;
	memset(aing,0,sizeof(aing));		
	strcat(aing,CMD_AGPS_ING);

	memset(s1,0,sizeof(s1));
	sprintf(s1,"%s","0,");
	strcat(aing,s1);
	temp_eph_mask=mask;
	for(i=0;i < EPH_MAX_LINES;i++){
		if((temp_eph_mask & 0x01) == 1)
			counter_gep++;
			temp_eph_mask = temp_eph_mask >> 1;
	}
	memset(s1,0,sizeof(s1));
	sprintf(s1,"%d,%x",counter_gep,mask);
	strcat(aing,s1);		
	check_sum=CrcSum(aing);
	memset(s1,0,sizeof(s1));
	sprintf(s1,"*%X\r\n",check_sum);
	strcat(aing,s1);
	D("AING_1:%s",aing);
	write(fd, aing, strlen(aing));
	usleep(50000);
	return;
}

static void cmp_mk_wrdata(unsigned int *mask)
{
	unsigned check_mask,que_mask_mask;
	int len,i;
	que_mask_mask=*mask;
	check_mask = que_mask_mask & td_agps_data.ehp_new.eph_mask;
	GpsState *s = (GpsState *)_gps_state;

	if(check_mask==0)
		return;
	write_ccaing(s->fd,check_mask);
	for(i = 0; i < EPH_MAX_LINES; i++)
	{
		if((check_mask & 0x01) == 0x01)
		{
			len = strlen(td_agps_data.ehp_new.line[i]);
			pthread_mutex_lock(&UartWriteMutex);
			write(s->fd, &td_agps_data.ehp_new.line[i], len);			
			pthread_mutex_unlock(&UartWriteMutex);
			D("write ehp :%d",i+1);
			usleep(50000);
		}
		check_mask = check_mask >> 1;
	}
	
}

static bool get_mask(NmeaTokenizer  *tzer,unsigned int *mask)
{
	Token  tok_agps_cmd   =  nmea_tokenizer_get(tzer, 3);
	int agps_cmd = str2int(tok_agps_cmd.p, tok_agps_cmd.end);
	int len;
	if(agps_cmd == 5)
	{
		unsigned char que_eph_mask_buff[9];
		unsigned int que_mask_mask = 0, check_mask = 0;
		int i, j;
		char str_num[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
		memset(que_eph_mask_buff, 0, sizeof(que_eph_mask_buff));
		Token tok_agps_mask = nmea_tokenizer_get(tzer, 4);
		memcpy(que_eph_mask_buff, tok_agps_mask.p, 8);
		D("nmea mask_1:%s", que_eph_mask_buff);

		for(i = 0; i < 8; i++)
		{
			if((que_eph_mask_buff[i] >= '0') && (que_eph_mask_buff[i] <= 'f'))
			{
				for(j = 0; j < 16; j++)
				{
					if(que_eph_mask_buff[i] == str_num[j])
					{
						que_mask_mask = que_mask_mask * 16 + j;
						break;
					}
				}
			}
			else
			{
				D("nmea $RUTXT mask error!agpsmask_1:%d", que_mask_mask);
				return FALSE;
			}
		}
		D("AGPS MASK_1:0X%X", que_mask_mask);
		*mask=que_mask_mask;
		return TRUE;
	}
	return FALSE;
}
*/


/**********************************************************/
/***NMEA PARSE********************************************/
/*********************************************************/

static void
nmea_reader_parse( NmeaReader  *r )
{
	NmeaTokenizer  tzer[1];
	Token          tok;
	int rmcmode=0;

	char gsvhead[5] = {0, 0, 0, 0, 0};
    static int GNModeList[2] = {MODE_GPS,MODE_BD};//
	//int i;
	
	nmea_tokenizer_init(tzer, r->in, r->in + r->pos);
	if(td_mode.work==0)
		td_mode.work=1;
    tdconfig.power_flag = TD_ON;
	
    if (GetDebugMaskBit(D_TOKEN) == 1)
	{
		int  n;
		D("Found %d tokens", tzer->count);

		for (n = 0; n < tzer->count; n++)
		{
			nmea_tokenizer_get(tzer, n);

		}
	}


	tok = nmea_tokenizer_get(tzer, 0);

	if (tok.p + 5 > tok.end)
	{
		D("nmea sentence  '%.*s' too short, ignored.", tok.end-tok.p, tok.p);
		return;
	}

	memcpy(gsvhead, tok.p, 5);
	tok.p += 2;

	if(!memcmp(tok.p, "GSV", 3))
	{
		if(!memcmp(gsvhead, "GPGSV", 5))
		{
            
		
			Token  tok_noSatellites  = nmea_tokenizer_get(tzer, 3);//visibility satellites
			int    noSatellites = str2int(tok_noSatellites.p, tok_noSatellites.end);

            if(GNModeList[1] != MODE_GPS)
            {
              GNModeList[0] = GNModeList[1];
              GNModeList[1] = MODE_GPS;
            }
			
			if ((noSatellites > 0) && (r->sv_status.num_svs < GPS_MAX_SVS))
			{
				//Token  tok_noSentences   = nmea_tokenizer_get(tzer, 1);//total number of messages
				//Token  tok_sentence      = nmea_tokenizer_get(tzer, 2);// message number
				//int sentence = str2int(tok_sentence.p, tok_sentence.end);
				//int totalSentences = str2int(tok_noSentences.p, tok_noSentences.end);
				int prn;
				int i = 0;

				while (i < 4)
				{
					Token  tok_prn = nmea_tokenizer_get(tzer, i * 4 + 4);  
					Token  tok_elevation = nmea_tokenizer_get(tzer, i * 4 + 5);
					Token  tok_azimuth = nmea_tokenizer_get(tzer, i * 4 + 6);
					Token  tok_snr = nmea_tokenizer_get(tzer, i * 4 + 7);
					prn = str2int(tok_prn.p, tok_prn.end);

					if(prn > 0)
					{
						if(r->sv_status.num_svs > (GPS_MAX_SVS - 1))
						{
							r->sv_status_changed = 1;
							return;
						}

						r->sv_status.sv_list[r->sv_status.num_svs].prn = prn;
						FixSattileCheck(prn, r);
						r->sv_status.sv_list[r->sv_status.num_svs].elevation = str2float(tok_elevation.p, tok_elevation.end);
						r->sv_status.sv_list[r->sv_status.num_svs].azimuth = str2float(tok_azimuth.p, tok_azimuth.end);
						r->sv_status.sv_list[r->sv_status.num_svs].snr = str2float(tok_snr.p, tok_snr.end);
						r->sv_status.num_svs += 1;
						r->sv_status_changed = 1;
						if (GetDebugMaskBit(D_GSV) == 1)
						{
						  D("GPGSV numsv:%d sv[%2d] prn:%d elevation:%f azimuth:%f snr:%f mask:0x%x", r->sv_status.num_svs, r->sv_status.num_svs - 1, \
						  r->sv_status.sv_list[r->sv_status.num_svs - 1].prn, r->sv_status.sv_list[r->sv_status.num_svs - 1].elevation, r->sv_status.sv_list[r->sv_status.num_svs - 1].azimuth, \
						  r->sv_status.sv_list[r->sv_status.num_svs - 1].snr, r->sv_status.used_in_fix_mask);
						}
						i++;
					}
					else
					{
						break;
					}
				}
			}
		}
		else if(!memcmp(gsvhead, "BDGSV", 5))
		{
			Token  tok_noSatellites  = nmea_tokenizer_get(tzer, 3);//visibility satellites
			int    noSatellites = str2int(tok_noSatellites.p, tok_noSatellites.end);

            if(GNModeList[1] != MODE_BD)
            {

			GNModeList[0] = GNModeList[1];
            GNModeList[1] = MODE_BD;
            }
			
			if ((noSatellites > 0) && (r->sv_status.num_svs < (GPS_MAX_SVS)))
			{
				//Token  tok_noSentences   = nmea_tokenizer_get(tzer, 1);//total number of messages
				//Token  tok_sentence      = nmea_tokenizer_get(tzer, 2);// message number
				//int sentence = str2int(tok_sentence.p, tok_sentence.end);
				//int totalSentences = str2int(tok_noSentences.p, tok_noSentences.end);
				int prn;
				int i = 0;

				while (i < 4)
				{
					if(r->sv_status.num_svs > (GPS_MAX_SVS - 1))
					{
						r->sv_status_changed=1;
						return;
					}

					Token  tok_prn = nmea_tokenizer_get(tzer, i * 4 + 4);
					Token  tok_elevation = nmea_tokenizer_get(tzer, i * 4 + 5);
					Token  tok_azimuth = nmea_tokenizer_get(tzer, i * 4 + 6);
					Token  tok_snr = nmea_tokenizer_get(tzer, i * 4 + 7);
					prn = str2int(tok_prn.p, tok_prn.end);

					if(prn > 0)
					{
						prn += tdconfig.bdprnaddr;
						r->sv_status.sv_list[r->sv_status.num_svs].prn = prn;
						FixSattileCheck(prn, r);
						r->sv_status.sv_list[r->sv_status.num_svs].elevation = str2float(tok_elevation.p, tok_elevation.end);
						r->sv_status.sv_list[r->sv_status.num_svs].azimuth = str2float(tok_azimuth.p, tok_azimuth.end);
						r->sv_status.sv_list[r->sv_status.num_svs].snr = str2float(tok_snr.p, tok_snr.end);
						r->sv_status.num_svs += 1;
						r->sv_status_changed = 1;
						if (GetDebugMaskBit(D_GSV) == 1)
						{
						D("BDGSV numsv:%d sv[%2d] prn:%d elevation:%f azimuth:%f snr:%f  mask:0x%x", r->sv_status.num_svs, r->sv_status.num_svs - 1, \
						  r->sv_status.sv_list[r->sv_status.num_svs - 1].prn, r->sv_status.sv_list[r->sv_status.num_svs - 1].elevation, r->sv_status.sv_list[r->sv_status.num_svs - 1].azimuth, \
						  r->sv_status.sv_list[r->sv_status.num_svs - 1].snr, r->sv_status.used_in_fix_mask);
						}

						i++;
					}
					else
					{
						break;
					}
				}
			}
		}
		else if(!memcmp(gsvhead, "GLGSV", 5))
		{
			Token  tok_noSatellites  = nmea_tokenizer_get(tzer, 3);//visibility satellites
			int    noSatellites = str2int(tok_noSatellites.p, tok_noSatellites.end);

            if(GNModeList[1] != MODE_GL)
            {
			  GNModeList[0] = GNModeList[1];
              GNModeList[1] = MODE_GL;
            }
			
			if ((noSatellites > 0) && (r->sv_status.num_svs < (GPS_MAX_SVS)))
			{
				//Token  tok_noSentences   = nmea_tokenizer_get(tzer, 1);//total number of messages
				//Token  tok_sentence      = nmea_tokenizer_get(tzer, 2);// message number
				//int sentence = str2int(tok_sentence.p, tok_sentence.end);
				//int totalSentences = str2int(tok_noSentences.p, tok_noSentences.end);
				int prn;
				int i = 0;

				while (i < 4)
				{
					if(r->sv_status.num_svs > (GPS_MAX_SVS - 1))
					{
						r->sv_status_changed=1;
						return;
					}

					Token  tok_prn = nmea_tokenizer_get(tzer, i * 4 + 4);
					Token  tok_elevation = nmea_tokenizer_get(tzer, i * 4 + 5);
					Token  tok_azimuth = nmea_tokenizer_get(tzer, i * 4 + 6);
					Token  tok_snr = nmea_tokenizer_get(tzer, i * 4 + 7);
					prn = str2int(tok_prn.p, tok_prn.end);

                    

					if(prn > 0)
					{
						r->sv_status.sv_list[r->sv_status.num_svs].prn = prn;
						FixSattileCheck(prn, r);
						r->sv_status.sv_list[r->sv_status.num_svs].elevation = str2float(tok_elevation.p, tok_elevation.end);
						r->sv_status.sv_list[r->sv_status.num_svs].azimuth = str2float(tok_azimuth.p, tok_azimuth.end);
						r->sv_status.sv_list[r->sv_status.num_svs].snr = str2float(tok_snr.p, tok_snr.end);
						
						r->sv_status.num_svs += 1;
						r->sv_status_changed = 1;
						if (GetDebugMaskBit(D_GSV) == 1)
						{
						D("GLGSV numsv:%d sv[%2d] prn:%d elevation:%f azimuth:%f snr:%f  mask:0x%x", r->sv_status.num_svs, r->sv_status.num_svs - 1, \
						  r->sv_status.sv_list[r->sv_status.num_svs - 1].prn, r->sv_status.sv_list[r->sv_status.num_svs - 1].elevation, r->sv_status.sv_list[r->sv_status.num_svs - 1].azimuth, \
						  r->sv_status.sv_list[r->sv_status.num_svs - 1].snr, r->sv_status.used_in_fix_mask);
						}

						i++;
					}
					else
					{
						break;
					}
				}
			}
		}		

	}

	///////////////////////////////////////////////////////GGA
	else if ( !memcmp(tok.p, "GGA", 3) )  													
	{
		// GPS fix
		r->fix.flags &= (~GPS_LOCATION_HAS_LAT_LONG);
		Token  tok_time          = nmea_tokenizer_get(tzer, 1);
		Token  tok_latitude      = nmea_tokenizer_get(tzer, 2);
		Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer, 3);
		Token  tok_longitude     = nmea_tokenizer_get(tzer, 4);
		Token  tok_longitudeHemi = nmea_tokenizer_get(tzer, 5);
		Token  tok_accuracy      = nmea_tokenizer_get(tzer, 8);
		Token  tok_altitude      = nmea_tokenizer_get(tzer, 9);
		//Token  tok_altitudeUnits = nmea_tokenizer_get(tzer, 10);
		
		nmea_reader_update_latlong(r, tok_latitude,
								   tok_latitudeHemi.p[0],
								   tok_longitude,
								   tok_longitudeHemi.p[0]);
		nmea_reader_update_accuracy(r, tok_accuracy);
		nmea_reader_update_altitude(r, tok_altitude);
		nmea_reader_update_time(r, tok_time);

		///////////////////////////////////////////////////////RMC
	}
	else if ( !memcmp(tok.p, "RMC", 3))
	{
		if(!memcmp(gsvhead, "GNRMC", 5))
		{
			nmeastate = NMEA_GNRMC;

            if(GNModeList[0] + GNModeList[1] == MODE_GPS + MODE_BD)
            {
 			   rmcmode=0x30;          
			}
			else if(GNModeList[0] + GNModeList[1] == MODE_GPS + MODE_GL)
			{

			   rmcmode=0x60;	

			}
			else if(GNModeList[0] + GNModeList[1] == MODE_BD + MODE_GL)
			{
				rmcmode=0x50;	
			}	


		}	
		else if(!memcmp(gsvhead, "BDRMC", 5)){
			nmeastate = NMEA_BDRMC;
			rmcmode=0x10;
		}	
		else if(!memcmp(gsvhead, "GPRMC", 5)){
			nmeastate = NMEA_GPRMC;
			rmcmode=0x20;
		}
		else if(!memcmp(gsvhead, "GLRMC", 5)){
		    nmeastate = NMEA_GLRMC;
		    rmcmode=0x40;
		}	

		pthread_mutex_lock(&TDMODEMutex);
		td_mode.nv_mode=rmcmode;
		pthread_mutex_unlock(&TDMODEMutex);

		
		if((r->sv_status_changed) == 1)
		{
					up_svstatus(r);
					r->sv_status_changed = 0;
		}
		else
		{
				ClearSv(r);
				up_svstatus(r);
		}



		Token  tok_time          = nmea_tokenizer_get(tzer, 1);
		Token  tok_fixStatus     = nmea_tokenizer_get(tzer, 2);
		//Token  tok_latitude      = nmea_tokenizer_get(tzer, 3);
		//Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer, 4);
		//Token  tok_longitude     = nmea_tokenizer_get(tzer, 5);
		//Token  tok_longitudeHemi = nmea_tokenizer_get(tzer, 6);
		Token  tok_speed         = nmea_tokenizer_get(tzer, 7);
		Token  tok_bearing       = nmea_tokenizer_get(tzer, 8);
		Token  tok_date          = nmea_tokenizer_get(tzer, 9);

		if (tok_fixStatus.p[0] == 'A')
		{
			nmea_reader_update_date( r, tok_date, tok_time );
			nmea_reader_update_bearing( r, tok_bearing );
			nmea_reader_update_speed  ( r, tok_speed );
		}else{
			r->fix.flags   &= (~GPS_LOCATION_HAS_BEARING);
			r->fix.flags   &= (~GPS_LOCATION_HAS_SPEED);			
		}

		///////////////////////////////////////////////////////GSA
	}
	else if ( !memcmp(tok.p, "GSA", 3) )
	{
		Token  tok_fixStatus   = nmea_tokenizer_get(tzer, 2);
		if((!memcmp(gsvhead, "GNGSA", 5))&&(nmeastate == NMEA_GNRMC))
		{
		    nmeastate = NMEA_1ST_GNGSA; 
            if(GNModeList[0] + GNModeList[1] == MODE_GPS + MODE_BD)
            {
			    gsvhead[1]='P';
             
			}
			else if(GNModeList[0] + GNModeList[1] == MODE_GPS + MODE_GL)
			{
				gsvhead[1]='P';

			}
			else if(GNModeList[0] + GNModeList[1] == MODE_BD + MODE_GL)
			{
				gsvhead[1]='L';

			}
		}

		if((!memcmp(gsvhead, "GNGSA", 5))&&(nmeastate == NMEA_1ST_GNGSA))
		{

			nmeastate = NMEA_2ND_GNGSA;
			
            if(GNModeList[0] + GNModeList[1] == MODE_GPS + MODE_BD)
            {
			    gsvhead[0]='B';
			    gsvhead[1]='D';           
			}
			else if(GNModeList[0] + GNModeList[1] == MODE_GPS + MODE_GL)
			{
				gsvhead[0]='G';
				gsvhead[1]='L';

			}
			else if(GNModeList[0] + GNModeList[1] == MODE_BD + MODE_GL)
			{
				gsvhead[0]='B';
				gsvhead[1]='D';

			}			
		}

		
		if (tok_fixStatus.p[0] != '\0' && tok_fixStatus.p[0] != '1')
		{
			int i;
			int prn;

			if((!memcmp(gsvhead, "GPGSA", 5)) && (GsaPrn.fixprnnum < GPS_MAX_SVS))
			{
				for(i = 3; i < 14; i++)
				{
					Token  tok_prn  = nmea_tokenizer_get(tzer, i);
					prn = str2int(tok_prn.p, tok_prn.end);

					if(prn > 0)
					{
						if(GsaPrn.fixprnnum >= GPS_MAX_SVS)
							break;

						GsaPrn.fixprn[GsaPrn.fixprnnum] = prn;
						if (GetDebugMaskBit(D_GSA) == 1)
						{
						D("GPGSA useprn=%d", GsaPrn.fixprn[GsaPrn.fixprnnum]);
						}
						GsaPrn.fixprnnum++;
					}
					else
						break;
				}
			}
			else if((!memcmp(gsvhead, "BDGSA", 5)) && (GsaPrn.fixprnnum < GPS_MAX_SVS))
			{
				for(i = 3; i < 14; i++)
				{
					Token  tok_prn  = nmea_tokenizer_get(tzer, i);
					prn = str2int(tok_prn.p, tok_prn.end);

					if(prn > 0)
					{
						if(GsaPrn.fixprnnum >= GPS_MAX_SVS)
							break;

						GsaPrn.fixprn[GsaPrn.fixprnnum] = prn + tdconfig.bdprnaddr;
						if (GetDebugMaskBit(D_GSA) == 1)
						{
						D("BDGSA useprn=%d", GsaPrn.fixprn[GsaPrn.fixprnnum]);
						}
						GsaPrn.fixprnnum++;
					}
					else
						break;
				}
			}
			else if((!memcmp(gsvhead, "GLGSA", 5)) && (GsaPrn.fixprnnum < GPS_MAX_SVS))
			{
				for(i = 3; i < 14; i++)
				{
					Token  tok_prn  = nmea_tokenizer_get(tzer, i);
					prn = str2int(tok_prn.p, tok_prn.end);

					if(prn > 0)
					{
						if(GsaPrn.fixprnnum >= GPS_MAX_SVS)
							break;

						GsaPrn.fixprn[GsaPrn.fixprnnum] = prn;
						if (GetDebugMaskBit(D_GSA) == 1)
						{
						D("GLGSA useprn=%d", GsaPrn.fixprn[GsaPrn.fixprnnum]);
						}
						GsaPrn.fixprnnum++;
					}
					else
						break;
				}
			}			
		}
	}
//	else if(!memcmp(gsvhead, "RUTXT", 5))
//	{
//	}
	else
	{
		tok.p -= 2;
	}
if (r->fix.flags & GPS_LOCATION_HAS_LAT_LONG)
{
		if (r->callback)
		{
			r->callback( &r->fix );//callback up fix
		}
	}
}

static void SDBP_Prase(int data)
{
  #define Maxlen 256
  static unsigned char ACKData[Maxlen] = {0};
  static unsigned int DataIndex = 0;
  char tmp_buff[Maxlen] = {0};
  char tmp[4] = {0};
  unsigned int i;
  unsigned int datalen;
  unsigned short crc16 = 0;
  //int ret = 0;

  for(i = 0; i < Maxlen - 1;i ++)
  {
    ACKData[i] = ACKData[i + 1];
  }
  ACKData[Maxlen - 1] = data;
  
  if(DataIndex > 0)DataIndex --;

  if(ACKData[Maxlen - 2] == 0x23 && ACKData[Maxlen - 1] == 0x3e)
  {
    DataIndex = Maxlen - 2;
	return;
  }


  if((DataIndex + 7)< Maxlen)
  {
    if(ACKData[DataIndex] == 0x23 && ACKData[DataIndex + 1] == 0x3e)
    {
        datalen = ACKData[DataIndex + 4] + (ACKData[DataIndex + 5]<<8) + 8;
        
		if(DataIndex + datalen - 1 < Maxlen)
		{
		  crc16 = ACKData[DataIndex + datalen - 2] + (ACKData[DataIndex + datalen - 1]<<8);
		  if(crc16 != FletCher16_sdbp(&ACKData[DataIndex + 2],0,datalen - 4))
		  {
		     return;
		  }
		}
		else return;
		
	}
	else return;

  }
  else return;

   

  if( ACKData[DataIndex + 2] == 0x05 &&  ACKData[DataIndex + 3] == 0x01)
  {

     snprintf(tmp_buff, datalen - 8 + 1,"%s",ACKData + DataIndex + 6);//

     D("SDBP : Ver %s",tmp_buff);
  }
  else 
  {
    	
    for(i = 0;i < datalen;i ++)
    {
      sprintf(tmp, "%02x ",ACKData[DataIndex + i]);
	  strcat(tmp_buff, tmp);
	}
    D("SDBP : %s",tmp_buff);
  }

  
	if(update_mode == 1)//update mode rx
	{
		update_td_handler_return(&ACKData[DataIndex]);
	}

  memset(&ACKData[DataIndex],0,datalen);
  DataIndex = 0;

}

static void
nmea_reader_addc( NmeaReader  *r, int  c )
{

	if (r->overflow)
	{
		r->overflow = (c != '\n');//restart from a new parase
		return;
	}

	//r->overflow
	if (r->pos >= (int) sizeof(r->in) - 1 )
	{
		r->overflow = 1;
		r->pos      = 0;
		return;
	}

	r->in[r->pos] = (char)c;
	r->pos       += 1;
	if (c == '\n' && r->in[r->pos - 2] == '\r')
	{
		
		if(NmeaCheck(r) == 0)
			nmea_reader_parse( r );

		r->pos = 0;
	}
}

static void origin_measure_deal(int c)
{
	static int pos = OG_MS_FRAME_HEAD_LEN;
	
	origin_data[pos] = (char)c;
	pos += 1;

	if (pos == (OG_MS_FRAME_HEAD_LEN + OG_MS_VALID_DATA_LEN))
	{
		//update_nmea(origin_data, origin_data + OG_MS_FRAME_LEN);
		update_origin_data((origin_data + OG_MS_FRAME_HEAD_LEN), OG_MS_VALID_DATA_LEN + OG_MS_FRAME_TAIL_LEN);

		#ifdef PRINT_ORIGIN_DATA
		print_long_data(origin_data + OG_MS_FRAME_HEAD_LEN, OG_MS_VALID_DATA_LEN);
		#endif
		pos = OG_MS_FRAME_HEAD_LEN;
		memset(origin_data + OG_MS_FRAME_HEAD_LEN, 0, OG_MS_VALID_DATA_LEN);
	}

}

void* tdchip_ack_timer_thread(void *param)
{
	time_t tm_old = 0;
	time_t tm_now = 0;

	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s(%d)", __FUNCTION__,(uint32_t)param);
	
	for( ; ; )
	{
		
		//while(cmd_to_chip_flag == 0)
		//{
			//sleep(2);
		//}
		
		pthread_cond_wait(&tdchip_ack_sync.cond, &tdchip_ack_sync.mutex);
		
		D("%s: start time", __FUNCTION__);
		tm_now = time(0);
		tm_old = tm_now;
		
		while(tm_now < (tm_old + TD_CMD_ACK_TIME_MAX))
		{
			sleep(1);
			tm_now = time(0);
		}
		
		cmd_to_chip_flag = 0;
		SetApUartbaud(tdconfig.uartboadrate);
		D("%s: %d sec pass", __FUNCTION__, TD_CMD_ACK_TIME_MAX);
	}
	
	D("exit %s", __FUNCTION__);
}

static void timer_for_tdchip_ack()
{
	int ret;
	pthread_attr_t attr;

	pthread_attr_init (&attr);
	ret = pthread_create(&tdchip_ack_timer_thread_id, &attr, tdchip_ack_timer_thread, NULL);

	if ( ret != 0 )
	{
		D("Could not create thread:tdchip_ack_timer_thread, ret=%d, errno=%d\n", ret, errno);
		return;
	}
}

static void do_tdchip_ack_frame_deal(const char *buff, const int len)
{
	char tmp[4] = {0};
	char tmp_buff[CMD_MAX_LEN] = {0};
	int i;
	
	if(len >= CMD_MAX_LEN / 3)
	{
		D("Cmd too long(len:%d)", len);
		return;
	}
	
	for(i = 0; i < len; i++)
	{
		sprintf(tmp, "%02x,", buff[i]);
		strcat(tmp_buff, tmp);
	}

	update_nmea(tmp_buff, tmp_buff + len * 3);
}

#if 1
static void tdchip_ack_frame_deal(char c)
{
	//D("%s:called", __FUNCTION__);
	static int pos = 0;
	int i;
	unsigned  short crc = 0;
	char ch = 0;
	char cl = 0;
	
	if(pos > CMD_MAX_LEN - 1)
		pos = 0;

	ack_buff[pos++] = c;
	if(pos >= 2)
	{
		if(ack_buff[pos - 2] == '\r' && ack_buff[pos - 1] == '\n')
		{
			if(pos < 5)
			{
				return;
			}
		
			if(ack_buff[0] == TD_SDBP_HEADER)
			{
				for(i = 0; i < pos; i++)
				{
					D("TDChip ACK[%d]:0x%x", i, ack_buff[i]);
				}
				
				crc = CalcCRC16(ack_buff, 1, pos - 5);
				ch = (crc >> 8) & 0xff;
				cl = crc & 0xff;
				if((ch == ack_buff[pos - 4]) && (cl == ack_buff[pos - 3]))
				{
					do_tdchip_ack_frame_deal(ack_buff, pos);
				}
				else
				{
					D("Ack frame CRC error");
				}
				//cmd_to_chip_flag = 0;
			}
			/*
			else
			{
				update_nmea(ack_buff, ack_buff + pos - 1);
			}
			*/
			memset(ack_buff, 0, CMD_MAX_LEN);
			pos = 0;
		}
	}
}
#else

static void tdchip_ack_frame_deal(char c)
{
	static int pos = 0;
	static int find_header_flag = 0;
	int i;

	if(pos > CMD_MAX_LEN - 1)
		pos = 0;

	ack_buff[pos++] = c;

	if(pos == 5)
	{
		if(ack_buff[0] == TD_SDBP_HEADER && ack_buff[1] == 0x16
			&& ack_buff[2] == 0xc0 && ack_buff[3] == 0x00 && ack_buff[4] == 0x00)
		{
			D("ack: find header");
			find_header_flag = 1;
		}
		else
		{
			find_header_flag = 0;

			for(i = 0; i < 4; i++)
			{
				ack_buff[i] = ack_buff[i + 1];
			}
			
			pos--;
		}
	}

	if(find_header_flag == 1)
	{
		if(c == '\n' && ack_buff[pos - 2] == '\r')
		{
			D("ack: find a frame");
			for(i = 0; i < pos; i++)
			{
				D("ack_buff[%d]:0x%x", i, ack_buff[i]);
			}
			update_nmea(ack_buff, ack_buff + (pos - 1));
			find_header_flag = 0;
			pos = 0;
		}
	}
}

#endif

void update_gps_status(GpsStatusValue value)
{
	// D("%s(): GpsStatusValue=%d", __FUNCTION__, value);
	GpsState  *state = _gps_state;
	state->status.status = value;

	if(state->callbacks.status_cb)
		state->callbacks.status_cb(&state->status);
}

static void
td_bdgps_state_done( GpsState  *s )
{
	update_gps_status(GPS_STATUS_ENGINE_OFF);

	char   cmd = CMD_QUIT;
	void  *dummy;
	write( s->control[0], &cmd, 1 );
	pthread_join(s->thread, &dummy);

	close( s->control[0] );
	s->control[0] = -1;
	close( s->control[1] );
	s->control[1] = -1;

	close( s->fd );
	s->fd = -1;
	s->init = 0;
}


static void
td_bdgps_state_start( GpsState  *s )
{
	// Navigation started.
	update_gps_status(GPS_STATUS_SESSION_BEGIN);//up gps navigation status

	char  cmd = CMD_START;
	int   ret;

	do
	{
		ret = write( s->control[0], &cmd, 1 );
	}
	while (ret < 0 && errno == EINTR);

	if (ret != 1)
		D("%s: could not send CMD_START command: ret=%d: %s",
		  __FUNCTION__, ret, strerror(errno));
}


static void
td_bdgps_state_stop( GpsState  *s )
{
	// Navigation ended.
	update_gps_status(GPS_STATUS_SESSION_END);

	char  cmd = CMD_STOP;
	int   ret;

	do
	{
		ret = write( s->control[0], &cmd, 1 );
	}
	while (ret < 0 && errno == EINTR);

	if (ret != 1)
		D("%s: could not send CMD_STOP command: ret=%d: %s",
		  __FUNCTION__, ret, strerror(errno));
}


static int
epoll_register( int  epoll_fd, int  fd )
{
	struct epoll_event  ev;
	int                 ret, flags;

	flags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	ev.events  = EPOLLIN;
	ev.data.fd = fd;

	do
	{
		ret = epoll_ctl( epoll_fd, EPOLL_CTL_ADD, fd, &ev );
	}
	while (ret < 0 && errno == EINTR);

	return ret;
}

/*
static int
epoll_deregister( int  epoll_fd, int  fd )
{
	int  ret;

	do
	{
		ret = epoll_ctl( epoll_fd, EPOLL_CTL_DEL, fd, NULL );
	}
	while (ret < 0 && errno == EINTR);

	return ret;
}*/

void update_gps_location(GpsLocation *location)
{
#if DUMP_DATA
	D("%s(): GpsLocation=%f, %f", __FUNCTION__, location->latitude, location->longitude);
#endif
	GpsState  *state = _gps_state;

	//Should be made thread safe...
	if(state->callbacks.location_cb)
		state->callbacks.location_cb(location);
}

void ChipUpdateResult(unsigned char flag)
{
	GpsState  *state = _gps_state;
	char buff[256] = {0};
	int percent;
	memset(buff, 0, sizeof(buff));
	GpsUtcTime system_time;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	system_time = (GpsUtcTime) tv.tv_sec * 1000 + tv.tv_usec / 1000;

	if(flag == 1)
	{
		strcpy(buff, "TD Chip Update Succeed");
		D("CallBack : %s",buff);
	}
	else if(flag == 2 )
	{
		int sync_now, sync_toall;
		update_td_get_sync(&sync_now, &sync_toall);

		if(sync_toall > 0)
		percent = 100 * sync_now / sync_toall;
		else percent = 0;
		
		sprintf(buff, "Percent:%d", percent);
		D("CallBack : %s",buff);
	}
	else
	{
		strcpy(buff, "TD Chip Update Failed");
		D("CallBack : %s",buff);
	}

	if(state->callbacks.nmea_cb != 0)
	{
		state->callbacks.nmea_cb(system_time, buff, strlen(buff));
	}
}

void update_bdgps_svstatus(GpsSvStatus *svstatus)  // callback of svstatus in _gps_state
{
	//if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);
	GpsState  *state = _gps_state;

	if(state->callbacks.sv_status_cb)
		state->callbacks.sv_status_cb(svstatus);
}
#if 0
void update_nmea(const char  *p, const char  *end )
{
	GpsState  *state = _gps_state;
	char buff[500];
	int i = 0;
	memset(buff, 0, sizeof(buff));
	memcpy(buff, p, (end - p));
	GpsUtcTime system_time;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	system_time = (GpsUtcTime) tv.tv_sec * 1000 + tv.tv_usec / 1000;
	//buff1[len]='\r';

#if TD_DEBUG_NMEA
	D("nmea:%s", buff);
#endif

	if(state->callbacks.nmea_cb)
	{
		state->callbacks.nmea_cb(system_time, buff, end - p);
	}
}
#endif

void update_nmea(const char  *p, const char  *end )
{
	GpsState  *state = _gps_state;
	char buff[500];
	memset(buff, 0, sizeof(buff));
	memcpy(buff, p, (end - p));
	GpsUtcTime system_time;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	system_time = (GpsUtcTime) tv.tv_sec * 1000 + tv.tv_usec / 1000;
	//buff1[len]='\r';

	if (GetDebugMaskBit(D_NMEA) == 1)
	{
	D("nmea:%s", buff);
    }

	if(state->callbacks.nmea_cb && update_mode == 0)
	{
		state->callbacks.nmea_cb(system_time, buff, end - p);
	}
}


#if 0
void write_agps_data(int fd)
{
	int i;
	int len;

	pthread_mutex_lock(&AgpsDataMutex);
	ehp_new = td_agps_info.ehp_new;
	pthread_mutex_unlock(&AgpsDataMutex);

	if((ehp_new.lines == 0) || (ehp_new.lines > 32))
		return;

	for(i = 0; i < ehp_new.lines; i++)
	{
		if((ehp_new.prn[i] != -1) && (ehp_new.prn[i] != 0))
		{
			len = strlen(ehp_new.line[i]);
			D("Agps:%s", ehp_new.line[i]);
			write(fd, ehp_new.line[i], len);
			usleep(50000);
		}
	}

	write(fd, CMD_AGPS_QUE, CMD_AGPS_QUE_LEN);

}
#endif
unsigned int CrcSum(char *s){
	unsigned int check_sum=0;
	int i,n;
	if(s==NULL)
		return -1;
	n= strlen(s);
	s++;
	for(i = 0; i < n-1; i++)
	{
		check_sum ^= (*s);
		s++;
	}
	return check_sum;
}

void write_agps_data(int fd)
{
	int i=0,cnt_w=0;
	//int len;
	//unsigned int temp_eph_mask;
	//time_t  time1,time2;
	unsigned char pack_sdbpbuf[73],pack_time[24];
		
	pthread_mutex_lock(&AgpsDataMutex);
	td_agps_data = td_agps_info.agps_data;
	pthread_mutex_unlock(&AgpsDataMutex);
	//temp_eph_mask = td_agps_data.ehp_new.eph_mask;
	//memset(&ap_chip_agps,0,sizeof(ap_chip_agps));
	//td_mode.work = 0;

	//usleep(500000);

	//PowerOn();
	UartTxData((unsigned char*)"agps",4);

//	ap_chip_agps.agps_get_status=TRUE;
	//ack=0;nck=0;
	if(td_agps_data.time_loc_new.mask>0){
		D("\n%s",td_agps_data.time_loc_new.line);
		D("\n%X\n",td_agps_data.ehp_new.eph_mask);
		if((pack_sdbp_time(td_agps_data.time_loc_new.line,pack_time))==-1)
			return;
		write(fd,pack_time,24);
		//for(i=0;i<24;i++)
		//	D("%X",pack_time[i]);
	}else{
		if(td_agps_data.ehp_new.eph_mask>0){
			pack_sdbp_time_local(pack_time);
			write(fd,pack_time,24);
		}
	}
		
	for(i=0;i<EPH_MAX_LINES;i++){
		if(((td_agps_data.ehp_new.eph_mask>>i) & 0x01) == 1)
			{
				//len = strlen(td_agps_data.ehp_new.line[i]);
				//D("AGPS:CEP-w:%s\n",td_agps_data.ehp_new.line[i]);
				memset(pack_sdbpbuf,0,sizeof(pack_sdbpbuf));
				//D("\n%s\n",td_agps_data.ehp_new.line[i]);
				pack_sdbp_agps(td_agps_data.ehp_new.line[i],pack_sdbpbuf);
				//if(cnt_w>17)
				/*{
					for(j=0;j<7;j++){
					D("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",pack_sdbpbuf[j*10+0],pack_sdbpbuf[j*10+1],pack_sdbpbuf[j*10+2],pack_sdbpbuf[j*10+3],pack_sdbpbuf[j*10+4],pack_sdbpbuf[j*10+5],pack_sdbpbuf[j*10+6],pack_sdbpbuf[j*10+7],pack_sdbpbuf[j*10+8],pack_sdbpbuf[j*10+9]);
					}				
					D("%02X %02X %02X\n",pack_sdbpbuf[70],pack_sdbpbuf[71],pack_sdbpbuf[72]);
				}*/
				pthread_mutex_lock(&UartWriteMutex);
				//write(fd, td_agps_data.ehp_new.line[i], len);
				write(fd,pack_sdbpbuf,73);
				usleep(10000);
				cnt_w++;
				//if(cnt_w>17)
					//break;
				//D("\n cnt_w =%d \n",cnt_w);
				pthread_mutex_unlock(&UartWriteMutex);
			}	
	}

}


/*
extern int com_fd;
bool CheckTdModuleVersion(int cnt)
{
	struct timespec timer;
	int ret = -1;
	static	int cnt_all=0;
	static int cnt_error=0;
	static int cnt_ok=0;
	unsigned char  cmdbuf[30];
	int wlen;
	//int i;
	td_mode.on_need=0;
	PowerOn();
	sleep(3);
	wlen = GetVersionCmd(cmdbuf, 0);	
	if( cnt > (QUE_TDVER_TIMES - 3) )
	{
		tdconfig.uartboadrate=BAUD_9600;
		SetApUartbaud(tdconfig.uartboadrate);	
	}
	D("set version mode cmd len %d\n", wlen);//wlen=11
	com_fd = _gps_state->fd;
	
	
	pthread_mutex_lock(&version_check_struct_lock);
	memset(&version_check, 0, sizeof(struct auto_update_version_check));
	version_check.state = AUTO_UPDATE_STATE_CHECK_VERSION;
	pthread_mutex_unlock(&version_check_struct_lock);
	
	clock_gettime(CLOCK_REALTIME, &timer);
	timer.tv_sec += 5;
	D("pthread start time=%d",(int)time(NULL));
	pthread_mutex_lock(&version_check.mutex);
	ret = TxData(cmdbuf, wlen);//comf_fd 
	cnt_all++;
	if(ret != wlen)
	{
		D("write cmd err%d", ret);
		ret = -1;
		//goto end;
	}
	ret = pthread_cond_timedwait(&version_check.cond, &version_check.mutex, &timer);
	D("pthread_cond_timedwait:ret=%d  time=%d",ret,(int)time(NULL));
	if(ret)
	{
		D("get version time out");
		ret = -1;
		cnt_error++;
		
		D("cnt_all:%d cnt_error:%d cnt_ok:%d",cnt_all,cnt_error,cnt_ok);
	}else{
		//tdconfig.cur_app_ver = version_check.ver.appver;
		tdconfig.querytdmodeinfo.boot_ver_maj = version_check.ver.boot_ver_maj;
		tdconfig.querytdmodeinfo.boot_ver_min = version_check.ver.boot_ver_min;
		tdconfig.querytdmodeinfo.app_ver_maj = version_check.ver.app_ver_maj;
		tdconfig.querytdmodeinfo.app_ver_min = version_check.ver.app_ver_min;
		tdconfig.querytdmodeinfo.app_ver_extend = version_check.ver.app_ver_extend;
		tdconfig.querytdmodeinfo.runstate = version_check.ver.runstate;
		D("que version,boot_maj:%d boot_min:%d app_maj:%d app_min:%d app_extend:%d runtype:%d",\
		tdconfig.querytdmodeinfo.boot_ver_maj, 
		tdconfig.querytdmodeinfo.boot_ver_min, 
		tdconfig.querytdmodeinfo.app_ver_maj,
		tdconfig.querytdmodeinfo.app_ver_min,
		tdconfig.querytdmodeinfo.app_ver_extend,
		tdconfig.querytdmodeinfo.runstate
		);
		if((tdconfig.tdsoftinfo.app_ver_maj!=version_check.ver.app_ver_maj)||\
			(tdconfig.tdsoftinfo.app_ver_min!=version_check.ver.app_ver_min)||\
			(tdconfig.tdsoftinfo.app_ver_extend!=version_check.ver.app_ver_extend))
			save_properties();
		cnt_ok++;
		D("cnt_all:%d cnt_error:%d cnt_ok:%d",cnt_all,cnt_error,cnt_ok);
	}
	pthread_mutex_unlock(&version_check.mutex);

    
	if(ret)
	{
		//tdconfig.cur_app_ver = 0;
		return TD_FALSE;
	}
	else
		return TD_TRUE;

}
*/
//end

int UartTxData(unsigned char *buffer, int length)
{
	int tx_len = 0;
	int com_fd = _gps_state->fd;
	if(com_fd != -1)
	{

		do
		{
			pthread_mutex_lock(&UartWriteMutex);
			tx_len = write( com_fd, buffer, length );
			pthread_mutex_unlock(&UartWriteMutex);
			//D("tx_len %d errno %d EINTR %d, EAGAIN %d b[2] %d b[3] %d.\n",tx_len, errno, EINTR, EAGAIN, buffer[2], buffer[3]);
		}
		while (tx_len < 0 && (errno == EINTR || errno == EAGAIN));

		return tx_len;
	}
	else
	{
		return 0;
	}
}


char og_ms_XOR(char *data, int startpoint, int endpoint)
{
	char sum = 0;
	int i;
	for(i = startpoint; i <= endpoint; i++)
	{
		sum ^= data[i];
	}

	return sum ;
}

void send_og_ms_cmd(int mode)
{
	char og_cmd[32] = {0};
	char *cmd_head = "$CCSUA,";
	int pos = 0;
	int checksum = 0;
	char cksum[3] = {0};
	int ret = 0;
	int len = 0;

	memcpy(og_cmd, cmd_head, strlen(cmd_head));
	pos = strlen(cmd_head);
	
	if(mode == 1)  //UART0 enter BIN
	{
		og_cmd[pos] = '3';
	}
	else if(mode == 2)  //UART0 enter NMEA
	{
		og_cmd[pos] = '1';
	}

	pos++;
	og_cmd[pos] = ',';
	pos++;
	og_cmd[pos] = '9';  //UART1 remain
	pos++;
	og_cmd[pos] = '*';

	checksum = og_ms_XOR(og_cmd, 1, pos - 1);
	sprintf(cksum, "%02X", checksum);

	og_cmd[pos + 1] = cksum[0];
	og_cmd[pos + 2] = cksum[1];
	og_cmd[pos + 3] = '\r';
	og_cmd[pos + 4] = '\n';
	
	len = pos + 5;
	ret = UartTxData((unsigned char *)og_cmd, len);
	if(ret != len)
	{
		D("write origin measure data err:%d", ret);
	}
	else
	{
		D("write og_ms success:%s", og_cmd);
	}
}
void update_origin_data(const char * data, const int data_len)
{
	int i;
	char tmp[4] = {0};
	int write_len = 0;
	//int ret;

	if(data_len >= 2048 / 2)   //rtcm_data_len max to 38459
	{
		D("Data too long:%d Byte", data_len);
		return;
	}

	for(i = 0; i < data_len; i++)
	{
		sprintf(tmp, "%02X", data[i]);
		strcat(origin_data_tmp + OG_MS_FRAME_HEAD_LEN, tmp);
	}	

	write_len = OG_MS_FRAME_HEAD_LEN + data_len * 2;

#ifdef WRITE_ORIGIN_SDCARD
	ret = write(glb_og_fd, origin_data_tmp, write_len);
	D("write num:%d, return:%d", write_len, ret);
#else
	update_nmea(origin_data_tmp, origin_data_tmp + write_len);
#endif
	
	memset(origin_data_tmp + OG_MS_FRAME_HEAD_LEN, 0, write_len);
}

#ifdef PRINT_LONG_DATA
void print_long_data(const char * rtcm_data, const int rtcm_data_len)
{

	int i = 0, m = 0;
	char tmp[4] = {0};

	if(rtcm_data_len >= 2048)   //rtcm_data_len max to 38459
	{
		D("Data too long:%d Byte", rtcm_data_len);
		return;
	}
	if(rtcm_data_len > MAX_PRINTF)
	{
		for(m = 0; m < rtcm_data_len / MAX_PRINTF; m++)
		{
			for(i = m * MAX_PRINTF; i < (m * MAX_PRINTF + MAX_PRINTF); i++)
			{
				sprintf(tmp, "%02X ", rtcm_data[i]);
				strcat(rtcm_tmp, tmp);
			}
			D("write rtcm data %d:%s", m, rtcm_tmp);
			memset(rtcm_tmp, 0, 2048);
		}

		for(i = m * MAX_PRINTF; i < rtcm_data_len; i++)
		{
			sprintf(tmp, "%02X ", rtcm_data[i]);
			strcat(rtcm_tmp, tmp);
		}
		D("write rtcm data %d:%s", m, rtcm_tmp);
		memset(rtcm_tmp, 0, 2048);

	}
	else
	{
		for(i = 0; i < rtcm_data_len; i++)
		{
			sprintf(tmp, "%02X ", rtcm_data[i]);
			strcat(rtcm_tmp, tmp);
		}
		
		D("write rtcm data:%s", rtcm_tmp);
	}
	memset(rtcm_tmp, 0, 2048);		
}
#endif

#ifdef WRITE_ORIGIN_SDCARD
static int open_file()
{
	
	int og_fd;
	time_t now;
	struct tm *tnow;
	char og_name[64] = {0};
	
	now = time(0);
	tnow = localtime(&now);
	//D("time:%d %d %d %d %d %d\n", 1900+tnow->tm_year,
		   //tnow->tm_mon+1,tnow->tm_mday,tnow->tm_hour,tnow->tm_min,tnow->tm_sec);
	sprintf(og_name, "/data/origin_%04d%02d%02d_%02d-%02d-%02d.txt", 1900+tnow->tm_year,
	tnow->tm_mon+1,tnow->tm_mday,tnow->tm_hour,tnow->tm_min,tnow->tm_sec);
	og_fd = open(og_name, O_RDWR | O_CREAT, 0777);
	if(og_fd < 0)
		D("open %s error", og_name);
	else
	{
		D("open %s success", og_name);
		glb_og_fd = og_fd;
	}

	return og_fd;
}
#endif

//end



/**
   *@brief The main thread always run
   *@param arg: State of our connection to the hardware_gpsd daemon
   *@retval None
   */
static void
gps_state_thread( void  *arg )
{
	GpsState   *state = (GpsState *) arg;
	NmeaReader  reader[1];
	int         epoll_fd   = epoll_create(2);
	int         started    = 0;
	int         gps_fd     = state->fd;
	int         control_fd = state->control[1];
	int  nn, ret;
	char  buff[32];
	static int cnt=0;
	int wret = 0;
	//static int ack=0;
	nmea_reader_init( reader );
	memset(&GsaPrn, 0, sizeof(GsaPrn));
  memset(fixed,0,sizeof(fixed));

	// register control file descriptors for polling
	epoll_register( epoll_fd, control_fd );
	epoll_register( epoll_fd, gps_fd );

	D("Enter : %s",__FUNCTION__);

	// now loop
	for (;;)
	{
		struct epoll_event   events[2];
		int                  ne, nevents;

		nevents = epoll_wait( epoll_fd, events, 2, -1 );

		if (nevents < 0)
		{
			if (errno != EINTR)
				D("epoll_wait() unexpected error: %s", strerror(errno));

			continue;
		}

		//D("gps thread received %d events", nevents);
		for (ne = 0; ne < nevents; ne++)
		{
			reader[0].sv_status.size = sizeof(GpsSvStatus);
			if ((events[ne].events & (EPOLLERR | EPOLLHUP)) != 0)
			{
				D("EPOLLERR or EPOLLHUP after epoll_wait() !?");
				return;
			}

			if ((events[ne].events & EPOLLIN) != 0)
			{
				int  fd = events[ne].data.fd;

				if (fd == control_fd)
				{
					char  cmd = 255;
					int   ret;
					D("Event : gps control fd event %d",fd);

					do
					{
						ret = read( fd, &cmd, 1 );
					}
					while (ret < 0 && errno == EINTR);

					if (cmd == CMD_QUIT)
					{
						D("CMD : Td module quitting on demand");
						return;
					}
					else if (cmd == CMD_START)
					{
						if (!started)
						{
							D("CMD : Td module starting navigation");
						  memset(fixed,0,sizeof(fixed));
							started = 1;
							nmea_reader_set_callback( reader, state->callbacks.location_cb);
						}
					}
					else if (cmd == CMD_STOP)
					{
						if (started)
						{
						   if(tdconfig.power_flag != TD_OFF)
						   {
							PowerOff();
							D("CMD : Td module stopping  Power off");
						   }
							tdconfig.power_flag = TD_OFF;
							
							started = 0;
							nmea_reader_set_callback( reader, NULL );
						}
					}
					else if(cmd == CMD_ENABLE_GPS)//
					{
						D("CMD : CMD_ENABLE_GPS");					
						memset(fixed,0,sizeof(fixed));
						pthread_mutex_lock(&UartWriteMutex);
						write( gps_fd, CMD_GPS_HOT, SET_CMD_TDMODULE_LEN);						
						pthread_mutex_unlock(&UartWriteMutex);
						tdconfig.gnssflag = RECV_GNSS_OFF;
						tdconfig.position_mode = MODE_GPS;
						tdconfig.last_gsv = RECV_NO;
						memset(reader->sv_status.sv_list, 0, sizeof(reader->sv_status.sv_list));
					}					
					else if(cmd == CMD_ENABLE_BD)//
					{
						D("CMD : CMD_ENABLE_BDS");				
						memset(fixed,0,sizeof(fixed));
						pthread_mutex_lock(&UartWriteMutex);
						write( gps_fd, CMD_BD_HOT, SET_CMD_TDMODULE_LEN);					
						pthread_mutex_unlock(&UartWriteMutex);
						tdconfig.gnssflag = RECV_GNSS_OFF;
						tdconfig.position_mode = MODE_BD;
						tdconfig.last_gsv = RECV_NO;
						memset(reader->sv_status.sv_list, 0, sizeof(reader->sv_status.sv_list));
					}
					else if(cmd == CMD_ENABLE_GL)//
					{
						D("CMD : CMD_ENABLE_GLO");				
						memset(fixed,0,sizeof(fixed));
						pthread_mutex_lock(&UartWriteMutex);
						write( gps_fd, CMD_GL_HOT, SET_CMD_TDMODULE_LEN);					
						pthread_mutex_unlock(&UartWriteMutex);
						tdconfig.gnssflag = RECV_GNSS_OFF;
						tdconfig.position_mode = MODE_GL;
						tdconfig.last_gsv = RECV_NO;
						memset(reader->sv_status.sv_list, 0, sizeof(reader->sv_status.sv_list));
					}					
					else if(cmd == CMD_ENABLE_BD_GP)//
					{
						D("CMD : CMD_ENABLE_BDS_GPS");
						memset(fixed,0,sizeof(fixed));
						pthread_mutex_lock(&UartWriteMutex);
						write( gps_fd, CMD_BD_GP_HOT, SET_CMD_TDMODULE_LEN);					
						pthread_mutex_unlock(&UartWriteMutex);
						tdconfig.gnssflag = RECV_BD_GP_ON;
						tdconfig.last_gsv = RECV_NO;
						tdconfig.position_mode = MODE_BD_GPS;
						memset(reader->sv_status.sv_list, 0, sizeof(reader->sv_status.sv_list));
					}
					else if(cmd == CMD_ENABLE_GP_GL)//
					{
						D("CMD : CMD_ENABLE_GPS_GLO");
						memset(fixed,0,sizeof(fixed));
						pthread_mutex_lock(&UartWriteMutex);
						write( gps_fd, CMD_GP_GL_HOT, SET_CMD_TDMODULE_LEN);					
						pthread_mutex_unlock(&UartWriteMutex);
						tdconfig.gnssflag = RECV_GP_GL_ON;
						tdconfig.last_gsv = RECV_NO;
						tdconfig.position_mode = MODE_GP_GL;
						memset(reader->sv_status.sv_list, 0, sizeof(reader->sv_status.sv_list));
					}	
					else if(cmd == CMD_ENABLE_BD_GL)//
					{
						D("CMD : CMD_ENABLE_BDS_GLO");
						memset(fixed,0,sizeof(fixed));
						pthread_mutex_lock(&UartWriteMutex);
						write( gps_fd, CMD_BD_GL_HOT, SET_CMD_TDMODULE_LEN);					
						pthread_mutex_unlock(&UartWriteMutex);
						tdconfig.gnssflag = RECV_BD_GL_ON;
						tdconfig.last_gsv = RECV_NO;
						tdconfig.position_mode = MODE_BD_GL;
						memset(reader->sv_status.sv_list, 0, sizeof(reader->sv_status.sv_list));
					}
					else if(cmd == GPS_COLD)
					{
						D("CMD : GPS_COLD");
						memset(fixed,0,sizeof(fixed));
											
						pthread_mutex_lock(&UartWriteMutex);
						write( gps_fd, CMD_GPS_COLD, SET_CMD_TDMODULE_LEN);						
						pthread_mutex_unlock(&UartWriteMutex);
						tdconfig.gnssflag = RECV_GNSS_OFF;
						tdconfig.position_mode = MODE_GPS;
						tdconfig.last_gsv = RECV_NO;
						memset(reader->sv_status.sv_list, 0, sizeof(reader->sv_status.sv_list));
					}					
					else if(cmd == GL_COLD)
					{
						D("CMD : GLO_COLD");
						memset(fixed,0,sizeof(fixed));
					
						pthread_mutex_lock(&UartWriteMutex);
						write( gps_fd, CMD_GL_COLD, SET_CMD_TDMODULE_LEN);						
						pthread_mutex_unlock(&UartWriteMutex);
						tdconfig.gnssflag = RECV_GNSS_OFF;
						tdconfig.position_mode = MODE_GL;
						tdconfig.last_gsv = RECV_NO;
						memset(reader->sv_status.sv_list, 0, sizeof(reader->sv_status.sv_list));
					}
					
					else if(cmd == BD_COLD)
					{
						D("CMD : BD_COLD");
						memset(fixed,0,sizeof(fixed));
					
						pthread_mutex_lock(&UartWriteMutex);
						write( gps_fd, CMD_BD_COLD, SET_CMD_TDMODULE_LEN);						
						pthread_mutex_unlock(&UartWriteMutex);
						tdconfig.gnssflag = RECV_GNSS_OFF;
						tdconfig.position_mode = MODE_BD;
						tdconfig.last_gsv = RECV_NO;
						memset(reader->sv_status.sv_list, 0, sizeof(reader->sv_status.sv_list));
					}
					else if(cmd == BD_GL_COLD)
					{
						D("CMD : BDS_GLO_COLD");
						memset(fixed,0,sizeof(fixed));
					
						pthread_mutex_lock(&UartWriteMutex);
						write( gps_fd, CMD_BD_GL_COLD, SET_CMD_TDMODULE_LEN);						
						pthread_mutex_unlock(&UartWriteMutex);
						tdconfig.gnssflag = RECV_BD_GL_ON;
						tdconfig.position_mode = MODE_BD_GL;
						tdconfig.last_gsv = RECV_NO;
						memset(reader->sv_status.sv_list, 0, sizeof(reader->sv_status.sv_list));
					}
					else if(cmd == BD_GP_COLD)
					{
						D("CMD : BDS_GPS_COLD");
						memset(fixed,0,sizeof(fixed));
					
						pthread_mutex_lock(&UartWriteMutex);
						write( gps_fd, CMD_BD_GP_COLD, SET_CMD_TDMODULE_LEN);						
						pthread_mutex_unlock(&UartWriteMutex);
						tdconfig.gnssflag = RECV_BD_GP_ON;
						tdconfig.position_mode = MODE_BD_GPS;
						tdconfig.last_gsv = RECV_NO;
						memset(reader->sv_status.sv_list, 0, sizeof(reader->sv_status.sv_list));
					}
					else if(cmd == GP_GL_COLD)
					{
						D("CMD : GPS+GLO_COLD");
						memset(fixed,0,sizeof(fixed));
					
						pthread_mutex_lock(&UartWriteMutex);
						write( gps_fd, CMD_GP_GL_COLD, SET_CMD_TDMODULE_LEN);						
						pthread_mutex_unlock(&UartWriteMutex);
						tdconfig.gnssflag = RECV_GP_GL_ON;
						tdconfig.position_mode = MODE_GP_GL;
						tdconfig.last_gsv = RECV_NO;
						memset(reader->sv_status.sv_list, 0, sizeof(reader->sv_status.sv_list));
					}					
					else if(cmd == CMD_START_AGPS)
					{
						D("CMD : CMD_START_AGPS");
					   StartAgps();
					}
					else if(cmd == CMD_AGPS)
					{
						if (started)
						{
						  D("CMD : CMD_AGPS");
						  pthread_mutex_lock(&QueagpsMutex);
						  write_agps_data(gps_fd);
						  pthread_mutex_unlock(&QueagpsMutex);
						}
					}
					else if(cmd == CMD_UPDATE_TD)
					{
						D("CMD : CMD_UPDATE_TD update_td_handler");
						Creat_update_thread();
						update_mode = 1;
					}else if(cmd == CMD_UPDATE_RTCM)
					{
						D("CMD : CMD_UPDATE_RTCM:write rtcm data");
						
						D("rtcm_data = 0x%x, rtcm_data_len = %d.", (unsigned int)rtcm_data, rtcm_data_len);
						//if(rtcm_data)
						{
							pthread_mutex_lock(&UartWriteMutex);
							wret = write( gps_fd, rtcm_data, rtcm_data_len);
							pthread_mutex_unlock(&UartWriteMutex);
							D("write rtcm data,return %d", wret);	

							#ifdef PRINT_RTCM_DATA
							print_long_data(rtcm_data, rtcm_data_len);
							#endif
						}
																		
					}
					else if(cmd == CMD_ORIGIN_OFF)
					{
						D("CMD : CMD_ORIGIN_OFF");
						send_og_ms_cmd(2);
						origin_flag = 0;						
					}
					else if(cmd == CMD_ORIGIN_ON)
					{
						D("CMD : CMD_ORIGIN_ON");
						#ifdef WRITE_ORIGIN_SDCARD
						open_file();
						#endif
						send_og_ms_cmd(1);
						origin_flag = 1;				
					}
					else if(cmd == CMD_SENT_TO_CHIP)
					{
						D("CMD : SENT_CMD_TO_CHIP");
												
						pthread_mutex_lock(&UartWriteMutex);
						wret = write( gps_fd, cmd_buff, cmd_length);
						pthread_mutex_unlock(&UartWriteMutex);

						//judge_cmd_change_baud();
					}
				}
				else if (fd == gps_fd)
				{
					//for (;;)
					{
						ret = read( fd, buff, sizeof(buff) );
						int pt=0;
						if(buff[0]==0x40){
							for(pt=0;pt<ret;pt++)
								D("src buff[%d]=%X",pt,buff[pt]);
						}
						if(ret>0)
						{
						    for(nn = 0; nn < ret; nn++)
							{
                              SDBP_Prase(buff[nn]);
							} 

						}
						if (ret < 0)
						{	cnt++;
							if(cnt<6)
								D("ret%d",ret);
							if (errno == EINTR)
								continue;
							if (errno != EWOULDBLOCK)
								D("error while reading from gps daemon socket: %s:", strerror(errno));
							break;
						}

						if(origin_flag == 1)
						{
							for (nn = 0; nn < ret; nn++)
								origin_measure_deal(buff[nn]);
						}
						else if(cmd_to_chip_flag == 1)
						{
							//tdchip_ack_frame_deal(buff, ret);
							for (nn = 0; nn < ret; nn++)
							{
								//D("TDchip ACK[%d]:0x%x", nn, buff[nn]);
								tdchip_ack_frame_deal(buff[nn]);
							}
						}
						else
						{
//							if(td_mode.on_need == 0){
//								//tcflush(fd,TCIFLUSH);
//								//D("%s\n",buff);
//								if(poweroff_flag==1){
//									PowerOff();
//									poweroff_flag=0;
//								}	
//							}
							for (nn = 0; nn < ret; nn++)
							{
							  nmea_reader_addc( reader, buff[nn] );
							}

						}
					}
					//D("gps fd event end");
				}
				else
				{
					D("epoll_wait() returned unkown fd %d ?", fd);
				}
			}
		}
	}
}










static void
gps_state_init( GpsState  *state, GpsCallbacks *callbacks )
{
	//int ret = -1;
	static int isInitialized = 0;
	state->init       = 1;
	state->control[0] = -1;
	state->control[1] = -1;
	tTD_BOOL result;
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);
	if( (callbacks != NULL) && (callbacks->size == sizeof(GpsCallbacks)) )
	{
		state->callbacks = *callbacks;
	}
	else
	{
		D("Error : gps_state_init Callback Function Error");
		return;
	}

	if(isInitialized == 1)
	{
		D(" GPS/BD has been initialized!!!");
		return;
	}
	isInitialized=1;
	//gpsStatus.size = sizeof(GpsStatus);
	//locationInfo.size = sizeof(GpsLocation);
	//niNotification.size = sizeof(GpsNiNotification);
	//agpsStatus.size = sizeof(AGpsStatus);

	if ( socketpair( AF_LOCAL, SOCK_STREAM, 0, state->control ) < 0 )
	{
		D("could not create thread control socket pair: %s", strerror(errno));
		goto Fail;
	}

	NetAccess = TD_FALSE;

	memset(&TdCellID, 0, sizeof(NetCellID));
	memset(&TdIdInfo, 0, sizeof(IdSetInfo));

	state->thread = callbacks->create_thread_cb( "gps_state_thread", gps_state_thread, state );
	result = td_bd_agps_init();
	

	
	if(result > 0 )
	{
		D("Succeed : agps init");
	}
	else
		D("Error : agps init fail");

	if ( !state->thread )
	{
		D("Error : Fail to create gps thread: %s", strerror(errno));
		goto Fail;
	}

	if(tdchip_ack_timer_thread_id == 0)
	{
		timer_for_tdchip_ack();
	}
	
	return;

Fail:
	td_bdgps_state_done( state );
}
/*
void VccOn(void)
{
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);
	int cts_fd;
	cts_fd = open(tdconfig.strconfigfile.cts, O_RDWR);
	if(cts_fd < 0)
	{
		D("Cts driver open fail !!!");
		return;
	}
	ioctl(cts_fd, VCC_ON, 0);
	close(cts_fd);
	return;
}

void VccOff(void)
{
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);
	int cts_fd;
	cts_fd = open(tdconfig.strconfigfile.cts, O_RDWR);
	if(cts_fd < 0)
	{
		D("Cts driver open fail !!!");
		return;
	}
	ioctl(cts_fd, VCC_OFF, 0);
	close(cts_fd);
	return;
}
*/

void PowerOn(void)
{
	D("Enter Normal WorkMode");
	start = time(NULL);
	UartTxData((unsigned char*)"on",2);
	usleep(150000);
	
	UartTxData(fix_dop100,15);
	tdconfig.power_flag = TD_ON;
}



void PowerOff(void)
{
	
	pthread_mutex_lock(&QueagpsMutex);
//	memset(&ap_chip_agps,0,sizeof(ap_chip_agps));
	pthread_mutex_unlock(&QueagpsMutex);
	if(update_mode==0){


		if(tdconfig.power_flag == TD_ON)
		{
		  D("Enter Standby WorkMode");
		  UartTxData(chip_standby,9);
		  tdconfig.power_flag = TD_OFF;
		}
	}else
		D(" TD chip is updateing not poweroff");
}




static int
td_bdgps_init(GpsCallbacks *callbacks)
{

	GpsState  *s = _gps_state;

	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);

	if (!s->init)
		gps_state_init(s, callbacks);

	if (s->fd < 0)
		return -1;

	return 0;
}

static void
td_bdgps_cleanup(void)
{
	GpsState  *s = _gps_state;

	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);

	char  cmd = CMD_STOP;
	int   ret;
	if(update_mode==1)
		return;
	do
	{
		ret = write( s->control[0], &cmd, 1 );
	}
	while (ret < 0 && errno == EINTR);

	/*
	    if (s->init)
	        gps_state_done(s);*/
}
static void StartAgps(void)
{
    RequestSetId();
	RequestRefLocation();
}

static int
td_bdgps_start()
{


	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);

	GpsState  *s = _gps_state;
	PowerOn();
	StartAgps();
	
	if (!s->init)
	{
		D("%s: called with uninitialized state !!", __FUNCTION__);
		return -1;
	}
	td_bdgps_state_start(s);
	return 0;
}


static int
td_bdgps_stop()
{

	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);

	GpsState  *s = _gps_state;
	td_mode.on_need=0;
	if (!s->init)
	{
		if (GetDebugMaskBit(D_FUCTION) == 1)
		{
		D("%s: called with uninitialized state !!", __FUNCTION__);
		}
		return -1;
	}

	td_bdgps_state_stop(s);
	PowerOff();
	return 0;
}


static int
td_bdgps_inject_time(GpsUtcTime time, int64_t timeReference, int uncertainty)
{
	if(tdconfig.power_flag = TD_ON)
	{
  	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s(%d,%lld,%d)", __FUNCTION__,(uint32_t)time,timeReference,uncertainty);
  
  	int gps_fd = _gps_state->fd;
  	pthread_mutex_lock(&QueagpsMutex);
  	write_agps_data(gps_fd);
  	pthread_mutex_unlock(&QueagpsMutex);
	}
	return 0;
}

static int
td_bdgps_inject_location(double latitude, double longitude, float accuracy)
{
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s(%f,%f,%f)", __FUNCTION__,latitude,longitude,accuracy);

	int gps_fd = _gps_state->fd;
	unsigned char pack_loc[24];
	pack_sdbp_location(latitude,longitude,accuracy,pack_loc);

	pthread_mutex_lock(&QueagpsMutex);
	write(gps_fd,pack_loc,24);
	write_agps_data(gps_fd);
	pthread_mutex_unlock(&QueagpsMutex);
	return 0;
}

static void
td_bdgps_delete_aiding_data(GpsAidingData flags)
{
    (void)flags;
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);
	//unsigned int f=0;
	//f =flags;
	//D("flags:0x%x",flags);
	//GpsState *s = (GpsState *)_gps_state;
	//char  cmd = BD_GP_COLD;
	//int   result;
	//hot start no call this function
	// warm start flag:0x01
	// cold start flag:0xffff
	
	UartTxData(cold_start,9);
	usleep(150000);
	//pthread_mutex_lock(&AgpsDataMutex);
	//memset(&td_agps_info.agps_data,0,sizeof(struct supl_agps_data));
	//pthread_mutex_unlock(&AgpsDataMutex);
	/*do
	{
		result = write(s->control[0], &cmd, 1 );
	}
	while (result < 0 && errno == EINTR);
	
	if (result != 1)
			D("could not send CMD_GN_COLD command");
	if(td_mode.agps_on==1){	
		sleep(1);
		RequestRefLocation();
	}*/	
	
	/*if(s->fd>0){
		sleep(1);
		pthread_mutex_lock(&UartWriteMutex);
		write(s->fd,CMD_GN_COLD,SET_CMD_TDMODULE_LEN);
		pthread_mutex_unlock(&UartWriteMutex);
		pthread_mutex_lock(&AgpsDataMutex);
		memset(&td_agps_data,0,sizeof(td_agps_data));
		pthread_mutex_unlock(&UartWriteMutex);
	}*/
	return;
}

static int td_bdgps_set_position_mode(GpsPositionMode mode, GpsPositionRecurrence recurrence,
									  unsigned int min_interval, unsigned int preferred_accuracy, unsigned int preferred_time)
{
	char cmd = 0x00;
	int ret ;
	GpsState  *s = _gps_state;
	int t_mode=0;
	//static unsigned int setmode=0;
	time_t  time1,time2;
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);
	D("Set : Fix Mode:0x%X %d %d %d %d", mode,recurrence,min_interval,preferred_accuracy,preferred_time);

	time_agps=0;

	td_mode.on_need=1;
	td_mode.work=0;
	
	//PowerOn();
	UartTxData((unsigned char*)"mode",4);
	UartTxData(fix_dop100,15);

	time1 = time(NULL);
	if(update_mode == 1)
		return 0;

	
	while(td_mode.work==0)
	{
	    //PowerOn();
		usleep(200000);
		time2 = time(NULL);
		if((time2-time1)>5){
			D("Error : TD module no work break!!!");
			break;
		}	
	}

	
	pthread_mutex_lock(&TDMODEMutex);
	t_mode = td_mode.nv_mode;
	pthread_mutex_unlock(&TDMODEMutex);
//	memset(&ap_chip_agps,0,sizeof(ap_chip_agps));
	
    if(t_mode==(int)mode)
	{
		D("Warning : POSITION MODE SET ALREADY");
		return 0;
	}
	switch(mode&0xff)
	{
	 case 0x02:cmd = CMD_START_AGPS;break;
	 	
     case 0x10:cmd = CMD_ENABLE_BD;break;
	 case 0x20:cmd = CMD_ENABLE_GPS;break;
	 case 0x30:cmd = CMD_ENABLE_BD_GP;break;
	 case 0x40:cmd = CMD_ENABLE_GL;break;
	 case 0x50:cmd = CMD_ENABLE_BD_GL;break;
	 case 0x60:cmd = CMD_ENABLE_GP_GL;break;
	 
	 case 0x11:cmd = BD_COLD;break;
	 case 0x21:cmd = GPS_COLD;break;
	 case 0x31:cmd = BD_GP_COLD;break;
	 case 0x41:cmd = GL_COLD;break;
	 case 0x51:cmd = BD_GL_COLD;break;
	 case 0x61:cmd = GP_GL_COLD;break;

	 default:cmd = 0x00;D("Error : Undefined mode!!!");break;




	}

	if(cmd != 0x00)
	{
		do
		{
			ret = write( s->control[0], &cmd, sizeof(cmd) );
		}
		while (ret < 0 && errno == EINTR);
	}

	return 0;
}

static int td_set_update_mode(void)
{
	char cmd;
	int ret ;
	GpsState  *s = _gps_state;

	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);

	//int i = 0;

	cmd = CMD_UPDATE_TD;

	do
	{
		//D("CMD UPDATE");
		ret = write( s->control[0], &cmd, sizeof(cmd) );
	}
	while (ret < 0 && errno == EINTR);

	return 0;
}


static int td_update_rtcm(char *data, int length )
{
	char cmd;
	int ret ;
	GpsState  *s = _gps_state;

	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);

	//int i = 0;

	if(length > MAX_RTCM_DATA_LEN)
		return 0;

	rtcm_data_len = length;
	
	memset(rtcm_data, 0, MAX_RTCM_DATA_LEN);
	memcpy(rtcm_data, data, length);

	cmd = CMD_UPDATE_RTCM;

	do
	{
		//D("CMD UPDATE");
		ret = write( s->control[0], &cmd, sizeof(cmd) );
	}
	while (ret < 0 && errno == EINTR);

	return 0;
}

bool  CheckImgData(const unsigned char *filedatas, const int startpoint, const int filelen)
{
	if(filedatas == NULL || startpoint < 0 || filelen < IMGFILE_HEADREAR_LEN)
		return FALSE;

	if(filedatas[startpoint] != 0x14 || filedatas[startpoint + 1] != 0x56 || filedatas[startpoint + 2] != 0x10  || filedatas[startpoint + 3] != 0x70)
	{	
		return FALSE;
	}
	
	
	return TRUE;
}

bool CheckSDBPData(const unsigned char *pdata,const int len)
{
  int i;
  //least sdbp data lenth is 8
  if(len < 8)return FALSE;

  for(i = 0;i < len - 8 + 5;i ++)
  {
      if(pdata[i] == 0x23 && pdata[i + 1] == 0x3e)
      {	 
        if(pdata[i + 2] == 0x02 && pdata[i + 3] == 0x04)
        {
            tdconfig.power_flag = TD_OFF;
			D("Warning : Send Standby Mode CMD");
		}
		
        return TRUE;
	  }
  }

   return FALSE;
}

bool CheckStringCmd(const unsigned char *pdata,const int len)
{
  //int i;

  if(pdata[0] == '$' && pdata[len - 1] == '\n')
  {
    return TRUE;
  }
  else
   return FALSE;
}

/*
static int og_ms_cmd_send(char cmd)
{
	int ret ;
	GpsState  *s = _gps_state;

	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);

	//int i = 0;

	do
	{
		//D("CMD UPDATE");
		ret = write( s->control[0], &cmd, sizeof(cmd) );
	}
	while (ret < 0 && errno == EINTR);

	return 0;
}*/
/*
static void og_ms_cmd_deal(const char *data, const int length)
{
	char cmd[OG_MS_CMD_LEN] = {0};
	unsigned int mode = 0;
	
	memcpy(cmd, data, length);
		
	if(cmd[0] == 0xa5 && cmd[1] == 0x5a)
	{
		mode = (cmd[2] << 8) | cmd[3];
		if(mode == 0x01)
		{
			og_ms_cmd_send(CMD_ORIGIN_ON);
		}
		else if(mode == 0x02)
		{
			og_ms_cmd_send(CMD_ORIGIN_OFF);
		}
	}
	
}*/
/*
static void judge_cmd_change_baud(void)
{
	if(change_baud_9600_flag == 0)
		return;
	
	if(cmd_buff[0] == TD_SDBP_HEADER)
	{
		if(cmd_buff[1] == 0x30)  //boot mode
		{
			sleep(1);
			D("%s:%d", __FUNCTION__, 9600);
			SetApUartbaud(9600);
		}
		else if(cmd_buff[1] == 0x34) //reboot
		{
			sleep(1);
			D("%s:%d", __FUNCTION__, tdconfig.uartboadrate);
			SetApUartbaud(tdconfig.uartboadrate);
		}
	}
}*/

static void send_cmd_to_TDchip(char cmd)
{
		int ret ;
		GpsState  *s = _gps_state;

		if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);

	
		do
		{
			ret = write( s->control[0], &cmd, sizeof(cmd) );
		}
		while (ret < 0 && errno == EINTR);
	
		return ;
}

static void command_handler(const char *data, const int length)
{
	//int i = 0;
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);

	memset(cmd_buff,0,CMD_MAX_LEN);

	if(length < CMD_MAX_LEN)
	{
	memcpy(cmd_buff, data, length);
	cmd_length = length;
	}
	else
	{
	D("length > CMD_MAX_LEN %d",length);
	memcpy(cmd_buff, data, CMD_MAX_LEN);
	cmd_length = CMD_MAX_LEN;
	}
    send_cmd_to_TDchip(CMD_SENT_TO_CHIP);

}


int ReadImg(char *fp,unsigned char ** file_data)
{ 
  int file_len = 0;
  get_img_file_data(fp, file_data, &file_len);
  return file_len;
}


int  bd_xtra_init( GpsXtraCallbacks *callbacks )
{
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);
	if(callbacks == 0)return 0;
	return 0;
}
////////////////////////////////////////////////
//
//APK send data to chip 1030
////////////////////////////////////////////////////
int  bd_inject_xtra_data( char *data, int length )
{
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);
	//unsigned char type;
	int ret;
	bool is_img_data = 0;
	unsigned char * img_data;
	int img_length;
	char tmp_buff[256] = {0};
	
	if(data == NULL || length <= 0)
	{
		return -1;
	}

	
	is_img_data = CheckImgData((unsigned char *)data, 0, length);
	if(is_img_data)
	{

        if(((unsigned int)length - 4 + 1) <= sizeof(tmp_buff))
        {
		  snprintf(tmp_buff, length - 4 + 1,"%s",data + 4);//
		  D("Xtra Data : Img Path %s",tmp_buff);

		  img_length = ReadImg(tmp_buff,&img_data);
        }
		else
		{
		    img_length = 0;
			D("Error : Img Path is too long or not exist");
		}
		

	    
	    
		if(img_length > 0)
		{
            D("Succeed : Img size is %d",img_length); 
		
			if((ret = update_td_init((unsigned char *)img_data, img_length, _gps_state[0].fd)) < 0)
			{
				D("Error : update_td_init failed %d .",ret);
				return ret;
			}
		}
		else return 0;
	}

	if(is_img_data)
	{
	  td_set_update_mode();
	  return 1;
	}


    if(CheckSDBPData((unsigned char *)data,length))
    {
      command_handler(data, length);
      //D("Succeed : Send SDBP Data To Chip TD1030");
	}
	else if(CheckStringCmd((unsigned char *)data,length))
	{
	  command_handler(data, length);
	  D("Succeed : Send String CMD To Chip TD1030");
	}	
	else
	{
	  td_update_rtcm(data, length);
	}
	return 1;
}

static const GpsXtraInterface xtraInterface =
{
	sizeof(GpsXtraInterface),
	bd_xtra_init,
	bd_inject_xtra_data,
};
static const void *
td_bdgps_get_extension(const char *name)
{
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s(%s)", __FUNCTION__,name);

	if(0 == strcmp(name, GPS_XTRA_INTERFACE))
	{
		return &xtraInterface;
	}

	if(0 == strcmp(name, AGPS_INTERFACE))
	{
		return &tdAGpsInterface;
	}

	if(0 == strcmp(name, GPS_NI_INTERFACE))
	{
		return &tdGpsNiInterface;
	}

//	if (0 == strcmp(name, GPS_DEBUG_INTERFACE))
//	{
//		       return &tdGpsDebugInterface;
//	}

	if (0 == strcmp(name, AGPS_RIL_INTERFACE))
	{
		return &tdAGpsRilInterface;
	}

	return NULL;
}

static  GpsInterface  hardwareGpsInterface =
{
	sizeof(GpsInterface),
	td_bdgps_init,
	td_bdgps_start,
	td_bdgps_stop,
	td_bdgps_cleanup,
	td_bdgps_inject_time,
	td_bdgps_inject_location,
	td_bdgps_delete_aiding_data,
	td_bdgps_set_position_mode,
	td_bdgps_get_extension,
};

void InitTdconfig(void)
{
	memset(&tdconfig, 0, sizeof(tdconfig));
}
void RstTdChip(void)
{
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);
	
	UartTxData(chip_rst,9);
	return;
}


void InitTdModule(void)
{
	GpsState  *state = _gps_state;
	//int ret;

	D("Ver:20171114");
	LoadBdGpsConfigFile();
	D("Succeed : Load configfile ok!");

	//vcc switch pin
	system("echo -wmode 138 0 > /sys/devices/virtual/misc/mtgpio/pin");
	system("echo -wdir 138 1 > /sys/devices/virtual/misc/mtgpio/pin");
	system("echo -wdout 138 1 > /sys/devices/virtual/misc/mtgpio/pin");

	//rst pin(optional);
	system("echo -wdir 18 1 > /sys/devices/virtual/misc/mtgpio/pin");
	system("echo -wdout 18 0 > /sys/devices/virtual/misc/mtgpio/pin");
	system("echo -wdout 18 1 > /sys/devices/virtual/misc/mtgpio/pin");

	usleep(5000);

	system("echo -wdout 18 0 > /sys/devices/virtual/misc/mtgpio/pin");
	usleep(150000);

	//add GPBIN frame header and tail
	memcpy(origin_data, "$GPBIN", OG_MS_FRAME_HEAD_LEN);
	origin_data[OG_MS_FRAME_LEN - 1] = OG_MS_FRAME_TAIL_H;
	origin_data[OG_MS_FRAME_LEN - 2] = OG_MS_FRAME_TAIL_L;
	
	state->fd = TdUartOpen();

	if(state->fd == -1)
	{
		state->init = 0;
		return;
	}

	UartTxData((unsigned char*)"init",4);
	usleep(150000);

	switch(tdconfig.gnss_mode)
	{
	 case 1:UartTxData(gnssmode_bds,15);break;
   case 2:UartTxData(gnssmode_gps,15);break;
	 case 3:UartTxData(gnssmode_gps_bds,15);break;
	 case 4:UartTxData(gnssmode_gl,15);break;
	 case 5:UartTxData(gnssmode_bds_gl,15);break;
	 case 6:UartTxData(gnssmode_gps_gl,15);break;
	 default:UartTxData(gnssmode_gps_bds,15);break;
	}

	usleep(150000);
	UartTxData(fix_dop100,15);

//	usleep(150000);
//	UartTxData(chip_standby,9);
//	tdconfig.power_flag = TD_OFF;

}

const GpsInterface *gps__get_gps_interface(struct gps_device_t *dev)
{
	static int check = 0;
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);
    if(dev == 0){}
	if(check == 0)
	{
		check = 1;
		InitTdModule();
	}

	return &hardwareGpsInterface;
}

static int open_gps(const struct hw_module_t *module, char const *name,
					struct hw_device_t **device)
{
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s name = %s", __FUNCTION__,name);

	struct gps_device_t *dev = malloc(sizeof(struct gps_device_t));
	memset(dev, 0, sizeof(*dev));

	dev->common.tag = HARDWARE_DEVICE_TAG;
	dev->common.version = 0;
	dev->common.module = (struct hw_module_t *)module;
	dev->get_gps_interface = gps__get_gps_interface;

	*device = (struct hw_device_t *)dev;
	return 0;
}


static struct hw_module_methods_t gps_module_methods =
{
	.open = open_gps
};


struct hw_module_t HAL_MODULE_INFO_SYM =
{
	.tag = HARDWARE_MODULE_TAG,
	.version_major = 1,
	.version_minor = 0,
	.id = GPS_HARDWARE_MODULE_ID,
	.name = "TD1030 Module",
	.author = "Techtotop Corporation",
	.methods = &gps_module_methods,
};

static pthread_t update_thread_id;

void Creat_update_thread(void)
{
	int ret;
	pthread_attr_t attr;

	pthread_attr_init (&attr);
	ret = pthread_create(&update_thread_id, &attr, update_td_handler_thread, NULL);

	if ( ret != 0 )
	{
		D("Error : Could not create thread:update_td_handler_thread");
		return;
	}
	else 
		D("Succeed : create thread:update_td_handler_thread");
}


void SendResultToApp(int res)
{
    if(res == 2)
    {
	  ChipUpdateResult(2);
    }
	else if(res == 1)
	{
		D("update_td success");
		ChipUpdateResult(1);
		update_mode = 0;	
		if(td_mode.on_need==0)
			PowerOff();
	}
	else if(res == 3)
	{
		ChipUpdateResult(3);
		D("update_td failed");
		update_mode = 0;
		if(td_mode.on_need==0)
			PowerOff();
	}
}




