#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include "agps.h"
#include "td_types.h"
#include "tdgnss.h"
extern GpsState _gps_state[1];
static pthread_mutex_t td_bd_gps_event_mutex   = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t td_bd_gps_supl_cellid_mutex   = PTHREAD_MUTEX_INITIALIZER;

//static pthread_cond_t td_bd_gps_cond     = PTHREAD_COND_INITIALIZER;
static pthread_cond_t td_bd_gps_event_cond     = PTHREAD_COND_INITIALIZER;
static pthread_cond_t td_bd_gps_supl_event_cond     = PTHREAD_COND_INITIALIZER;
pthread_mutex_t AgpsDataMutex   = PTHREAD_MUTEX_INITIALIZER;

static tTD_UINT32 td_bd_gps_pending_event = 0;
int line_len(char *p);

//td_bd_gps_commonCfgData *pcfg_data = 0;
static pthread_t td_bd_gps_thread_id, supl_client_assist_id;

#ifdef UPDATE_SUPL_LOCATION
extern pthread_mutex_t locationInfoEventMutex;
extern GpsLocation      locationInfo;
#endif

TD_INFO td_agps_info;

void *td_bd_gps_thread(void *param);
void *supl_client_assist_thread(void *param);


static int td_bd_gps_send_event(TD_EVENT event, void *data, int size)
{
	D("Enter : %s, event:%d, size:%d", __FUNCTION__, event, size);

	assert(event < TD_EVENT_NUM);
	assert(data);

	pthread_mutex_lock(&td_bd_gps_event_mutex);
	td_bd_gps_pending_event |= 1UL << event;
	memcpy(td_agps_info.td_event_data[event], data, size);
	pthread_cond_signal(&td_bd_gps_event_cond);
	pthread_mutex_unlock(&td_bd_gps_event_mutex);

	return 0;
}

tTD_BOOL td_bd_agps_init(void)
{
	int ret;
	pthread_attr_t attr;
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);
	memset(&td_agps_info, 0, sizeof(TD_INFO));
	
	td_bd_gps_setAgpsServer(tdconfig.strconfigfile.agps_server, tdconfig.agps_port);
	
	td_agps_info.td_event_data[TD_EVENT_CELL_INFO] = &td_agps_info.EventCellInfo;
	td_agps_info.td_event_data[TD_EVENT_SETID_INFO] = &td_agps_info.EventSETIDInfo;
	
	pthread_attr_init (&attr);
	ret = pthread_create(&supl_client_assist_id, &attr, supl_client_assist_thread, NULL);

	if ( ret != 0 )
	{
		D("supl_client_assist_thread: fatal error: could not create thread, ret=%d, errno=%d\n", ret, errno);
		return TD_FALSE;
	}

	pthread_attr_init (&attr);
	ret = pthread_create(&td_bd_gps_thread_id, &attr, td_bd_gps_thread, NULL);

	if ( ret != 0 )
	{
		D("td_bd_gps_thread: fatal error: could not create thread, ret=%d, errno=%d\n", ret, errno);
		return TD_FALSE;
	}


	return TD_TRUE;
}

tTD_BOOL  td_bd_gps_setAgpsServer(tTD_CHAR *ipAddress, tTD_UINT16 portNumber)
{
	if(!ipAddress || !strlen(ipAddress) || (strlen(ipAddress) >= 256))
	{
		D("%s addr err\n", __FUNCTION__);
		return TD_FALSE;
	}

	strcpy(td_agps_info.supl_serverAddress, ipAddress);
	td_agps_info.supl_serverPort = portNumber;

	return TD_TRUE;
}

tTD_BOOL CP_SendCellInfo(NetCellID *pCellInfo)
{
	assert(pCellInfo);

	if(pCellInfo->CellInfoFlag)
		td_bd_gps_send_event(TD_EVENT_CELL_INFO, pCellInfo, sizeof(NetCellID));
	else
		D("CellInfoInvalid");

	return TD_TRUE;
}

