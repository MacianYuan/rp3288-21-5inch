#ifndef TD_AGPS_H
#define TD_AGPS_H

#define USE_ASSERT
#define NEW_FORMAT  1
//#define UPDATE_SUPL_LOCATION

#include "td_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/


#ifdef USE_ASSERT

#define assert(expr) \
 do{	\
        if(!(expr)) {				        \
        D("Assertion failed! %s,%s,%s,line=%d\n",	\
	#expr, __FILE__, __func__, __LINE__);		        \
        }				\
	}while(0)
#else
#define assert(expr) do {} while (0)
#endif


	typedef enum
	{
		TD_EVENT_CELL_INFO = 0,
		TD_EVENT_SETID_INFO,
		TD_EVENT_NUM
	}
	TD_EVENT;


#define SUPL_CMD_BUFF_SIZE 1024
#define SUPL_RESPONSE_BUFF_SIZE (10*1024)


#define SUPL_CMD_GET_POSITION	0
#define SUPL_CMD_GET_EPHEMERIS	1



	struct supl_almanac_s
	{
		u_int8_t prn;
		u_int16_t e;
		u_int8_t toa;
		int16_t Ksii;
		int16_t OMEGA_dot;
		u_int32_t A_sqrt;
		int32_t OMEGA_0;
		int32_t w;
		int32_t M0;
		int16_t AF0;
		int16_t AF1;
		// int32_t health;
	};

	struct supl_ephemeris_s
	{
		u_int8_t prn;
		u_int8_t fill1;
		u_int16_t delta_n;
		int32_t M0;
		u_int32_t e;
		u_int32_t A_sqrt;
		int32_t OMEGA_0;
		int32_t i0;
		int32_t w;
		int32_t OMEGA_dot;
		int16_t i_dot;
		int16_t Cuc;
		int16_t Cus;
		int16_t Crc;
		int16_t Crs;
		int16_t Cic;
		int16_t Cis;
		u_int16_t toe;
		u_int16_t IODC;
		u_int16_t toc;
		int32_t AF0;
		int16_t AF1;
		int8_t AF2;
		u_int8_t nav_model;

		/* nav model */
		u_int8_t bits;
		u_int8_t ura;
		u_int8_t health;
		char reserved[11];
		int8_t tgd;
		u_int8_t AODA;
	};

	struct gps_time
	{
		long gps_tow, gps_week;
		struct timeval stamp;
	} ;

	struct gps_pos
	{
		int uncertainty;
		double lat, lon; /* of the base station */
	} ;



	struct supl_utc_s
	{
		int32_t a0;
		int32_t a1;
		int8_t delta_tls;
		u_int8_t tot;
		u_int8_t wnt;
		u_int8_t wnlsf;
		u_int8_t dn;
		u_int8_t delta_tlsf;
		u_int8_t fill[8];
	};


#define EPH_MAX_LINES  32 //32 eph(CCGEP) 

	struct supl_ephemeris_new
	{
		unsigned int eph_mask;  //
		char line[EPH_MAX_LINES][256];
	};

	struct supl_time_loc_new
	{
		int mask;
		char line[256];
	};

	struct supl_agps_data
	{
		struct supl_ephemeris_new ehp_new;
		struct supl_time_loc_new time_loc_new;
	};

	struct que_prn
	{
		int prn_need;
		int prn[EPH_MAX_LINES - 1];
	};

	typedef struct _TD_INFO
	{

		NetCellID 	EventCellInfo, supl_thread_CellInfo;
		IdSetInfo		EventSETIDInfo;

		void *td_event_data[TD_EVENT_NUM];

		tTD_UINT32 supl_serverPort;
		char supl_serverAddress[256];
		int cnt_eph;
		struct supl_ephemeris_s gps_ephemeris[32];
		struct gps_time	pos_time, eph_time;
		struct gps_pos  pos;
		struct supl_utc_s utc;
		struct supl_agps_data agps_data;
	} TD_INFO;

	tTD_BOOL td_bd_agps_init(void);
	tTD_BOOL CP_SendCellInfo(NetCellID *pCellInfo);
	tTD_BOOL CP_SendSETIDInfo(IdSetInfo *pSETidInfo);
	
	tTD_BOOL  td_bd_gps_setAgpsServer(tTD_CHAR *ipAddress, tTD_UINT16 portNumber);
	int pack_sdbp_agps(char *s_cep,unsigned char *packbuf);
	int pack_sdbp_time(char *s_rmi,unsigned char *packbuf);
	void pack_sdbp_time_local(unsigned char *packbuf);
	void pack_sdbp_location(double lat, double lon, float acc, unsigned char *packbuf);


#ifdef __cplusplus
}
#endif /*__cplusplus*/



#endif
