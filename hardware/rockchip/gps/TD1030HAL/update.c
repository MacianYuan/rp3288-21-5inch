#include "update.h"
#include "sdbp.h"
#include "tdgnss.h"
#include "serial.h"

#define WAIT_TIME 20000 //20 ms
#define REC_BUF_MAX 256
#define CHECK_FRAME_SUCCESS 0
#define BURN_MAX_LEN 512

int com_fd = -1;
extern GpsState  _gps_state[1];
extern int update_mode;
int update_sync_count = -1;
int update_sync_total = 0;
typedef struct _update_data_
{
	byte *data;
	int offset;
	int len;
}_update_data_;
_update_data_ update_d = {NULL,0,0};


typedef enum{
	UP_IDEL = 0x00,
	UP_ENT,
	UP_CON,
	UP_ERA,
	UP_WRI_A,
	UP_WRI_E,
	UP_VER,
	UP_FAIL,
	UP_CMP
}STATE_TYPE;

//static byte flag_ack = 0;
STATE_TYPE update_state = UP_IDEL;


//file data func ==============================================
//
//fp: path to the img file
//file_data: return file addr
//file_len: return file len
// ======================================================
extern int get_img_file_data(char *fp, unsigned char **file_data, int *file_len)
{
	int ret = UPDATE_SUCCESS;
	unsigned char *pFileDatas = NULL;
	FILE *f;
	struct stat f_st;
	int rleft = 0;

	if(stat(fp, &f_st) < 0)
	{
		D("Error : stat file error path = %s\n",fp);
		ret = UPDATE_GET_FILE_STAT_ERROR;
		goto ERR;
	}

	if((pFileDatas = (unsigned char *)malloc(f_st.st_size)) == NULL)
	{
		D("malloc img file data error\n");
		ret = UPDATE_ALLOC_MEMORY_ERROR;
		goto ERR;
	}

	if((f = fopen(fp, "rb")) == NULL)
	{
		D("open img file error\n");
		ret = UPDATE_FILE_OPEN_ERROR;
		goto ERR;
	}

	rleft = f_st.st_size;

	while(rleft != 0)
	{
		rleft -= fread(pFileDatas, sizeof(unsigned char), rleft, f);
	}

	*file_data = pFileDatas;
	*file_len = f_st.st_size;

	if(f)fclose(f);

	return ret;


ERR:

	if(pFileDatas)
	{
		free(pFileDatas);
		pFileDatas = NULL;
	}
	*file_data = 0;
	*file_len = 0;

	if(f)fclose(f);

	return ret;
}

int update_td_check_img_file(unsigned char *data, int len, unsigned char *type)
{

	if(!CheckImgFile(data, 0, len, type))
	{
		D("update_td_check_img_file error.\n");
		return UPDATE_FILE_TYPE_ERROR;
	}

	return TRUE;
}

void update_td_get_sync(int *sync_now, int *sync_total)
{
	*sync_now = update_sync_count;
	*sync_total = update_sync_total;
}


int update_td_init(const unsigned char *update_data, int data_len, int comid)
{
	D("update_td_init called.\n");

	byte type;
	if(comid < 0)
	{
		return UPDATE_COM_TX_ERROR;
	}

	if(CheckImgFile(update_data, 0, data_len, &type)!=TRUE)
	{
		D("Error : header error %x %x %x %x\n",update_data[0],update_data[1],update_data[2],update_data[3]);
		return UPDATE_FILE_TYPE_ERROR;
	}

	update_d.data = (unsigned char *)malloc(data_len);

	if(NULL == update_d.data)
	{
		D("update_td_init malloc data error.\n");
		return UPDATE_DATA_MALLOC_ERROR;
	}

	com_fd = comid;
	memcpy(update_d.data, update_data, data_len);
	update_d.len = data_len;
	update_d.offset = 0;
	update_state = UP_ENT;
	update_sync_count = 0;
	update_sync_total = data_len / BURN_MAX_LEN;


	D("update_td_init returned. update len: %d. sync total: %d.\n", update_d.len, update_sync_total);


	return TRUE;
}

static void update_end(void)
{
	D("update end called\n");
	
	SDBP_UPD_REBOOT();
	usleep(10000);

	if(update_d.data)
		free(update_d.data);

	update_d.data = NULL;
	update_d.len = 0;
	update_d.offset = 0;	
	update_state = UP_IDEL;
	update_mode = 0;
}