tTD_BOOL CP_SendSETIDInfo(IdSetInfo *pSETidInfo)
{
	assert(pSETidInfo);

	if(pSETidInfo->IdSetFlag)
		td_bd_gps_send_event(TD_EVENT_SETID_INFO, pSETidInfo, sizeof(IdSetInfo));
	else
		D("isSETIDInfoInvalid");

	return TD_TRUE;
}

static void td_bd_gps_handle_cellid(NetCellID *cellid)
{
	D("Enter:%s", __FUNCTION__);
	assert(cellid);
	pthread_mutex_lock(&td_bd_gps_supl_cellid_mutex);
	td_agps_info.supl_thread_CellInfo = *cellid;
	pthread_cond_signal(&td_bd_gps_supl_event_cond);
	pthread_mutex_unlock(&td_bd_gps_supl_cellid_mutex);
}


void *td_bd_gps_thread(void *param)
{
	uint32_t pending_event;
	NetCellID 	CellInfo;
	//IdSetInfo	setid;
	(void)param;
	while(1)
	{
		pthread_mutex_lock(&td_bd_gps_event_mutex);

		while (td_bd_gps_pending_event == 0)
		{
			pthread_cond_wait(&td_bd_gps_event_cond, &td_bd_gps_event_mutex);
		}

		pending_event = td_bd_gps_pending_event;
		CellInfo = td_agps_info.EventCellInfo;
		//setid = td_agps_info.EventSETIDInfo;
		td_bd_gps_pending_event = 0;
		pthread_mutex_unlock(&td_bd_gps_event_mutex);

		if(pending_event & (1UL << TD_EVENT_CELL_INFO))
		{
			D("Event : td_bd_gps_EVENT_CELL_INFO");
			td_bd_gps_handle_cellid(&CellInfo);
		}

		if(pending_event & (1UL << TD_EVENT_SETID_INFO))
		{
		    D("Event : td_bd_gps_EVENT_SETID_INFO");
			td_bd_gps_handle_cellid(&CellInfo);//no sim card for debug
			
		}
	}
}


static int CheckPermissions(char*filename)
{
   char cmd[1024] = {0};

   strcpy(cmd, "ls -l ");
   strcat(cmd, filename);
   FILE *pp = popen(cmd,"r");
      
   if(!pp)
   {
    D("Error : File open fail");
   	return -1;
   }

   memset(cmd,0,sizeof(cmd));

   fgets(cmd,sizeof(cmd)-1, pp);
   D("SUPL : %s",cmd);
   pclose(pp);
   
   if(strstr(cmd,"-rwxrwxrwx") == NULL)
   {
     D("Error : supl-client Operation not permitted");

	 /*
     memset(cmd,0,sizeof(cmd));
     strcpy(cmd, "chmod 777 ");
	 strcat(cmd, filename);

	 D("SUPL : %s",cmd);
	 
     pp = popen(cmd,"r");
     memset(cmd,0,sizeof(cmd));

     fgets(cmd,sizeof(cmd)-1, pp);
     D("SUPL : %s",cmd);
	 pclose(pp);
	 */
   }


   return 1;
}


static int mk_supl_cmd(char *buff, NetCellID *CellInfo, NET_TYPE net_type)
{
	char tmp_buff[256];

	assert(buff);
	assert(CellInfo);

	memset(buff, 0, SUPL_CMD_BUFF_SIZE);

    CheckPermissions("/system/bin/supl-client");

	strcpy(buff, "supl-client ");

    if(td_agps_info.supl_serverAddress[0] && (strlen(td_agps_info.supl_serverAddress) < 256))
		strcat(buff, td_agps_info.supl_serverAddress);

	strcat(buff, " --cell ");

     

	
	if(net_type == TD_GSM)
	{
		//char tmp_buff2[256];
		strcat(buff, "gsm:");

		sprintf(tmp_buff, "%d,%d:0x%x,0x%x",
				CellInfo->m.mcc,
				CellInfo->m.mnc,
				CellInfo->m.lac,
				(tTD_UINT16)CellInfo->m.cid  
			   );

		strcat(buff, tmp_buff);
	}
	else if(net_type == TD_WCDMA)
	{
		strcat(buff, "wcdma:");

		sprintf(tmp_buff, "%d,%d,0x%x",
				CellInfo->m.mcc,
				CellInfo->m.mnc,
				CellInfo->m.cid  
			   );
		strcat(buff, tmp_buff);
	}
	else
		return -1;
	
	if(td_agps_info.supl_serverPort!=0){
		sprintf(tmp_buff, " -p %d ", (int)td_agps_info.supl_serverPort);
		strcat(buff, tmp_buff);
	}
	if(strlen(td_agps_info.EventSETIDInfo.setid) == 15)
	{
		//int i;

		sprintf(tmp_buff, " -i %s ", td_agps_info.EventSETIDInfo.setid);
		strcat(buff, tmp_buff);
	}



	D("SUPL : %s", buff);
	return 0;
}

