/*
    一个模块，其所用AT指令集大致如此组成: 标准AT命令 + 扩展AT命令
    标准AT命令大家都一样，但扩展AT命令可能不同，目前所知道的情况如下:
        中兴部分模块使用其自定义的扩展AT命令
        中国移动、中国联通在GSM模块方面所用的扩展AT命令相同
        中国电信在CDMA模块方面也有自己扩展AT命令
        
    注: 目前没有调过国外的模块，他们所用AT扩展指令集是如何的就不得而知了
    
    cmy: 需要使用扩展AT命令实现的功能，将其集中在该文件中
 */

typedef enum {
    NETWORK_MODE_WCDMA_PREF = 0,           /* GSM/UMTS (UMTS preferred) */
    NETWORK_MODE_GSM_ONLY,                 /* GSM only */
    NETWORK_MODE_WCDMA_ONLY,               /* UMTS only */
    NETWORK_MODE_GSM_UMTS,                 /* cmy: 用该值表示GSM preferred. GSM/WCDMA/TD-SCDMA (auto mode, according to PRL) */
    NETWORK_MODE_CDMA,                     /* CDMA and EvDo (auto mode, according to PRL) */
    NETWORK_MODE_CDMA_NO_EVDO,             /* CDMA only */
    NETWORK_MODE_EVDO_NO_CDMA,             /* EvDo only */
    NETWORK_MODE_GLOBAL,                   /* cmy: 用该值表示自动选择 GSM/WCDMA/TD-SCDMA, CDMA, and EvDo (auto mode, according to PRL) */
} RIL_NetworkMode;


typedef struct{
    int srv_status;
    int srv_domain;
    int roam_status;
    int sys_mode;
    int sim_state;
    int reserve;
    int sys_submode;
}AT_SYSTEM_INFO;

#if (RIL_VERSION >= 6)
typedef enum {
    RADIO_TECHNOLOGY_UNKNOWN = 0,
    RADIO_TECHNOLOGY_GPRS,
    RADIO_TECHNOLOGY_EDGE,
    RADIO_TECHNOLOGY_UMTS,
// CDMA95A/B
    RADIO_TECHNOLOGY_IS95A,
    RADIO_TECHNOLOGY_IS95B,
// CDMA2000 1x RTT
    RADIO_TECHNOLOGY_1xRTT,
// CDMA2000 1x EVDO
    RADIO_TECHNOLOGY_EVDO_0,
    RADIO_TECHNOLOGY_EVDO_A,
//
    RADIO_TECHNOLOGY_HSDPA,
    RADIO_TECHNOLOGY_HSUPA,
    RADIO_TECHNOLOGY_HSPA,
	
    RADIO_TECHNOLOGY_EVDO_B,
    RADIO_TECHNOLOGY_EHRPD,
    RADIO_TECHNOLOGY_LTE,
    RADIO_TECHNOLOGY_HSPAP, // HSPA+

    RADIO_TECHNOLOGY_TDSCDMA,
} RIL_NetworkType;
#endif

int getPreferredNetworkType_cnm();
int setPreferredNetworkType_cnm(int rat);
int getNetworkType_cnm();
int getRestrictedState_cnm();

int getPreferredNetworkType_cnt();
int setPreferredNetworkType_cnt(int rat);

int getPreferredNetworkType_zte();
int setPreferredNetworkType_zte(int rat);
int getNetworkType_zte();
int getRestrictedState_zte();

int getPreferredNetworkType_zte_ad3812();
int setPreferredNetworkType_zte_ad3812(int rat);

int getPreferredNetworkType_generic();
int setPreferredNetworkType_generic(int rat);
int getRestrictedState_generic();
int getNetworkType_generic();

int getNetworkType_lc6341();
int getRestrictedState_lc6341();

int getPreferredNetworkType_archos_g9();
int setPreferredNetworkType_archos_g9(int rat);

int getQualmNetworkType();


int getPreferredNetworkType_SCV_SPW9P();
int setPreferredNetworkType_SCV_SPW9P(int rat);

int networkType2RAT(int net);