int update_td_enter_update(void)
{
	int datalen;
	int waitcnt = 0;	
    int repeat_send_times = 0;
	
  while(update_sync_count <= update_sync_total)	
  	{
  	    update_state = UP_WRI_A;
		datalen = (update_sync_count != update_sync_total) ? BURN_MAX_LEN : (update_d.len - update_sync_total * BURN_MAX_LEN);
		if(datalen <= 0)break;
		
		if(FALSE == SDBP_UPD_WRITE_I(update_d.data, update_d.offset, datalen))
		{
			update_d.offset += datalen;
			update_sync_count++;
	        continue;

		}
		else
		{
			update_d.offset += datalen;
			update_sync_count++;
		}
        waitcnt = 0;
		while(update_state != UP_WRI_E&& waitcnt < 200){usleep(15000);waitcnt ++;}

		
		if(waitcnt >= 200)
		{
          repeat_send_times ++;
		  update_d.offset -= datalen;
		  update_sync_count--;
		}
		if(repeat_send_times >= 10)
		{
		  D("Error : Upgrad timeout");
		  update_state = UP_FAIL;
		  break;
		}
  	}

  if(update_d.offset == update_d.len)
  	{
       D("Succeed : upgrade finish %d/%d",update_d.offset,update_d.len);
	   update_state = UP_VER;
	   SDBP_QUE_VER_I();
	   return 1;
    }
  else 
  	{
  	    D("ERROR : upgrade FALSE %d/%d",update_d.offset,update_d.len);
        update_state = UP_FAIL;
		return 0;
  	}
  	
  
}

int update_td_check_state(void)
{
	SDBP_QUE_VER_I();
	return 0;
}

int update_td_check_state_return(unsigned char *data, int len)
{
	char ver[150];
	if(FALSE == SDBP_QUE_VER_O(data, 0))
	{
		D("get version nak");
		return UPDATE_TRANS_ERROR;
	}
	
	memcpy(ver, &data[6], len-6-2);
	ver[len-6-2] = '\0';
	D("Version: %s",ver);

	update_state = UP_IDEL;	

//	return update_td_handler();
	return 0;
}


void update_td_set_BaudRate(int BaudRate)
{
    UARTPara para;

	para.com = 1;	
	para.databit = 0;
	para.stopbit = 0;
	para.checkbit = 0;


	if(BaudRate == 115200)
	{
		para.baudrate = 6;
	    SDBP_CFG_UART_I(para, 0); //115200 0 not save; 1 autosave
	    usleep(1000000);
	    BaudRateChange(B115200);//how to check 115200 or not
	    usleep(500000);
		D("Succeed : Set BaudRate to %d",BaudRate);
	}
	else if(BaudRate == 9600)
	{
		para.baudrate = 2;
		SDBP_CFG_UART_I(para, 0); //115200 0 not save; 1 autosave
	    usleep(1000000);
	    BaudRateChange(B9600);//how to check 115200 or not
	    usleep(500000);
		D("Succeed : Set BaudRate to %d",BaudRate);
	}


}

void* update_td_handler_thread(void *arg)
{
   (void)arg;
   int again_times = 0;

   do
   {
	   do
	   {
	       
		   SDBP_UPD_ENTER();
		   again_times ++;
		   DiscardBuffer();
		   Write55();
		   usleep(100000);
		   update_state = UP_CON;
		   SDBP_UPD_CONFIRM_I();
		   usleep(500000);
	   
	   }while(update_state != UP_ERA && again_times < 5);
	   
	   if(again_times >= 5)
	   	{
	   	  goto Fail;

		}
		update_td_set_BaudRate(115200);
		update_state = UP_ERA;
		SDBP_UPD_ERASE_I1();
		usleep(6000000);
   	}while(update_state != UP_WRI_A&& again_times < 5);
   	if(again_times >= 5)
	{
	   	goto Fail;
	}
 
    if(update_td_enter_update() == 0)
    {
	   	goto Fail;
	}

    again_times = 0;
    while(update_state != UP_CMP&& again_times < 5){again_times ++;usleep(500000);}
   	if(again_times >= 5)
	{
	   	goto Fail;
	}

    update_end();
    update_td_set_BaudRate(tdconfig.uartboadrate);
	

    return 0;

Fail:
	update_end();
	update_td_set_BaudRate(tdconfig.uartboadrate);
    update_state = UP_FAIL;

    return 0;
}


int update_td_handler_return(unsigned char *data)
{
	switch(update_state)
	{
		case UP_VER: 
			if(SDBP_QUE_VER_O(data, 0) == TRUE)
			{
			  D("Succeed : SDBP_QUE_VER_O");
			  SendResultToApp(1);
			  update_state = UP_CMP;
              return UPDATE_SUCCESS;
			}
			break;
		case UP_ENT:break;
		case UP_CON:
			if(SDBP_UPD_CONFIRM_O(data, 0) == TRUE)
			{
			  D("Succeed : SDBP_UPD_CONFIRM_O");
              update_state = UP_ERA;
			}
			break;
		case UP_ERA:
			
			if(TRUE == SDBP_PUB_ACK(data,eraseID, 0))
			{
			  D("Succeed : SDBP_PUB_ACK ERASE");
              update_state = UP_ERA;
			}	
			else if(TRUE== SDBP_UPD_ERASE_O1(data, 0))
			{
			  D("Succeed : SDBP_UPD_ERASE_O1");
              update_state = UP_WRI_A;
			}			
			break;
		case UP_WRI_A:
			if(TRUE== SDBP_UPD_WRITE_O(data, 0))
			{
			  D("Succeed : SDBP_UPD_WRITE_O");
              update_state = UP_WRI_E;
			  SendResultToApp(2);
			}			
			break;		
		case UP_IDEL:break;
		default:
			break;
	}
	return UPDATE_ING;
}