static
int handle_supl_cmd(char *response, char *cmd)
{
	fd_set rfds;
	FILE *pstr = 0;
	int fd = -1, result = -1, res_size = 0;
	struct timeval timeout;

	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s(%s)", __FUNCTION__,cmd);

	if(!response || !cmd)
	{
		D("%s params err", __FUNCTION__);
		return result;
	}

	memset(response, 0, SUPL_RESPONSE_BUFF_SIZE);

	pstr = popen(cmd, "r");

	if(!pstr)
	{
		D("%s popen err", __FUNCTION__);
		return result;
	}

	fd = fileno(pstr);
	fcntl(fd, F_SETFL, O_NONBLOCK);

	while(1)
	{
		int ret;
		timeout.tv_sec = 100;
		timeout.tv_usec = 0;

		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		ret = select(fd + 1, &rfds, NULL, NULL, &timeout);

		switch(ret)
		{
			case 0:  //timeout
				D("Error : supl get timeout");
				result = -1;
				goto end;

			case -1:
				D("Error : supl get err:%d", errno);
				result = -1;
				goto end;

			default:
				if(FD_ISSET(fd, &rfds))
				{
					memset(cmd, 0, SUPL_CMD_BUFF_SIZE);
					int size = fread(cmd, 1, SUPL_CMD_BUFF_SIZE - 1, pstr);

					if(size > 0)
					{
//#ifdef DEBUG_SUPL
						D("SUPL : read %d bytes %s", size, cmd);
//#endif
						res_size += strlen(cmd);

						if(res_size < SUPL_RESPONSE_BUFF_SIZE)
							strcat(response, cmd);
						else
						{
							D("response too much");	
							result = -1;
							goto end;
						}
					}
					else
					{
						result = 0;
						goto end;
					}
				}

				break;
		}
	}

end:

	if(pstr)
		pclose(pstr);

	return result;
}



