#ifndef __UPDATE_H_
#define __UPDATE_H_

#define UPDATE_REC_DATA_ING	   (2)
#define UPDATE_ING		   (1)
#define UPDATE_SUCCESS 		   (0)
#define UPDATE_ERROR		   (-1)
#define UPDATE_GET_FILE_STAT_ERROR (-2)
#define UPDATE_ALLOC_MEMORY_ERROR  (-3)
#define UPDATE_FILE_OPEN_ERROR     (-4)
#define UPDATE_OPEN_COM_ERROR      (-5)
#define UPDATE_COM_TX_ERROR        (-6)

#define UPDATE_PRE_ERROR           (-7)
#define UPDATE_TRANS_ERROR	   (-8)
#define UPDATE_FILE_DATA_ERROR     (-9)
#define UPDATE_FILE_TYPE_ERROR     (-10)
#define UPDATE_COM_NULL_ERROR	   (-11)
#define UPDATE_DATA_MALLOC_ERROR   (-12)

#define UPDATE_DES_FLASH 0
#define UPDATE_DES_SRAM  1
#define UPDATE_DEBUG    0
extern int get_img_file_data(char *fp, unsigned char **file_data, int *file_len);

extern int update_bd(unsigned char *pdata, int len, char *transpport, int update_des);

extern int update_td_scheduler_use_id(unsigned char *pdata, int len, int comid, int update_des);

extern int update_td_check_img_file(unsigned char *data, int len, unsigned char *type);

extern int update_td_handler(void);

extern int update_td_handler_return(unsigned char *data);

extern int update_td_enter_update(void);

extern int update_td_init(const unsigned char *update_data, int data_len, int comid);

extern void update_td_get_sync(int *sync_now, int *sync_total);
void* update_td_handler_thread(void *arg);


#endif

