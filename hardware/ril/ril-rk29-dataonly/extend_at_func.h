/*
    һ��ģ�飬������ATָ�����������: ��׼AT���� + ��չAT����
    ��׼AT�����Ҷ�һ��������չAT������ܲ�ͬ��Ŀǰ��֪�����������:
        ���˲���ģ��ʹ�����Զ������չAT����
        �й��ƶ����й���ͨ��GSMģ�鷽�����õ���չAT������ͬ
        �й�������CDMAģ�鷽��Ҳ���Լ���չAT����
        
    ע: Ŀǰû�е��������ģ�飬��������AT��չָ�����εľͲ��ö�֪��
    
    cmy: ��Ҫʹ����չAT����ʵ�ֵĹ��ܣ����伯���ڸ��ļ���
 */

typedef enum {
    NETWORK_MODE_WCDMA_PREF = 0,           /* GSM/UMTS (UMTS preferred) */
    NETWORK_MODE_GSM_ONLY,                 /* GSM only */
    NETWORK_MODE_WCDMA_ONLY,               /* UMTS only */
    NETWORK_MODE_GSM_UMTS,                 /* cmy: �ø�ֵ��ʾGSM preferred. GSM/WCDMA/TD-SCDMA (auto mode, according to PRL) */
    NETWORK_MODE_CDMA,                     /* CDMA and EvDo (auto mode, according to PRL) */
    NETWORK_MODE_CDMA_NO_EVDO,             /* CDMA only */
    NETWORK_MODE_EVDO_NO_CDMA,             /* EvDo only */
    NETWORK_MODE_GLOBAL,                   /* cmy: �ø�ֵ��ʾ�Զ�ѡ�� GSM/WCDMA/TD-SCDMA, CDMA, and EvDo (auto mode, according to PRL) */
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