int is_letter(char ch)
{
	if(((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')))
		return 1;

	return 0;
}

int is_num(char ch)
{
	if((ch >= '0') && (ch <= '9'))
		return 1;

	return 0;
}

char *supl_get_first_head(char *head, char *buff)
{
	if(!head || !buff)
		return 0;

	return strstr(buff, head);
}

int copy_line(char *dst, char *src)
{
	int i = 0;

	while(1)
	{
		if((src[i] == '\r') && (src[i + 1] == '\n'))
		{
			dst[i] = src[i];
			dst[i + 1] = src[i + 1];
			dst[i + 2] = 0;
			return i + 2;
		}
		else
			dst[i] = src[i];

		if(src[i] == 0)
		{
			return i;
		}

		i++;
	}

	return 0;
}

char *supl_get_next_num(char *buff)
{
	int i = 0, find_no_num = 0;

	if(!buff)
		return 0;

	while(buff[i])
	{
		if((buff[i] == '.') || (buff[i] == '-') || is_num(buff[i]))	//
		{
			if(find_no_num)
			{
				return (buff + i);
			}
		}
		else
		{
			find_no_num = 1;
		}

		i++;
	}

	return 0;
}
/*
#ifdef DEBUG_SUPL
void print_position_response()
{
	D("T %ld %ld %ld %ld\n", td_agps_info.pos_time.gps_week, td_agps_info.pos_time.gps_tow,
	  td_agps_info.pos_time.stamp.tv_sec, td_agps_info.pos_time.stamp.tv_usec);

	D("L %f %f %d\n", td_agps_info.pos.lat, td_agps_info.pos.lon, td_agps_info.pos.uncertainty);
}


void print_ephemeris_response()
{
	int i;

	for(i = 0; i < td_agps_info.cnt_eph; i++)
	{
		D("e %d %d %d %d %d %d %d %d %d %d",
		  td_agps_info.gps_ephemeris[i].prn,
		  td_agps_info.gps_ephemeris[i].delta_n,
		  td_agps_info.gps_ephemeris[i].M0,
		  td_agps_info.gps_ephemeris[i].A_sqrt,
		  td_agps_info.gps_ephemeris[i].OMEGA_0,
		  td_agps_info.gps_ephemeris[i].i0,
		  td_agps_info.gps_ephemeris[i].w,
		  td_agps_info.gps_ephemeris[i].OMEGA_dot,
		  td_agps_info.gps_ephemeris[i].i_dot,
		  td_agps_info.gps_ephemeris[i].e);
		D(" %d %d %d %d %d %d",
		  td_agps_info.gps_ephemeris[i].Cuc,
		  td_agps_info.gps_ephemeris[i].Cus,
		  td_agps_info.gps_ephemeris[i].Crc,
		  td_agps_info.gps_ephemeris[i].Crs,
		  td_agps_info.gps_ephemeris[i].Cic,
		  td_agps_info.gps_ephemeris[i].Cis);
		D(" %d %d %d %d %d %d",
		  td_agps_info.gps_ephemeris[i].toe,
		  td_agps_info.gps_ephemeris[i].IODC,
		  td_agps_info.gps_ephemeris[i].toc,
		  td_agps_info.gps_ephemeris[i].AF0,
		  td_agps_info.gps_ephemeris[i].AF1,
		  td_agps_info.gps_ephemeris[i].AF2);
		D(" %d %d %d %d %d\n",
		  td_agps_info.gps_ephemeris[i].bits,
		  td_agps_info.gps_ephemeris[i].ura,
		  td_agps_info.gps_ephemeris[i].health,
		  td_agps_info.gps_ephemeris[i].tgd,
		  td_agps_info.gps_ephemeris[i].AODA);
	}
}
#endif
*/

int line_len(char *p)
{
	int i = 0;

	while(1)
	{
		if((p[i] == '\r') && (p[i + 1] == '\n'))
		{

			return i + 2;
		}

		if(p[i] == 0)
			return i;

		i++;
	}
}


//handle_supl_client_response
int handle_supl_client_response(char *buff, int size)
{
	int i = 0, ret = -1, len;
	char *p_ch = buff;
	char *buff_end = buff + size;

	if(!buff)
		return -1;

#define P_CH_OK(x)	((x) && ((x) < buff_end))

	pthread_mutex_lock(&AgpsDataMutex);


	p_ch = supl_get_first_head("$CCRMI", p_ch);

	if((!P_CH_OK(p_ch)) || (line_len(p_ch) >= 256) )
	{
		p_ch = buff;   //not found CCRMI. so locate p_ch at origin
		D("SUPL : got no CCRMI.go handle eph");
		goto handle_eph;
	}

	if(td_agps_info.agps_data.time_loc_new.mask == 0)
	{
		//so first supl response is accurate.
		len = copy_line(td_agps_info.agps_data.time_loc_new.line, p_ch);
		td_agps_info.agps_data.time_loc_new.mask = 1;
	}
	else
	{
		//ignore this RRCMI sentence cause it have been found at first supl-client get.
		char tmp_buf[256];
		len = copy_line(tmp_buf, p_ch);
	}

	p_ch += len;


handle_eph:

	for(i = 0; i < 32; i++)
	{
		p_ch = supl_get_first_head("$CCGEP", p_ch);

		if(!P_CH_OK(p_ch))
			break;

		if(line_len(p_ch) >= 256)
		{
		  return -2;
		}

		char *p_num = supl_get_next_num(p_ch);

		if(p_num == 0) //supl-client error!!!
		{		  
		  return -3;
		}

		if(atol(p_num) != (i + 1))
			continue;
		else
			td_agps_info.agps_data.ehp_new.eph_mask |= (0x1 << i);

		len = copy_line(td_agps_info.agps_data.ehp_new.line[i], p_ch);

		if(len == 0)
		{		  
		  return -4;
		}

		p_ch += len;
	}


	if(td_agps_info.agps_data.ehp_new.eph_mask == 0 || td_agps_info.agps_data.time_loc_new.mask == 0) //get no any eph or get no time_loc
		ret = -5;
	else
		ret = 0;

	pthread_mutex_unlock(&AgpsDataMutex);
	return ret;
}

static void notify_agps(void)
{
	GpsState *s = (GpsState *)_gps_state;
	char  cmd = CMD_AGPS;
	int   result;

	do
	{
		result = write(s->control[0], &cmd, 1 );
	}
	while (result < 0 && errno == EINTR);

	if (result != 1)
		D("could not send CMD_AGPS command");
}
void *supl_client_assist_thread(void *param)
{
    (void)param;
	if (GetDebugMaskBit(D_FUCTION) == 1)D("Enter : %s", __FUNCTION__);
	//uint32_t pending_event;
	NetCellID 	CellInfo;
	char *cmd_str = 0;
	char *cmd_response = 0;
	int ret, response_size = 0;

     
	while(1)
	{
	
		pthread_mutex_lock(&td_bd_gps_supl_cellid_mutex);
		pthread_cond_wait(&td_bd_gps_supl_event_cond, &td_bd_gps_supl_cellid_mutex);
		CellInfo = td_agps_info.supl_thread_CellInfo;
		pthread_mutex_unlock(&td_bd_gps_supl_cellid_mutex);

		memset(&td_agps_info.pos_time, 0, sizeof(struct gps_time));
		memset(&td_agps_info.pos, 0, sizeof(struct gps_pos));
		memset(td_agps_info.gps_ephemeris, 0, sizeof(struct supl_ephemeris_s) * 32);
		td_agps_info.cnt_eph = 0;
		memset(&td_agps_info.eph_time, 0, sizeof(struct gps_time));
		memset(&td_agps_info.agps_data.ehp_new, 0, sizeof(struct supl_ephemeris_new));
		memset(&td_agps_info.agps_data.time_loc_new, 0, sizeof(struct supl_time_loc_new));

		D("SUPL : supl_client_assist start");
     
		cmd_str = (char *)malloc(SUPL_CMD_BUFF_SIZE);

		if(!cmd_str)
		{
			D("%s mem err", __FUNCTION__);
			goto end;
		}

		cmd_response = (char *)malloc(SUPL_RESPONSE_BUFF_SIZE);

		if(!cmd_response)
		{
			D("%s mem err1", __FUNCTION__);
			goto end;
		}
		CellInfo.m.type = TD_GSM;
		if(mk_supl_cmd(cmd_str, &CellInfo, CellInfo.m.type) != 0)
		{
			D("Error : %s mk_supl_cmd err", __FUNCTION__);
			goto end;
		}

		if(handle_supl_cmd(cmd_response, cmd_str) != 0)
		{
			D("Error : %s handle_supl_cmd err", __FUNCTION__);
			goto end;
		}

		response_size = strlen(cmd_response);
		D("SUPL : get response:%d bytes", response_size);
		ret = handle_supl_client_response(cmd_response, response_size);

		if(ret != 0) 
		{
			D("SUPL : handle_position_response ERR:%d", ret);
		}

		D("SUPL : before notify cmd_agps RMI_mask %d, eph_mask 0x%x", td_agps_info.agps_data.time_loc_new.mask, td_agps_info.agps_data.ehp_new.eph_mask);

		notify_agps();

end:

		if(cmd_str)
			free(cmd_str);

		cmd_str = 0;

		if(cmd_response)
			free(cmd_response);

		cmd_response = 0;

		D("SUPL : supl_client_assist end");
	}
}




