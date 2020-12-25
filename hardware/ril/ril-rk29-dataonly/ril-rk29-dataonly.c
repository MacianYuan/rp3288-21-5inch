/* //device/system/reference-ril/reference-ril.c
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

/**********************************************************************************
 *                      Version
 *
 *  ril-rk29-mid v0.10      ��android2.1����ֲ����
 *  ril-rk29-mid v0.11      �¼Ӽ���modem��֧��
 *  ril-rk29-mid v0.12      ����ʱ��������³�����Ϊ����ʧ�ܣ���pppd�������������ղ��ųɹ���
 *                          Ŀǰ�����������ʱ��ɱ��PPPD����
 *                          ��ѯ��Ӫ�̵ķ��ش������⣬������
 *  ril-rk29-mid v0.13      modem�б�ŵ�modem_list.h�ļ���
 *
 *  ril-rk29-dataonly v0.14      �����������Ӳ���ģ�飬�����������͵Ļ�ȡ
 *  ril-rk29-dataonly v0.15      ���ֶ���ttyACM�豸(��Ҫ��TD-SCDMA�豸)���ڻ�ȡ����vid/pidʱ�������⣬Ŀǰ������
 *  ril-rk29-dataonly v0.16      ֧��USSD(Unstructured Supplementary Service Data)
 *  ril-rk29-dataonly v0.17      ttyACM�����������⣬TD-SCDMA�豸ȫ��ʹ��ttyUSB����
 *                             ע���ں��Ǳ�Ҫ��ACM����ȥ��
 *  ril-rk29-dataonly v0.18      ��Ӽ���dongle��֧��
 *  ril-rk29-dataonly v0.20      ����Dongle����ϵͳ��˯���л��Ѻ󣬷���ԭ�е��������Ӳ����ã���
 *                             ʱ��Ҫ��������
 *  ril-rk29-dataonly v0.23      �޸�MC8630ģ���֧�����⣬����µ�ģ��:F210(19d2/2003)��ALCATEL
 *  ril-rk29-dataonly v0.33      ���ֳ���"˯���л��Ѻ�ԭ���������Ӳ�����"��ģ����Ҫ��CDMA��������CDMA/EVDOģ��
 *                             ������������ʱ�����뻽��������ֹϵͳ����˯��
 *  ril-rk29-dataonly v1.0.00  �����������У����ʳ���pppd�Զ��˳�������ʱ�ϲ㲢δ�����������޷�����������
 *                             �����ڲ�����ɺ��½��߳�������pppd״̬
 *  ril-rk29-dataonly v1.0.02  ʹ��ENABLE_STAY_AWAKE�����Ƿ񱣳�����״̬(����������)
 *                             ����BUG: ��modemƥ��������ڵ���onSIMReadyʱ���ٴη��������ƥ��
 *  ril-rk29-dataonly v1.0.03  ����ģ���ڳ�ʼ�������У����ܲ���AT�����ATָ��ķ��ش��л��ԣ�����ڳ�ʼ������ʱ�ٽ�����"ATE0"ȡ������
 *  ril-rk29-dataonly v1.0.04  �������dongle��֧�֣������ɿͻ��ṩ,���5��modem��֧�֡�����APN�б��������������Թ��ҵ�APN
 *  ril-rk29-dataonly v1.0.05  CDMAģʽ��һ�β�������ʱ���ȴ�һ��ʱ��
 *  ril-rk29-dataonly v1.0.06  ���modem֧�ֵķ�ʽ��ֻ֧��EVDOģʽ������TDģʽ������WCDMAģʽ
 *  ril-rk29-dataonly v1.0.07  ��Ӽ���Dongle֧�֣�����һ������������TU930ʹ�����µĲ��Žű�
 *  ril-rk29-dataonly v1.0.08  ���CDMA/EVDOģ�飬�޸���������Ļ���������ݳ�ʱ�޷�����������
 *  ril-rk29-dataonly v1.0.09  ��԰�����dongle��2G/3G�����л���������
 *  ril-rk29-dataonly v1.0.10  ��ֲ��android4.0
 *  ril-rk29-dataonly v1.0.11  ���3��DONGLE֧��
 *  ril-rk29-dataonly v1.0.12  ֧��E392��MF820
 *  ril-rk29-dataonly v1.1.01  ��ȥ��ѯ����ע��״̬����ΪE1750һֱ����"CGREG: 2,0", ��������۲���MF820�Ľ����������
 *  ril-rk29-dataonly v1.1.02  ��֧�� LG VL600 4G Dongle
 *  ril-rk29-dataonly v1.1.03  ֧��LG VL600 4G Dongle ����ǰ��modem�����vl600 attach
 *  ril-rk29-dataonly v1.2.00  �������modem��֧�֣�����Ӧ������AT�ڵ����
 *  ril-rk29-dataonly v1.2.01  ��������modem�Ĺ���
 *  ril-rk29-dataonly v1.2.02  MU509 2G/3G �л�ʵ��
 *  ril-rk29-dataonly v1.2.03  MTK2  MT6276MA 3G support
 *  ril-rk29-dataonly v1.2.04  E153��CGREG����������⣬ʹ��CREG
 *  ril-rk29-dataonly v1.2.05  �������ģ��mc509�ͼ���dongle�����й�������dongel(����һ��ȴ��ͻ����Խ��)                            
 *  ril-rk29-dataonly v1.2.06  �ϲ�XXH����
 *  ril-rk29-dataonly v1.2.07  ��Ӻ� RIL_RELEASE�����ڿ���δȷ�ϵ�ģ��
 *  ril-rk29-dataonly v1.2.08  ����SIM���������������޸������BUG
 *  ril-rk29-dataonly v1.2.09  ����SIM PUK��������
 *  ril-rk29-dataonly v1.2.10  ���ӶԻ�ȡ����ЧDNS:10.11.12.13�ļ��
 *  ril-rk29-dataonly v1.2.11  ���Ӷ� ������F3307 WCDMA modem��֧��
 *  ril-rk29-dataonly v1.2.12  ������F3307���Žű����⵼���ٴ�����ʱ�����ײ���ʧ��
 *                             DNS������ŵ������ʵ�λ��
 *  ril-rk29-dataonly v1.2.13  ��ΪMU739֧��
 *  ril-rk29-dataonly v1.2.14  ��������3G Dongle
 *  ril-rk29-dataonly v1.2.15  ����vodafone K3770 �ڹ���ʹ�õ����[��ѡ], ���MF631
                               ʹ�ú��������ICS�ں�option.c�仯������2.3�Ͽ���ʹ�ö�ICS�޷�ʹ�õ����
                               ʹ��request������������޸�PIN��ͽ���PUK��
 *  ril-rk29-dataonly v1.2.16  ���3G Dongle֧�֣�UE660,EC122,T1731����һ������ģ�� MW100G
                               �޸�����3Gģ��UW100 2G/3G�л����ڹ�����Է���ֻ������2G
 *  ril-rk29-dataonly v1.2.17  ���/�޸� ��3��3g dongle: Vtion E1916/Vtion U1920/������LKT868
 *  ril-rk29-dataonly v1.2.18  ֧�ֵ�ͼͨ��Modem��λ��ͨ��CREG��ָ���õ�LAC��CID��ֵ
 *  ril-rk29-dataonly v1.2.19  ���Է���LACֵ����4λ����CIDֵ����7λ���������ͼӦ���޷���λλ��
                               ����modem��CIDֻ����4λ�����貹��Ϊ7λ��
 *  ril-rk29-dataonly v1.8.00  �Ƴ����� v1.8
 *  ril-rk29-dataonly v1.8.01  ����E172ʹ������
 *  ril-rk29-dataonly v1.8.02  ֧��USI modem
 *  ril-rk29-dataonly v1.8.03  ֧��Rate EC183��SCV SEV859����ע��SEV859��������AT�ڲ�ͬ
 *  ril-rk29-dataonly v1.8.04  ֧��BSNL 3G��HW E1756
 *  ril-rk29-dataonly v1.8.05  ֧��MMX353G��HW E1731��HW E177��HW E171��ɽկ��E1750��ZTE MC2718
                                ɽկ��E1750��Ҫ�޸�devices_filter.h
 *  ril-rk29-dataonly v1.8.06  ֧��Vodafone K3772-Z����Ҫ�޸�VOLD ����ִ��һ��usb_modeswitch
 *  ril-rk29-dataonly v1.8.07  ֧�ֶ��ʵ���˵�3g dongle��Ŀǰ����VID/PID=0x05C6/0x6000��3g dongle
                               �в���ʹ��ttyUSB2��ΪAT�ڣ�������һ����ʹ��ttyUSB1�����źŶ���ttyUSB0
 *  ril-rk29-dataonly v1.8.08  ���dongle֧�֣���������dongle ATָ��  
                               INTEX(0x230D, 0x000D)��ʼ��ʱʹ��AT*ELED=1,1,0 ����LED��
                               ʵ����TD STD808/ cgmm����NULL���ϲ���������Իᱨ�����һ������ֵ
 *  ril-rk29-dataonly v1.8.09  ���3G dongle֧��
                               ֧��IE701 3G ����ģ��ʹ��ie701.c�������޸�����ģ��MC2718�źŲ�ѯ�Լ��ƹ�SIM�����
                               ӡ��EC156��EC122�ƹ�SIM����⣬����ź��ֶ���ѯ�������ϱ�
                               ӡ��EVDO dongle 0x12D1, 0x140B ������ҪSIM��
                               ���һ���ֻ�Modem(0x20A6, 0x1105, "Test number"), ����һ��CIMI����ֵ������������3G 
 *  ril-rk29-dataonly v1.8.10  ���3G dongle֧��:
                               ZTE-MF193, Nokia CS-11, Onda-MSA14.4
 *  ril-rk29-dataonly v1.8.11  ��Ӷ��Ź���֧�ֵĿ���
 *  ril-rk29-dataonly v1.8.12  ֧��Android4.1�±���
 *  ril-rk29-dataonly v1.8.13  ֧��VID/PID��ͬ ��AT channel��ͬ��ģ��
 *  ril-rk29-dataonly v1.8.14  ��û��3gʱ��radio״̬����ΪRADIO_STATE_OFF���Ӷ��ӿ�ػ����ٶ�
 *  ril-rk29-dataonly v1.9.00  �Ƴ�����1.9
 *  ril-rk29-dataonly v1.9.01  �����ϲ��ź�ͼ����ʾ������
 *  ril-rk29-dataonly v2.0.01  �����й���ͨ3G��������ȫF������
 *  ril-rk29-dataonly v2.0.02  ģ��modemEarlyInit��ʱ����AT+CFUN=1ָ�������Ƶ����
 *  ril-rk29-dataonly v2.0.03  û��3gģ�����3Gģ�鲻֧��ʱ���ϱ�RADIO_STATE_UNAVAILABLE
                               ���ҵ�3Gģ�鲢��AT��ʱ���ϱ�RADIO_STATE_OFF
 *  ril-rk29-dataonly v2.0.04  ����ģ��û��SIM����ʱ�򣬹ر�ģ��,���͹���
 *  ril-rk29-dataonly v2.0.05  ���K3570-Z��Micromax MMX352G��Qualocomm HSPA USB MODEM MF180 3G dognle֧�� 
 *  ril-rk29-dataonly v2.1.00  �Ƴ�����V2.1.00
 *  ril-rk29-dataonly v2.1.01  �Ƴ�����V2.1.01 ӡ��3G  dongle�ۺϴ���
 *  ril-rk29-dataonly v2.1.02  ��Ӳ�����3gģ���֧��
 *  ril-rk29-dataonly v2.1.03  ��������3Gģ�飬֧��AT+CFUN=0��ָ�����ʵ�ַ���ģʽ������ģ����Ȼ��֧�ָ�ָ��
 *  ril-rk29-dataonly v2.1.04  ����ģ�������ϱ���+CGREG����"n"������������Դ�
 *  ril-rk29-dataonly v2.1.05  ���Ӱ���E303,Olivetti��ӡ�� BSNL 3G dongle֧��
 *  ril-rk29-dataonly v2.2.00  ����V2.2.00����
 *  ril-rk29-dataonly v2.2.01  ���ʵ����MU270
 *  ril-rk29-dataonly v2.2.02  ���3G dongle֧�֣�MF669 E3131 E398 E1553  ���� CBP7.1 3G dongle ,B150(����)
 *  ril-rk29-dataonly v2.2.03  USIģ���ѯ��Ӫ��Ϊ���ֵ����(δ����)
 *  ril-rk29-dataonly v2.2.05  ��ӵϰ�3G dongle֧��(������)
                               (0x1C9E,0x9915, "HSDPA mobile station)����AT+CGACT���������ʧ�ܣ����²��Ų��ɹ������˵���ATָ��
 *  ril-rk29-dataonly v2.2.06  ���Ӷ�����Speedup 3G dongle,ӡ��LW272��EVDO AC2787(NO SIM Card),E303D
 *  ril-rk29-dataonly v2.2.07  ӡ�Ȳ�������ģ��ֻ����2G��HUAWEI E303C(modemid=E303S)
                               speedup 3G dongleֻ������һ��3G���޸�Option.c
                               operator_table���MTS��Ӫ��MCC/NCC
 *  ril-rk29-dataonly v2.2.08  ɽկdognle CGREG=2,0 olivetti 3G dognle �л��ű��޸�,����ں˱�������
                               �޸�call-pppd�ű�,ʹ���ֻ��Ǳ�Ҳ������dataonly.so��
 *  ril-rk29-dataonly v2.2.09  add ioctrl to get usi imei
                               ���3G Dongle MULTI_RIL LIB ֧��
                               ���������л�������ǰ����֧�ֵ�dongle���ֻ��в�֧�������
 *  ril-rk29-dataonly v2.2.10  �޸�ZTE EVDO ��ʹ�ú�����/dev/ttyUSB1�޷�ʹ�õ�BUG

 *  ril-rk29-dataonly v2.2.11  �޸Ĳ���dongleһֱ��ȡ��ЧDNS�������ʹ��"8.8.8.8"������net.dns1,net dns2
                               ������֤��������
                              
 *  ril-rk29-dataonly v2.3.00  add MMX144F/MF652/MF193/MF190U/MF667/Airis/DMW-156/GI1515/E220/MMX377GG 3G dongle support 
                               MF195��WM66E��E303��Ҫ�ڹ�����Ժ��ٸ���
 *  ril-rk29-dataonly v2.3.01  USI MT6229 CGERG�����ϱ��޸�,�����޸Ķ�AP_READY���ŵĿ���
                               E3231/E1780 3g DONGLE 
 
 *  ril-rk29-dataonly v2.3.02  ֧��չѶU7501 WCDMA 3G modem
                               ֧�ֻ�ΪE1230 WCDMA 3G Modem 
                               ֧��C218D EVDO 3G modem(���̲�����)

 *  ril-rk29-dataonly v2.3.03  ����SIM�����ڲ�ѯ��IMEI��Źر�����ģ���Դ                          
                                MC509-A ʹ��AT^PPPCFG����user password

 *  ril-rk29-dataonly v2.3.04  ���������������ѯ��Ӫ�̣�2G/3G�޷����ֵ�����,�����������BUG
 *  ril-rk29-dataonly v2.3.05  ���ZTE�������ݿ�֧�֣�
                               ���DHCP��ʽ����
                               ���LTE MESSI DHCP����
 *  ril-rk29-dataonly v2.3.06  ���REQUEST_SIM_IO����                         
                               �������UNA+ DONGLE(0X00A7/0X00A5),SCV HSPS+
 *  ril-rk29-dataonly v2.3.07  �޸�CALL-PPPD�ű�����������PAP��Ȩ��Ч�����      

 *  ril-rk29-dataonly v2.3.08  �����������ģ�飬RILD���ʹҵ���������������:���kill_rild����

 *  ril-rk29-dataoly  v2.3.09  ����donngle�ڲ��Ž׶�AT+CGDCONT����ERROR�����²���ʧ�ܣ�
                               �����ֽ���취:
                                1.���˸�ָ��
                                2.��ȥ��ⷵ��ֵ�Ƿ�OK(beta 1)
                                3.�����˴󲿷�ZTE WCDMA 3G dongle
                                
 *  ril-rk29-dataoly  v2.3.10   1.֧������NCM����LTE dongle ,ʹ��chcpcd usb0��ʽ
                                2.����AT+CGACT����ֵ��飬���Ⲧ��ʧ��

 *  ril-rk29-dataoly  v3.1.00   1.zte DC-LTE���ݿ���ʹ��modem���� 
 *  ril-rk29-dataoly  v3.2.00   1.and network type 13
 **********************************************************************************/

#include <telephony/ril.h>
#define MULTI_RIL_LIB  0
#if (RIL_VERSION >= 6)
#define RIL_CardStatus	RIL_CardStatus_v6
#define RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED		RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
#define RIL_REQUEST_REGISTRATION_STATE					RIL_REQUEST_VOICE_REGISTRATION_STATE
#define RIL_REQUEST_GPRS_REGISTRATION_STATE				RIL_REQUEST_DATA_REGISTRATION_STATE
#endif

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <alloca.h>
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <cutils/properties.h>
#include <sys/cdefs.h>
#include <arpa/inet.h>
#include <termios.h>
#include <sys/system_properties.h>
#include "atchannel.h"
#include "at_tok.h"
#include "misc.h"
#include "extend_at_func.h"
#include "modem_define_func.h"
#include "operator_table.h"
#include "modem_list.h"
#include "gsm.h"
#include "config.h"
//#include "telephony/config.h"
static const char* REFERENCE_RIL_VERSION = "RIL_RK_DATA_V8.0"
#ifndef RIL_RELEASE
    " [debug version]"
#endif
#if !ENABLE_DNS_CHECK
    " [!DNS]"
#endif
#if SUPPORT_SMS
    " [SMS]"
#endif
;

#define DBG_1

#define LOG_NDEBUG 0
#define LOG_TAG "RIL_RK_DATA_V8.0"
#include <utils/Log.h>

#define MAX_AT_RESPONSE 0x1000

#define RIL_REQUEST_SEND_SMS_EXTENDED 512

#define PPP_TTY_PATH "ppp0"

#define OPERATOR_TABLE_PATH     "/etc/operator_table"

#ifdef ENABLE_STAY_AWAKE
#include <hardware_legacy/power.h>
static const char *WAKE_LOCK_ID = "ril-rk29-dataonly";
#endif

#ifdef USE_TI_COMMANDS

// Enable a workaround
// 1) Make incoming call, do not answer
// 2) Hangup remote end
// Expected: call should disappear from CLCC line
// Actual: Call shows as "ACTIVE" before disappearing
#define WORKAROUND_ERRONEOUS_ANSWER 1

// Some varients of the TI stack do not support the +CGEV unsolicited
// response. However, they seem to send an unsolicited +CME ERROR: 150
#define WORKAROUND_FAKE_CGEV 1
#endif

#define SUPPORT_MODEM_TYPE_NONE		0
#define SUPPORT_MODEM_TYPE_EVDO		1
#define SUPPORT_MODEM_TYPE_WCMDA	2
#define SUPPORT_MODEM_TYPE_TDSCDMA	4


#define MODEM_CONTRL_PATH      "/dev/voice_modem"
#define BP_IOCTL_BASE             0x1a
#define BP_IOCTL_RESET         _IOW(BP_IOCTL_BASE, 0x01, int)
#define BP_IOCTL_POWOFF        _IOW(BP_IOCTL_BASE, 0x02, int)
#define BP_IOCTL_POWON         _IOW(BP_IOCTL_BASE, 0x03, int)
#define BP_IOCTL_WRITE_STATUS  _IOW(BP_IOCTL_BASE, 0x04, int)
#define BP_IOCTL_GET_STATUS    _IOR(BP_IOCTL_BASE, 0x05, int)
#define BP_IOCTL_SET_PVID      _IOW(BP_IOCTL_BASE, 0x06, int)
#define BP_IOCTL_GET_BPID      _IOR(BP_IOCTL_BASE, 0x07, int)
#define BP_IOCTL_GET_IMEI      _IOR(BP_IOCTL_BASE, 0x08, int)
char IMEI_value[17];
static int sSupportModemType = SUPPORT_MODEM_TYPE_EVDO|SUPPORT_MODEM_TYPE_WCMDA|SUPPORT_MODEM_TYPE_TDSCDMA;


typedef enum {
    SIM_ABSENT = 0,
    SIM_NOT_READY = 1,
    SIM_READY = 2, /* SIM_READY means the radio state is RADIO_STATE_SIM_READY */
    SIM_PIN = 3,
    SIM_PUK = 4,
    SIM_NETWORK_PERSONALIZATION = 5,
    RUIM_ABSENT = 6,   
    RUIM_NOT_READY = 7,   
    RUIM_READY = 8,    
    RUIM_PIN = 9,   
    RUIM_PUK = 10,   
    RUIM_NETWORK_PERSONALIZATION = 11,
    ISIM_ABSENT = 12,
    ISIM_NOT_READY = 13,
    ISIM_READY = 14,
    ISIM_PIN = 15,
    ISIM_PUK = 16,
    ISIM_NETWORK_PERSONALIZATION = 17,

} SIM_Status; 

static void onRequest (int request, void *data, size_t datalen, RIL_Token t);
static RIL_RadioState currentState();
static int onSupports (int requestCode);
static void onCancel (RIL_Token t);
static const char *getVersion();
static int isRadioOn();
static SIM_Status getSIMStatus();
static int getCardStatus(RIL_CardStatus **pp_card_status);
static void freeCardStatus(RIL_CardStatus *p_card_status);
static void onDataCallListChanged(void *param);
static void closeATReader();
static void onATReaderClosed();
static int findModem(int vid, int pid, const char* modemID, RIL_ModemInterface** modem);
static char* getModelID();
static void kill_rild();
static int kill_pppd();
extern const char * requestToString(int request);
static int GetIMSI(char* imsi);


/*** Static Variables ***/
static const RIL_RadioFunctions s_callbacks = {
    RIL_VERSION,
    onRequest,
    currentState,
    onSupports,
    onCancel,
    getVersion
};

#ifdef RIL_SHLIB
static const struct RIL_Env *s_rilenv;

#define RIL_onRequestComplete(t, e, response, responselen) s_rilenv->OnRequestComplete(t,e, response, responselen)
#define RIL_onUnsolicitedResponse(a,b,c) s_rilenv->OnUnsolicitedResponse(a,b,c)
#define RIL_requestTimedCallback(a,b,c) s_rilenv->RequestTimedCallback(a,b,c)
#endif

/*
   ��RIL_ATMONITOR������ ��Ϊ 'y' ʱ��������model��������
 */
#define RIL_ATMONITOR               "rild.debug.atmonitor"

/*
    ��ָ�����豸����ϵͳ��ʹ��ָ�����豸��Ϊatchannel��
 */
#define RIL_ATCHANNEL               "ril.atchannel"

/* �����ӵ�����󣬸����Դ�ű���IP
 */
#define RIL_LOCAL_IP                "net.ppp0.local-ip"
#define RIL_LOCAL_GW                "net.ppp0.gw"
#define RIL_LOCAL_DNS1		    "net.ppp0.dns1"
#define RIL_LOCAL_DNS2              "net.ppp0.dns2"

static RIL_ModemInterface* s_current_modem = NULL;

static RIL_RadioState sState = RADIO_STATE_UNAVAILABLE;

static pthread_mutex_t s_state_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_state_cond = PTHREAD_COND_INITIALIZER;

static int s_port = -1;
static const char * s_device_path = NULL;
static int          s_device_socket = 0;

/* trigger change to this with s_state_cond */
static int s_closed = 0;

static int sFD;     /* file desc of AT channel */
static char sATBuffer[MAX_AT_RESPONSE+1];
static char *sATBufferCur = NULL;
static char *sNITZtime = NULL;

static const struct timeval TIMEVAL_SIMPOLL = {1,0};
static const struct timeval TIMEVAL_CALLSTATEPOLL = {0,500000};
static const struct timeval TIMEVAL_0 = {0,0};

/* 
    ��ƥ��modem�ͺ�ʱ�����modem��VID/PID�ж��ƥ�䣬����ȡ��һ��ƥ������ʼ����
    �ڳ�ʼ�����ڣ���ͨ����ѯ����modem id���о�ȷƥ�䡣
*/
static int sMmodemRematch = 0;

#ifdef WORKAROUND_ERRONEOUS_ANSWER
// Max number of times we'll try to repoll when we think
// we have a AT+CLCC race condition
#define REPOLL_CALLS_COUNT_MAX 4

// Line index that was incoming or waiting at last poll, or -1 for none
static int s_incomingOrWaitingLine = -1;
// Number of times we've asked for a repoll of AT+CLCC
static int s_repollCallsCount = 0;
// Should we expect a call to be answered in the next CLCC?
static int s_expectAnswer = 0;
#endif /* WORKAROUND_ERRONEOUS_ANSWER */

static void pollSIMState (void *param);
static void setRadioState(RIL_RadioState newState);

static int rematchModem(const char* modemID, const char* atchannel);

static int isgsm=1;
static int isEth=0;
static char *callwaiting_num;

/*
    �����ź� CDMA/GSM/UMTS ��ʹ�� AT+CSQ��EVDOʹ��AT^HDRCSQ��ѯ
    �����ϱ����ź�ֵ��"^RSSI:"��"^RSSILVL:"������ֵ�����signalStrength�� "^HRSSILVL:"�����signalStrength_hdr
 */
static int signalStrength_hdr[2];
static int signalStrength[2];

// ��ס��ǰģ���IMSI���Ա�CDMA��ѯOperatorʱ��
static char IMSI[20];

static int s_cur_vid;
static int s_cur_pid;
static char s_cur_atchannel[64] = "";
static char s_cur_pppchannel[64] = "";

//static pthread_mutex_t s_waitmutex = PTHREAD_MUTEX_INITIALIZER;
//static pthread_cond_t s_waitcond = PTHREAD_COND_INITIALIZER;


/*
  restricted state:
    RIL_RESTRICTED_STATE_NONE
    RIL_RESTRICTED_STATE_CS_EMERGENCY
    RIL_RESTRICTED_STATE_CS_NORMAL
    RIL_RESTRICTED_STATE_CS_ALL
    RIL_RESTRICTED_STATE_PS_ALL
 */
static int s_restricted_state=RIL_RESTRICTED_STATE_NONE;

static int s_reg_stat=-1;
static int s_reg_rat=-1;
static int s_greg_stat=-1;
static int s_sys_mode=-1;

static int tryCount = 0;

int g_fileid = -1;
char g_smspath[64] = {0}; 

static pthread_t s_tid_pppdState;
static pthread_t s_tid_checkTtyState;
static pthread_mutex_t s_waitmutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_waitcond = PTHREAD_COND_INITIALIZER;
static int s_pppd_exception = 0;
static int nosimcard =0;
static int s_first_dialup = 1;
static int sim_pin =0;
#define ETH_OPERSTATE_PATH "/sys/class/net/wwan0/operstate"
#define SERVICE_LTE  "lteup"
#define PPP_ETH_PATH "wwan0"
#define POLL_PPP_SYSFS_RETRY	3
#define POLL_PPP_SYSFS_SECONDS	3
/*
extern int dhcp_get_results(const char *interface,
                     char *ipaddr,
                     char *gateway,
                     uint32_t *prefixLength,
                     char *dns[],
                     char *server,
                     uint32_t *lease,
                     char *vendorInfo,
                     char *domain,
                     char *mtu);
					

extern int dhcp_start(const char *interface);
extern int dhcp_stop(const char *interface);	
*/
#if 1
int setTimer(int seconds, int mseconds)
{
        struct timeval temp;
        temp.tv_sec = seconds;
        temp.tv_usec = mseconds;

	select(0, NULL, NULL, NULL, &temp);
        return 1;
}

static void *pppdStateLoop(void *param)
{
   // char value[64] = "";
	char value[PROPERTY_VALUE_MAX];
	char dns1[PROPERTY_VALUE_MAX];
	char dns2[PROPERTY_VALUE_MAX];
   // char dns1[64]="";
   // char dns2[64]="";
    s_pppd_exception = 0;
    LOGD("[%s]: begin pppd state monitor", __FUNCTION__);
    //while( ETIMEDOUT==pthread_cond_timeout_np(&s_waitcond, &s_waitmutex, 3000) )
    while(1 == setTimer(3,0))
    {
        property_get("net.gprs.ppp-exit", value, "");
        if(value[0])
        {
            LOGD("[%s] pppd: exception exited", __FUNCTION__);
            s_pppd_exception = 1;
		//	if(0 == modem_cmp(0x106C, 0x3718, NULL))
	          //  at_emulate_exit();
            RIL_requestTimedCallback (onDataCallListChanged, NULL, NULL);
            break;
        }
        value[0] = 0;

#if ENABLE_DNS_CHECK
        ///xxh:����DNS��ȡ����ȷ�����
        property_get("net.dns1",  dns1,  "");
        property_get("net.dns2",  dns2,  "");
        if(!strncmp(dns1,"10.11.12.13",strlen("10.11.12.13")) &&
            !strncmp(dns2,"10.11.12.14",strlen("10.11.12.14")))
        {
           // LOGD("[%s] DNS not correct ! pppd: exception exited", __FUNCTION__);
            //s_pppd_exception = 1;
           // RIL_requestTimedCallback (onDataCallListChanged, NULL, NULL);
              LOGD("[%s] the Server give worng DNS ,set to Generic", __FUNCTION__);
              property_set("net.dns1", "8.8.8.8");
              property_set("net.dns2", "8.8.8.8");
			  property_set("net.ppp0.dns1", "8.8.8.8");
			  property_set("net.ppp0.dns2", "8.8.8.8");
        }
#endif
    }

    LOGD("[%s] exit pppdStateLoop, s_pppd_exception=%d", __FUNCTION__, s_pppd_exception);
    
    return 0;
}
#endif
static void *checkTtyStateLoop(void *param)
{
	while(1 == setTimer(6,0)){
	if( access(s_current_modem->atchannel_index, 0) == 0 ){
		LOGD("TTY IS OK\n");
	}else{
		LOGD("TTY EXIT\n");
		kill_rild();
		}
	}
	return 0;
}
/*:
     1   support
     0   unsupport
 */
int is_support_modem_type(int type)
{
	int ret = SUPPORT_MODEM_TYPE_NONE;
	
	if(type == RADIO_TYPE_EVDO)
		ret = sSupportModemType&SUPPORT_MODEM_TYPE_EVDO;
	else if(type == RADIO_TYPE_WCDMA)
		ret = sSupportModemType&SUPPORT_MODEM_TYPE_WCMDA;
	else if(type == RADIO_TYPE_TDSCDMA)
		ret = sSupportModemType&SUPPORT_MODEM_TYPE_TDSCDMA;
	
	return ret?1:0;
}


/*
    ����:
        vid: һ��������������Ƚϸ�ֵ�ɸ�����
        pid: һ��������������Ƚϸ�ֵ�ɸ�����
        name: һ���ַ�����������Ƚϸ�ֵ�ɸ�NULL
    ��ͬ���� 0
    ��ͬ���� ��0
 */
int modem_cmp(int vid, int pid, const char* name)
{
    if( vid>=0 && s_current_modem->modem_info.vid != vid )
        return !0;
    if( pid>=0 && s_current_modem->modem_info.pid != pid )
        return !0;
    if( name!=NULL && strcmp(s_current_modem->modem_info.model_id,name) )
        return !0;
        
    return 0;
}

#define EATCHAR(x, c) for (; *(x) == (c); (x)++) ; // ȥ���ַ���x�����Ϊc���ַ�
/* -1:error. */
static int atox( const char * line, int f_base )
{
    int base = 10;
    char max = '9';
    int v = 0;

    EATCHAR(line, ' ');
    if(*line == 0) return 0;
    
    if( line[1] == 'x' || line[1] == 'X' ){
        base = 16;
        max = 'f';      /* F*/
        line += 2;
    }
    else if(f_base==16)
    {
        base = 16;
        max = 'f';      /* F*/
    }
    
    if( base == 10 ) {
            while( *line >= '0' && *line <= max ) {
                        v *= base ;
                        v += *line-'0';
                        line++;
            }
    } else {
            while( *line >= '0' && *line <= max ) {
                        v *= base ;
                        if( *line >= 'a' )
                                v += *line-'a'+10;
                        else if ( *line >= 'A' )
                                v += *line-'A'+10;
                        else
                                v += *line-'0';
                        line++;
                }
    }
    return v;
}

RIL_RadioType getCurModelRadio()
{
    return s_current_modem->radioType;
}

static const char* networkStatusToRilString(int state)
{
	switch(state){
		case 0: return("unknown");   break;
		case 1: return("available"); break;
		case 2: return("current");   break;
		case 3: return("forbidden"); break;
		default: return NULL;
	}
}

static void sleepMsec(long long msec)
{
    struct timespec ts;
    int err;

    ts.tv_sec = (msec / 1000);
    ts.tv_nsec = (msec % 1000) * 1000 * 1000;

    do {
        err = nanosleep (&ts, &ts);
    } while (err < 0 && errno == EINTR);
}

/*
    ����ȷ��modem�Ƿ����շ�ATָ������򷵻�0
    Ҫ��: ������ȥ��atָ����뷵��"OK"������˵��ʧ��
 */
static int check_at_ready(const char* at, int count)
{
	int err;
	ATResponse *p_response = NULL;
    int i;

    for (i = 0 ; i < count ; i++) {
    	err = at_send_command(at, &p_response);
    	if (err < 0 || p_response->success == 0) {
    		err = 1;
            sleepMsec(250);
    	}
        else
        {
            err = 0;
            break;
        }
    }

	at_response_free(p_response);

	return err;
}


/** do post-AT+CFUN=1 initialization */
/*
    ��ʱAT+CFUN=1������Ӧ�ж�SIM״̬��ֱ��SIM��״̬ΪREADY
 */
static void onRadioPowerOn()
{
// ����AT����
    if(check_at_ready("AT", 32))
    {
        LOGE("AT handshake failed!!!");
        return;
    }

// ��ѯSIM��״̬
    pollSIMState(NULL);
}

/** do post- SIM ready initialization */
/*
    ��SIM ready�󣬴�ʱ�ɽ��и���ĳ�ʼ��
 */
static void onSIMReady()
{
// ȷ��ģ���ͺ�
    if( sMmodemRematch )
    {
        // ��ѯmodem id
        char* modemID = getModelID();
        RLOGD("Current modem id is: %s", modemID);
        // ����modem id �Լ�AT channel��ȷ�� modem �ͺ�
        int match_count = rematchModem(modemID, s_current_modem->atchannel_index);
		
        RLOGD("[%s]: match model count=%d", __FUNCTION__, match_count);
        if( match_count != 1 || !is_support_modem_type(s_current_modem->radioType))
        {
            // û���ҵ�ƥ���modem����radio״̬ΪRADIO_STATE_UNAVAILABLE���ϲ㲻���·�ָ��
            RLOGE("Can not match modem!!!");
            setRadioState (RADIO_STATE_UNAVAILABLE);
            return;
        }
        // cmy@20110818: �Ѿ�ƥ����ɣ���sMmodemRematch��0
        sMmodemRematch = 0;
    }
    
    switch(s_current_modem->radioType)
    {
    case RADIO_TYPE_CDMA:
    case RADIO_TYPE_EVDO:
        isgsm = 0;
        break;
    case RADIO_TYPE_GSM:
    case RADIO_TYPE_WCDMA:
    case RADIO_TYPE_TDSCDMA:
    default:
        isgsm = 1;
        break;
    }

    LOGD("[%s] isgsm:%d", __FUNCTION__, isgsm);
//s    property_set("ril.radio.type", isgsm?"gsm":"cdma");

// modem ���г�ʼ��
    s_current_modem->modemDefFunc->initModem();
}
int hexCharToInt(char c)
{
	if (c >= '0' && c <= '9') 
		return (c - '0');
	if (c >= 'A' && c <= 'F') 
		return (c - 'A' + 10);
	if (c >= 'a' && c <= 'f') 
		return (c - 'a' + 10);
	return 0;
}


/************************************************
 Function:  hexStringToBytes
 Description:  
    16�����ַ�����ת��Ϊ�ֽ�����, "ABCD" ---> {0xAB,0xCD}
 Calls: 
 Called By: 
    request_send_ussd, request_sim_io, cdma_pdu_decode
 Input:
   16�����ַ�����,16�����ַ����鳤��
 Output:
    �ֽ�����
 Return:
    �ֽ����鳤��
 Others:    
**************************************************/
int hexStringToBytes(char * s,char *hs ,int len)
{	
	    int i;
        if (s == 0) return -1;

        int sz = strlen(s);

        for (i=0 ; i <sz ; i+=2) {
			if(i/2 >= len)
				return -1;
	 		hs[i/2] = (char) ((hexCharToInt(s[i]) << 4) 
	                                | hexCharToInt(s[i+1]));
        }
        return (sz+1) / 2;
}
static char *StrToUpper(char * str)
{
    if( NULL == str)
    {
		return NULL;
	}
    char *s=str;
	
    while(*s != '\0')
    {	
    	if(*s <= 'z' && *s >= 'a' )
    	{
            *s=*s-('a'-'A');
    	}
    	s++;
    }
    return str;
}


static void  requestSIM_IO(void *data, size_t datalen, RIL_Token t)
{
	int err;
	char *cmd = NULL;
	char *line;
	char  hs[20];
	unsigned short file_size; 
	RIL_SIM_IO_Response sr;
	ATResponse *p_response = NULL;

	memset(&sr, 0, sizeof(sr));
	
    if( NULL == data )
    	goto error;
   
     RIL_SIM_IO_v6 *p_args;
     p_args = (RIL_SIM_IO_v6 *)data;
	if(p_args->path != NULL){
		if((strlen(p_args->path)%4) != 0)
			goto error;
	}
	
    if(p_args->data != NULL)
    {
		p_args->data = StrToUpper(p_args->data);
    }
	
	/* FIXME handle pin2 */    
	// goto sim_io_error;
    //modified by wkf32792 begin for DTS2011062001722
	
    /* Begin to modify by hexiaokong kf39947 for DTS2011122605765 2012-01-13*/
    if (p_args->data == NULL) {
	   asprintf(&cmd, "AT+CRSM=%d,%d,%d,%d,%d,,\"%s\"", 
	   p_args->command, p_args->fileid,
	   p_args->p1, p_args->p2, p_args->p3, p_args->path);
	   
      if(p_args->fileid == 28476)
      {             
          sprintf(g_smspath,"%s",p_args->path);
          g_fileid = p_args->fileid;
      }
      
    } else {	   
	   asprintf(&cmd, "AT+CRSM=%d,%d,%d,%d,%d,\"%s\",\"%s\"",
	   p_args->command, p_args->fileid,
	   p_args->p1, p_args->p2, p_args->p3, p_args->data, p_args->path);   // p_args->p1��ȡ����Ӧʱ�䣬�ǲ��Ƕ�ril��Ӱ��
	   
       if(p_args->fileid == 28476)
       {          
           sprintf(g_smspath,"%s",p_args->path);
           g_fileid = p_args->fileid; 
       }
       
	}	//modified by wkf32792 end for DTS2011062001722
	/* End to modify by hexiaokong kf39947 for DTS2011122605765 2012-01-13*/
	
	err = at_send_command_singleline(cmd, "+CRSM:", &p_response);

	if (err < 0 || p_response->success == 0) {
		goto error;
	}

	line = p_response->p_intermediates->line;

	err = at_tok_start(&line);
	if (err < 0) goto error;

	err = at_tok_nextint(&line, &(sr.sw1));
	if (err < 0) goto error;

	err = at_tok_nextint(&line, &(sr.sw2));
	if (err < 0) goto error;

	if (at_tok_hasmore(&line)) {
		err = at_tok_nextstr(&line, &(sr.simResponse));
		if (err < 0) goto error;
		
		
		if(('6' == sr.simResponse[0]) 
			&& ('2' == sr.simResponse[1]) 
			&& 192 ==p_args->command 
			&& ('0' == sr.simResponse[6])
			&& ('5' == sr.simResponse[7]))
		{ //usim card
       
			//LOGD("USIM : sr.simResponse = %s,len=%d",sr.simResponse,strlen(sr.simResponse));
			
			hexStringToBytes(sr.simResponse,hs,15);

			sr.simResponse[0] = '0';
			sr.simResponse[1] = '0';
			sr.simResponse[2] = '0';
			sr.simResponse[3] = '0';
            //file_size = Record length * Number of records
			file_size = ((hs[6]<<8)+hs[7]) * hs[8];
			//copy file size
			sprintf(&(sr.simResponse[4]),"%04x",file_size);
			//copy file id
			sr.simResponse[8] = sr.simResponse[22];
			sr.simResponse[9] = sr.simResponse[23];
			sr.simResponse[10] = sr.simResponse[24];
			sr.simResponse[11] = sr.simResponse[25];
            //TYPE_EF RESPONSE_DATA_FILE_TYPE
			sr.simResponse[13] = '4';
            //EF_TYPE_LINEAR_FIXED RESPONSE_DATA_STRUCTURE
			sr.simResponse[26] = '0';
			sr.simResponse[27] = '1';
			//Record length
			sr.simResponse[28] = sr.simResponse[14];
			sr.simResponse[29] = sr.simResponse[15];
			sr.simResponse[30] = 0x0;//the end
			//LOGD("response simResponse = %s,len=%d",sr.simResponse,strlen(sr.simResponse));
			
		}
		
	}

	RIL_onRequestComplete(t, RIL_E_SUCCESS, &sr, sizeof(sr));
	at_response_free(p_response);
	free(cmd);
	return;
	
error:
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	if(p_response != NULL)
	{
		at_response_free(p_response);
	}
    free(cmd);
	cmd = NULL;
	return;
}

/*
    radio �� on/off Ӧ��ֻ���ڿ���/�ر� ����ģʽʱ�����õ�
 */
static void requestRadioPower(void *data, size_t datalen, RIL_Token t)
{
// onOff:  0 - OFF,  1 - ON
    int onOff;

    int err;
    ATResponse *p_response = NULL;

    assert (datalen >= sizeof(int *));
    onOff = ((int *)data)[0];

#ifdef DBG_1
    LOGD("[%s]: sState=%d  onOff=%d\n", __FUNCTION__, sState, onOff);
#endif

    if (onOff == 0 && sState != RADIO_STATE_OFF) {
        if (s_current_modem->inner == 0)
        {
            LOGD("[%s]: not support 'AT+CFUN=0'\n", __FUNCTION__);
         //   setRadioState(RADIO_STATE_SIM_NOT_READY);
             setRadioState(RADIO_STATE_OFF);
        }
        else
        {
            err = at_send_command("AT+CFUN=0", &p_response);
            if (err < 0 || p_response->success == 0) goto error;
            setRadioState(RADIO_STATE_OFF);
        }
    } else if (onOff > 0 && sState == RADIO_STATE_OFF) {
        //if( 0 == modem_cmp(0x19D2, 0xFFFE, "MC8630") )
           // err = at_send_command("AT+CFUN=3", &p_response);
       // else
            err = at_send_command("AT+CFUN=1", &p_response);
        if (err < 0|| p_response->success == 0) {
            // Some stacks return an error when there is no SIM,
            // but they really turn the RF portion on
            // So, if we get an error, let's check to see if it
            // turned on anyway

            if (isRadioOn() != 1) {
                goto error;
            }
        }
        LOGD("[%s]: sleep %ds befor open %s", __FUNCTION__, s_current_modem->initWaitTime, s_current_modem->atchannel_index);
        sleep(s_current_modem->initWaitTime);
        setRadioState(RADIO_STATE_ON);
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestOrSendDataCallList(RIL_Token *t);

static void onDataCallListChanged(void *param)
{
    requestOrSendDataCallList(NULL);
}

static void requestDataCallList(void *data, size_t datalen, RIL_Token t)
{
    requestOrSendDataCallList(&t);
}

#if (RIL_VERSION >= 6)
static void requestOrSendDataCallList(RIL_Token *t)
{
  if(!modem_cmp(0x05c6,0x9025,NULL)){
  	ATResponse *p_response;
	RIL_Data_Call_Response_v11 *responses;
	ATLine *p_cur;
	int err;
	int n = 1;
	responses = alloca(sizeof(RIL_Data_Call_Response_v11));
	responses[0].status = 0;
	responses[0].suggestedRetryTime = -1;
	responses[0].cid = 1;
	responses[0].active = 1;
	responses[0].type = "IP";
	responses[0].ifname = "lte0";
	responses[0].addresses = "0.0.0.0";//ip_address;
	responses[0].dnses = "";
	responses[0].gateways = "";
	responses[0].pcscf = "";
	responses[0].mtu = 1500;
	if (t != NULL)
		RIL_onRequestComplete(*t, RIL_E_SUCCESS, responses,
			n * sizeof(RIL_Data_Call_Response_v11));
		else
			RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED,
				responses, n * sizeof(RIL_Data_Call_Response_v11));
  }else
	{ATResponse *p_response;
    RIL_Data_Call_Response_v11 *responses;
    ATLine *p_cur;
    int err;
    int n = 0;
    char *out;
//XXH:CID ���ϴ����ӵĲ�һ�£������ϲ��Ͽ��ϴ����ӣ��ٲ���
//XXH:MF190 AT+CGACT?�ϱ� +CGACT: 1,0���������ӶϿ�
	if (isgsm && modem_cmp(0x19D2, 0x0167, "MF820")
                  && modem_cmp(0x12d1, 0x1506, "E303")
                  && modem_cmp(0x12d1, 0x1506, "E303C")
                  && modem_cmp(0x12d1, 0x1506, "E3131")
                  && modem_cmp(0x12D1, 0x1506, "E173")
                  && modem_cmp(0x12d1, 0x1001, "E122")
                  && modem_cmp(0x12D1, 0x1506, "E357")
                  && modem_cmp(0x12d1, 0x0117, "MF190")
                  && modem_cmp(0x12d1, 0x1001, "K3772")
                  && modem_cmp(0x1782, 0x0002, "V1.0.1-B7")
                  && modem_cmp(0x12d1, 0x140c, "E261")
                  && modem_cmp(0x21F5, 0x2012, "SPW9P")
				  && !modem_cmp(0x19D2, 0x2003, "MF667")
				  && !modem_cmp(0x19D2, 0x0117, "MF667")
				  && !modem_cmp(0x19D2, 0x1405, "MF667")) {
        err = at_send_command_multiline ("AT+CGACT?", "+CGACT:", &p_response);
        if (err != 0 || p_response->success == 0) {
            if (t != NULL)
                RIL_onRequestComplete(*t, RIL_E_GENERIC_FAILURE, NULL, 0);
            else
                RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED,
                                          NULL, 0);
            return;
        }

        for (p_cur = p_response->p_intermediates; p_cur != NULL;
             p_cur = p_cur->p_next)
            n++;

        responses = alloca(n * sizeof(RIL_Data_Call_Response_v11));

        int i;
        for (i = 0; i < n; i++) {
        
	    responses[i].status = -1;
        responses[i].suggestedRetryTime = -1;
        responses[i].cid = -1;
        responses[i].active = -1;
        responses[i].type = "";
        responses[i].ifname = "";
        responses[i].addresses = "";
        responses[i].dnses = "";
        responses[i].gateways = "";
	responses[i].pcscf = "";
	responses[i].mtu = 0;
        }

        RIL_Data_Call_Response_v11 *response = responses;
    for (p_cur = p_response->p_intermediates; p_cur != NULL;
         p_cur = p_cur->p_next) {
        char *line = p_cur->line;

        err = at_tok_start(&line);
        if (err < 0)
            goto error;

        err = at_tok_nextint(&line, &response->cid);
        if (err < 0)
            goto error;
        err = at_tok_nextint(&line, &response->active);
        if (err < 0)
            goto error;
        if(response->active == 1 ||response->active==0){
				ALOGD("Foce active to 2.............");
			   response->active = 2;
		}
        response++;
	
    }

    at_response_free(p_response);

    err = at_send_command_multiline ("AT+CGDCONT?", "+CGDCONT:", &p_response);
    if (err != 0 || p_response->success == 0) {
        if (t != NULL)
            RIL_onRequestComplete(*t, RIL_E_GENERIC_FAILURE, NULL, 0);
        else
            RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED,
                                      NULL, 0);
        return;
    }

        for (p_cur = p_response->p_intermediates; p_cur != NULL;
             p_cur = p_cur->p_next) {
          char *line = p_cur->line;
        int cid;

        err = at_tok_start(&line);
        if (err < 0)
            goto error;

        err = at_tok_nextint(&line, &cid);
        if (err < 0)
            goto error;

        for (i = 0; i < n; i++) {
            if (responses[i].cid == cid)
                break;
        }

        if (i >= n) {
            /* details for a context we didn't hear about in the last request */
            continue;
        }

        // Assume no error
        responses[i].status = 0;

        // type
        err = at_tok_nextstr(&line, &out);
        if (err < 0)
            goto error;
        responses[i].type = alloca(strlen(out) + 1);
        strcpy(responses[i].type, out);

        // APN ignored for v5
        err = at_tok_nextstr(&line, &out);
        if (err < 0)
            goto error;

        responses[i].ifname = alloca(strlen(PPP_TTY_PATH) + 1);
        strcpy(responses[i].ifname, PPP_TTY_PATH);

        //xxh: ���ڲ���3G dongel AT+CGDCONT?����ص�pdp addresΪ��null�������ϲ㱨���Ӵ���
        
        err = at_tok_nextstr(&line, &out);
        if (err < 0)
           goto error;
         if ('\0' == out[0])
        {   
            responses[i].addresses = (char *)alloca(strlen("0.0.0.0")+1);
            strcpy(responses[i].addresses, "0.0.0.0");
        } else {
            responses[i].addresses = alloca(strlen(out) + 1);
            strcpy(responses[i].addresses, out);
	}
    }
        at_response_free(p_response);
	} else {
		//CDMA
		n = 1;
		responses = alloca(sizeof(RIL_Data_Call_Response_v11));
        	responses[0].status = 0;
        	responses[0].suggestedRetryTime = -1;
        	responses[0].cid = 1;
        	responses[0].active = 1;
        	responses[0].type = "IP";
        	responses[0].ifname = "ppp0";
        	responses[0].addresses = "0.0.0.0";//ip_address;
		responses[0].dnses = "";
		responses[0].gateways = "";
		responses[0].pcscf = "";
		responses[0].mtu = 1500;
	}
	
    if(s_pppd_exception)
    {
    /*  ����active=0����ʹ�ϲ�֪��pppd�Ѿ����Զ��˳����Ӷ�����״̬�����·������������ӡ�
     */
        LOGD("data state force set to 0");
        int i;
        for(i=0; i<n; i++)
            responses[i].active = 0;
    }
    
    if (t != NULL)
        RIL_onRequestComplete(*t, RIL_E_SUCCESS, responses,
                              n * sizeof(RIL_Data_Call_Response_v11));
    else
        RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED,
                                  responses,
                                  n * sizeof(RIL_Data_Call_Response_v11));

    return;

error:
    if (t != NULL)
        RIL_onRequestComplete(*t, RIL_E_GENERIC_FAILURE, NULL, 0);
    else
        RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED,
                                  NULL, 0);

    at_response_free(p_response);
}
}
#else
static void requestOrSendDataCallList(RIL_Token *t)
{
    ATResponse *p_response;
    RIL_Data_Call_Response *responses;
    ATLine *p_cur;
    int err;
    int n = 0;
    char *out;

	if (isgsm && modem_cmp(0x19D2, 0x0167, "MF820")) {
        err = at_send_command_multiline ("AT+CGACT?", "+CGACT:", &p_response);
        if (err != 0 || p_response->success == 0) {
            if (t != NULL)
                RIL_onRequestComplete(*t, RIL_E_GENERIC_FAILURE, NULL, 0);
            else
                RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED,
                                          NULL, 0);
            return;
        }

        for (p_cur = p_response->p_intermediates; p_cur != NULL;
             p_cur = p_cur->p_next)
            n++;

        responses = alloca(n * sizeof(RIL_Data_Call_Response));

        int i;
        for (i = 0; i < n; i++) {
            responses[i].cid = -1;
            responses[i].active = -1;
            responses[i].type = "";
            responses[i].apn = "";
            responses[i].address = "";
        }

        RIL_Data_Call_Response *response = responses;
        for (p_cur = p_response->p_intermediates; p_cur != NULL;
             p_cur = p_cur->p_next) {
            char *line = p_cur->line;

            err = at_tok_start(&line);
            if (err < 0)
                goto error;

            err = at_tok_nextint(&line, &response->cid);
            if (err < 0)
                goto error;

            err = at_tok_nextint(&line, &response->active);
            if (err < 0)
                goto error;

            response++;
        }

        at_response_free(p_response);

        err = at_send_command_multiline ("AT+CGDCONT?", "+CGDCONT:", &p_response);
        if (err != 0 || p_response->success == 0) {
            if (t != NULL)
                RIL_onRequestComplete(*t, RIL_E_GENERIC_FAILURE, NULL, 0);
            else
                RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED,
                                          NULL, 0);
            return;
        }

        for (p_cur = p_response->p_intermediates; p_cur != NULL;
             p_cur = p_cur->p_next) {
            char *line = p_cur->line;
            int cid;
            char *type;
            char *apn;
            char *address;


            err = at_tok_start(&line);
            if (err < 0)
                goto error;

            err = at_tok_nextint(&line, &cid);
            if (err < 0)
                goto error;

            for (i = 0; i < n; i++) {
                if (responses[i].cid == cid)
                    break;
            }

            if (i >= n) {
                /* details for a context we didn't hear about in the last request */
                continue;
            }

            err = at_tok_nextstr(&line, &out);
            if (err < 0)
                goto error;

            responses[i].type = alloca(strlen(out) + 1);
            strcpy(responses[i].type, out);

            err = at_tok_nextstr(&line, &out);
            if (err < 0)
                goto error;

            responses[i].apn = alloca(strlen(out) + 1);
            strcpy(responses[i].apn, out);

            err = at_tok_nextstr(&line, &out);
            if (err < 0)
                goto error;

            responses[i].address = alloca(strlen(out) + 1);
            strcpy(responses[i].address, out);
        }

        at_response_free(p_response);
	} else {
		//CDMA
		n = 1;
		responses = alloca(sizeof(RIL_Data_Call_Response));	
		responses[0].cid = 1;
		responses[0].active = 1;
		responses[0].address = "";
		responses[0].type = "internet";
		responses[0].apn = "";
		
	}
    if(s_pppd_exception)
    {
    /*  ����active=0����ʹ�ϲ�֪��pppd�Ѿ����Զ��˳����Ӷ�����״̬�����·������������ӡ�
     */
        LOGD("data state force set to 0");
        int i;
        for(i=0; i<n; i++)
            responses[i].active = 0;
    }
    
    if (t != NULL)
        RIL_onRequestComplete(*t, RIL_E_SUCCESS, responses,
                              n * sizeof(RIL_Data_Call_Response));
    else
        RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED,
                                  responses,
                                  n * sizeof(RIL_Data_Call_Response));

    return;

error:
    if (t != NULL)
        RIL_onRequestComplete(*t, RIL_E_GENERIC_FAILURE, NULL, 0);
    else
        RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED,
                                  NULL, 0);

    at_response_free(p_response);
}
#endif
/*
    cmy: ��ЩDongle�Ĳ���ATָ��ķ���ֵ����û��ǰ׺��Ҳ������ǰ׺
    ����:
        cmd: ����� AT+CGMR
        prex: ������ķ���ֵ�������ǰ׺����ǰ׺�����Ǹ�ֵ���� +CGMR:
        try_count: �������ʱ���Դ���
        
    ����ֵ:
          ����ʱ����NULL
     
 */
static char* getCommandResponse(const char* cmd, const char* prex, int try_count)
{
	int err = 0;
	ATResponse *p_response = NULL;
	char * response = NULL;
	char* line = NULL;
	char* result = NULL;

    while( try_count-- )
    {
        at_response_free(p_response);
        
        err = at_send_command_singleline(cmd, "", &p_response);
    	
    	if (err < 0 || p_response->success == 0)
    	    goto error;

    	line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if( err < 0)
            line = p_response->p_intermediates->line;
//        else if(strncmp(p_response->p_intermediates->line, prex, strlen(prex)))
        else if( !strstr(p_response->p_intermediates->line, prex) )
            continue;

    	err = at_tok_nextstr(&line, &response);
    	if (err < 0)
    	    continue;
		//ʵ����0x21f5, 0x1101 dongle���ص�CGMMΪ�գ������ϲ㴦����������ֶ���ֵ������Ϣ
		if(0 == modem_cmp(0x21f5, 0x1101, NULL))
			{
			 response="StrongRsing";
			}
        asprintf(&result, "%s", response);
        LOGD("[%s]: result=%s", __FUNCTION__, result);
        break;
    }

    if( try_count<0 )
        goto error;

	at_response_free(p_response);
	return result;

error:
    LOGD("[%s]: error=%d", __FUNCTION__, err);
	at_response_free(p_response);
	return NULL;
}

#if 1
/*
    cmy: �ú����������� SIM Ready ֮��
 */
static char* getModelID()
{
/*
 * cmy: Vtion_E1916ģ�飬��ģ���"AT+CGMM"����ķ��ز�����ǰ׺.
    +CGMM: E1916
    E1916

   @20101206: �ڲ�ѯmodem idʱ�������յ������������ϱ���Ŀǰ�ö�λ�ȡmodem id�ķ�������ȡ����ȷ��ֵ
 */
// cmy@20120105: ����modem��֧��AT+CGMM�������֧��AT+GMM
   if(!modem_cmp(0x05c6,0x9025,NULL)){

    int err;   
	char * responseStr = NULL;   
	ATResponse *p_response = NULL;   
	const char *cmd;   
	const char *prefix;   
	char *line, *p;   
	int commas;    
	int skip;    
	int count = 4;    
	err = at_send_command_singleline("AT+CGMM", "", &p_response);    
	if (err < 0 || !p_response->success) 
		goto error;   
	responseStr = p_response->p_intermediates->line;    
	

error:	
			   free(responseStr);

	   return responseStr;


   }else{
    char * response = NULL;
    response = getCommandResponse("AT+CGMM", "GMM:", 6);
    if(response == NULL)
        response = getCommandResponse("AT+GMM", "GMM:", 5);
    return response;
   	}

  
}
#else
/*
    cmy: ��ȡģ���ͺſ����޷�Ӧ��
         �ɹ����� model id
         ʧ�ܷ��� NULL
 */
static char* getModelID()
{
/*
 * cmy: Vtion_E1916ģ�飬��ģ���"AT+CGMM"����ķ��ز�����ǰ׺.
    +CGMM: E1916
    E1916

   ע��: ��Щģ����û�в�SIM��ʱ���޷���ȡģ���ͺ�
   TODO: ���޷���ȡģ���ͺ�ʱ���ɷ�������CPIN����ѯSIM��״̬

   @20101206: �ڲ�ѯmodem idʱ�������յ������������ϱ���Ŀǰ�ö�λ�ȡmodem id�ķ�������ȡ����ȷ��ֵ
 */
    char * response = NULL;
    response = getCommandResponse("AT+CGMM", "GMM:", 5);

    if( !response )
	{
        if (getSIMStatus() == SIM_ABSENT) {
            // ȱ��SIM��
            LOGD("[%s]: Sim card not inserted!", __FUNCTION__);
            setRadioState(RADIO_STATE_SIM_LOCKED_OR_ABSENT);
        }
	}

	return response;
}
#endif
static void requestBaseBandVersionT(int request, void *data,  
                      size_t datalen, RIL_Token t){  

int err;    
char * responseStr;    
ATResponse *p_response = NULL; 
const char *cmd;    
const char *prefix;    
char *line, *p; 
int commas;    
int skip;    
int count = 4;   
err = at_send_command_singleline("AT+GMR", "", &p_response);  
if (err < 0 || !p_response->success) goto error;   
responseStr = p_response->p_intermediates->line;   
RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, sizeof(responseStr));

error:  
	free(responseStr);}

static void requestBasebandVersion(void *data, size_t datalen, RIL_Token t)
{
    char* model_id = getModelID();
    if( model_id != NULL )
    {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, model_id, sizeof(char *));
        free(model_id);
    }
    else
    {
    	LOGE("ERROR: requestBasebandVersion failed\n");
    	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
}

// �ɹ����� >= 0
// ʧ�ܷ��� < 0
static int queryNetworkSelectionMode()
{
    int err;
    ATResponse *p_response = NULL;
    int response = 0;
    char *line;

    LOGD("Enter queryNetworkSelectionMode");

	// AT+COPS?     Ask for current Mode Preference
    err = at_send_command_singleline("AT+COPS?", "+COPS:", &p_response);

    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);

    if (err < 0) {
        goto error;
    }

    err = at_tok_nextint(&line, &response);

    if (err < 0) {
        goto error;
    }

    LOGD("response=%d", response);

    return response;
    
error:
    at_response_free(p_response);
    return -1;
}

/* cmy: ��ѯ��ǰѡ�������
    AT+COPS?
    
    +COPS: 0    �Զ�����ģʽ��δѡ������
    or
    +COPS: 1,2,"46000",2    �ֶ�����ģʽ��ѡ��������
    +COPS: 0,2,"46000",2    �Զ�����ģʽ��ѡ��������
 */
static void requestQueryNetworkSelectionMode(
                void *data, size_t datalen, RIL_Token t)
{
    int response = 0;

    LOGD("Enter requestQueryNetworkSelectionMode");
/*
	if(isgsm) { //this command conflicts with the network status command
	    response = queryNetworkSelectionMode();
	    if( response < 0 ) goto error;
	}
*/
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));
    return;
error:
    LOGE("requestQueryNetworkSelectionMode must never return error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}
//adb by xxh 2103-5-10
char *getFirstElementValue(const char* document,
                                  const char* elementBeginTag,
                                  const char* elementEndTag,
                                  char** remainingDocument)
{
    char* value = NULL;
    char* start = NULL;
    char* end = NULL;

    if (document != NULL && elementBeginTag != NULL && elementEndTag != NULL)
    {
        start = strstr(document, elementBeginTag);
        if (start != NULL)      
        {
            end = strstr(start, elementEndTag);
            if (end != NULL)
            {
                int n = strlen(elementBeginTag);
                int m = end - (start + n);
                value = (char*) malloc((m + 1) * sizeof(char));
                strncpy(value, (start + n), m);
                value[m] = (char) 0;
                
                /* Optional, return a pointer to the remaining document,
                   to be used when document contains many tags with same name. */
                if (remainingDocument != NULL)
                {
                    *remainingDocument = end + strlen(elementEndTag);
                }
            }
        }
    }
    return value;
}

void requestQueryAvailableNetworksxxh(void *data, size_t datalen, RIL_Token t)
{

    int err = 0;
    ATResponse *atresponse = NULL;
    const char *statusTable[] =
        { "unknown", "available", "current", "forbidden" };
    char **responseArray = NULL;
    char *p;
    int n = 0;
    int i = 0;

    err = at_send_command_multiline("AT+COPS=?", "+COPS:", &atresponse);
    if (err < 0 || 
        atresponse->success == 0 || 
        atresponse->p_intermediates == NULL)
        goto error;

    p = atresponse->p_intermediates->line;
    while (*p != '\0') {
        if (*p == '(')
            n++;
	if(*p == ',' && *(p+1) == ',')
		break;
        p++;
    }

    responseArray = alloca(n * 4 * sizeof(char *));

    p = atresponse->p_intermediates->line;

    for (i = 0; i < n; i++) {
        int status = 0;
        char *line = NULL;
        char *s = NULL;
        char *longAlphaNumeric = NULL;
        char *shortAlphaNumeric = NULL;
        char *numeric = NULL;
        char *remaining = NULL;
        char *GSMType="-2G";
	char *UTRANType="-3G";
        char *networkType = NULL;


        s = line = getFirstElementValue(p, "(", ")", &remaining);
        p = remaining;

        if (line == NULL) {
            LOGE("Null pointer while parsing COPS response. This should not happen.");
            break;
        }
        err = at_tok_nextint(&line, &status);
        if (err < 0)
            goto error;

        err = at_tok_nextstr(&line, &longAlphaNumeric);
        if (err < 0)
            goto error;
     
        err = at_tok_nextstr(&line, &shortAlphaNumeric);
        if (err < 0)
            goto error;

        err = at_tok_nextstr(&line, &numeric);
        if (err < 0)
            goto error;
         
        err = at_tok_nextstr(&line,&networkType);
	if(err<0)
	     goto error;

         if(strcmp(networkType,"2")==0)	 {	 	
         strcat(longAlphaNumeric,UTRANType);		
         LOGD("the operator is %s",longAlphaNumeric);	 
          }	 
           else if((strcmp(networkType,"1")==0) || (strcmp(networkType,"0")==0))	 
           {	
               	strcat(longAlphaNumeric,GSMType);
		LOGD("the operator is %s",longAlphaNumeric);
        }
        responseArray[i * 4 + 0] = alloca(strlen(longAlphaNumeric) + 1);
        strcpy(responseArray[i * 4 + 0], longAlphaNumeric);
    
        responseArray[i * 4 + 1] = alloca(strlen(shortAlphaNumeric) + 1);
        strcpy(responseArray[i * 4 + 1], shortAlphaNumeric);

        responseArray[i * 4 + 2] = alloca(strlen(numeric) + 1);
        strcpy(responseArray[i * 4 + 2], numeric);

        free(s);

        if (responseArray[i * 4 + 0] && strlen(responseArray[i * 4 + 0]) == 0) {
            responseArray[i * 4 + 0] = alloca(strlen(responseArray[i * 4 + 2])
                                              + 1);
            strcpy(responseArray[i * 4 + 0], responseArray[i * 4 + 2]);
        }

        if (responseArray[i * 4 + 1] && strlen(responseArray[i * 4 + 1]) == 0) {
            responseArray[i * 4 + 1] = alloca(strlen(responseArray[i * 4 + 2]) + 1);
            strcpy(responseArray[i * 4 + 1], responseArray[i * 4 + 2]);
        }

        responseArray[i * 4 + 3] = alloca(strlen(statusTable[status]) + 1);
        sprintf(responseArray[i * 4 + 3], "%s", statusTable[status]);
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseArray,
                          i * 4 * sizeof(char *));

finally:
    at_response_free(atresponse);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    goto finally;
}

/* cmy: ��ѯ��������
    AT+COPS=?
    +COPS: (2,"�й��ƶ�","CMCC","46000",2),(1,"�й��ƶ�","CMCC","46000",0),(3,"�й���ͨ","CUCC","46001"),,(0-4),(0-2)
 */
#define OPS_RET    4
static void requestQueryAvailableNetworks(void *data, size_t datalen, RIL_Token t)
{
	/* We expect an answer on the following form:
	   +COPS: (2,"AT&T","AT&T","310410",0),(1,"T-Mobile ","TMO","310260",0)
	   +COPS: (2,"CHN-CUGSM","CU-GSM","46001",2),(1,"3 HK","3 HK","45403",2),(3,"CSL","CSL","45400",2),(3,"PCCW","PCCW","45419",2),(1,"SmarToneVodafone","SMC-Voda","45406",2),
	 */
    
	int err, operators, i=0, skip, status, rat;
	ATResponse *p_response = NULL;
	char * c_skip, *line, *p = NULL;
	char ** response = NULL;

// cmy: CDMA not support 'COPS' commands
    if(!isgsm)
    {
    	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    	return;
    }

// ��ѯ��Ӫ�̵���������ʱ���൱��������źŲ��õĻ�������ʱ������
	err = at_send_command_singleline_t("AT+COPS=?", "+COPS:", 80000, &p_response);

	if (err < 0 || p_response->success == 0) goto error;

	line = p_response->p_intermediates->line;

	err = at_tok_start(&line);
	if (err < 0) goto error;

	/* Count number of '(' in the +COPS response to get number of operators*/
	operators = 0;
	for (p = line ; *p != '\0' ;p++) {
		if (*p == '(') operators++;
	}

#ifdef DBG_1
    LOGD("[%s] operators=%d", __FUNCTION__, operators);
#endif
	response = (char **)alloca(operators * 5 * sizeof(char *));

	for (i = 0 ; i < operators ; i++ )
	{
		err = at_tok_nextstr(&line, &c_skip);
		if (err < 0) goto error;
		if (strcmp(c_skip,"") == 0)
		{
			operators = i;
#ifdef DBG_1
            LOGD("[%s] break in %d", __FUNCTION__, i);
#endif
			continue;
		}
        // stat
		status = atoi(&c_skip[1]);
		response[i*OPS_RET+3] = (char*)networkStatusToRilString(status);

        // long format alphanumeric
		err = at_tok_nextstr(&line, &(response[i*OPS_RET+0]));
		if (err < 0) goto error;

        // short format alphanumeric
		err = at_tok_nextstr(&line, &(response[i*OPS_RET+1]));
		if (err < 0) goto error;

        // numeric
		err = at_tok_nextstr(&line, &(response[i*OPS_RET+2]));
		if (err < 0) goto error;

		err = at_tok_nextstr(&line, &c_skip);
		if (err < 0) goto error;
                LOGD("[%s] act is %s", __FUNCTION__, c_skip);
// cmy: ��֧������
        if(response[i*OPS_RET+0]!=NULL )
        {
        	if( !strcmp(response[i*OPS_RET+0], "�й��ƶ�") )
        	    response[i*OPS_RET+0] = "China Mobile";
        	else if( !strcmp(response[i*OPS_RET+0], "�й�����") )
        	    response[i*OPS_RET+0] = "China Telecom";
        	else if( !strcmp(response[i*OPS_RET+0], "�й���ͨ") )
        	    response[i*OPS_RET+0] = "China Unicom";
        }
	}

	RIL_onRequestComplete(t, RIL_E_SUCCESS, response, (operators * OPS_RET * sizeof(char *)));
	at_response_free(p_response);
	return;

error:
    if(i>0)
    {// cmy: ����л����һ������Ӫ�̣������Ǵ���
        LOGD("[%s] get oper count=%d", __FUNCTION__, i);
        RIL_onRequestComplete(t, RIL_E_SUCCESS, response, (i * 5 * sizeof(char *)));
    }
    else
    {
    	LOGE("ERROR - requestQueryAvailableNetworks() failed");
    	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
    
	at_response_free(p_response);
}

static void requestGetPreferredNetworkType(void *data, size_t datalen, RIL_Token t)
{
    int response = 0;
    response = s_current_modem->extATFunc->getPrefNetworkType();
    if(response < 0)
    {
    	LOGE("ERROR: requestGetPreferredNetworkType() failed\n");
    	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
    else
    {
        LOGD("[%s]: return network mode=%d", __FUNCTION__, response);
    	RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));
    }

    return;
}

/**
 * RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE
 *
 * Requests to set the preferred network type for searching and registering
 * (CS/PS domain, RAT, and operation mode)
 *
 * "data" is int *
 *
 * ((int *)data)[0] is == 0 for GSM/WCDMA (WCDMA preferred)
 * ((int *)data)[0] is == 1 for GSM only
 * ((int *)data)[0] is == 2 for WCDMA only
 * ((int *)data)[0] is == 3 for GSM/WCDMA (auto mode, according to PRL)
 * ((int *)data)[0] is == 4 for CDMA and EvDo (auto mode, according to PRL)
 * ((int *)data)[0] is == 5 for CDMA only
 * ((int *)data)[0] is == 6 for EvDo only
 * ((int *)data)[0] is == 7 for GSM/WCDMA, CDMA, and EvDo (auto mode, according to PRL)
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
 *  MODE_NOT_SUPPORTED
 */
static void requestSetPreferredNetworkType(void *data, size_t datalen, RIL_Token t)
{
    int err, rat;
	assert (datalen >= sizeof(int *));
	rat = ((int *)data)[0];

    LOGD("[%s]: new Network Type: %d", __FUNCTION__, rat);
   
    //kill_pppd(25);
    /* Set new preferred network type */
    err = s_current_modem->extATFunc->setPrefNetworkType(rat);
    if(err < 0)
    {
        if(err == -1) goto error_not_support;
        else goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, sizeof(int));
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    return;
error_not_support:
    RIL_onRequestComplete(t, RIL_E_MODE_NOT_SUPPORTED, NULL, 0);
    return;
}

static void requestQueryFacilityLock(void *data, size_t datalen, RIL_Token t)
{
	int err, rat, response;
	ATResponse *p_response = NULL;
	char * cmd = NULL;
	char * line = NULL;
	char * facility_string = NULL;
	char * facility_password = NULL;
	char * facility_class = NULL;

	assert (datalen >=  (3 * sizeof(char **)));

	facility_string   = ((char **)data)[0];
	facility_password = ((char **)data)[1];
	facility_class    = ((char **)data)[2];

	LOGD("FACILITY: %s", facility_string);
	LOGD(" facility_string=%s   facility_password=%s  facility_class=%s ",facility_string, facility_password, facility_class);
//	asprintf(&cmd, "AT+CLCK=\"%s\",2,\"%s\",%s", facility_string, facility_password, facility_class);
	asprintf(&cmd, "AT+CLCK=\"%s\",2", facility_string);
	
	err = at_send_command_singleline(cmd,"+CLCK:", &p_response);
	free(cmd);
	if (err < 0 || p_response->success == 0){
		goto error;
	}

	line = p_response->p_intermediates->line;

	err = at_tok_start(&line);

	if (err < 0) {
		goto error;
	}

	err = at_tok_nextint(&line, &response);

	if (err < 0) {
		goto error;
	}

	RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));
	at_response_free(p_response);
	return;

error:
	at_response_free(p_response);
	LOGE("ERROR: requestQueryFacilityLock() failed\n");
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestGetCurrentCalls(void *data, size_t datalen, RIL_Token t)
{
    int countValidCalls;
    RIL_Call **pp_calls;

	if(currentState() != RADIO_STATE_ON){
		/* Might be waiting for SIM PIN */
		RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
	}

/*  CMY: ���Է��֣��ڲ�������ʱ�������Եػ����������棬ϵ�������ķ��ؽ����
    ĿǰMID��δ֧������ͨ�����ܣ���ֱ�ӷ��ؿյĽ�����ϲ㣬�Խ����BUG
 */
    countValidCalls = 0;
    pp_calls = NULL;
    RIL_onRequestComplete(t, RIL_E_SUCCESS, pp_calls,
            countValidCalls * sizeof (RIL_Call *));
    return;
}

// ����ֵ:  0 - ����״̬���ı�
//          1 - ����״̬���ı�
static int queryRestrictedState()
{
    int restricted_state;

    LOGD("[%s] === ENTER ===\n", __FUNCTION__);
    
    if(!isgsm || !modem_cmp(0x0E8D, 0x00A2, "MTK2"))// C���޷���ѯ���޷���״̬
        restricted_state = RIL_RESTRICTED_STATE_NONE;
    else
        restricted_state = s_current_modem->extATFunc->getRestrictedState();

    if(restricted_state >= 0 && restricted_state != s_restricted_state)
    {
        LOGD("[%s]: restricted state: old=%d, new=%d", __FUNCTION__, s_restricted_state, restricted_state);
        s_restricted_state = restricted_state;
// cmy:���� AST �����÷������޵Ĺ���
        RIL_onUnsolicitedResponse(RIL_UNSOL_RESTRICTED_STATE_CHANGED,
                                  &s_restricted_state,
                                  sizeof(int) );
        return 1;
    }

    return 0;
}

/*
    ��ѯע��״̬
    ����:
        regInfo:    ״̬��Ϣͨ���ò������أ�ͨ���ɵ��÷�����һ�� int[4]������
        count:      ״̬��Ϣ����
    ����ֵ:
        0:          �ɹ�
        ��0:        ʧ��
 */
static int getRegistrationInfo(int *regInfo, int* count)
{
    int err;
    int* response = regInfo;
    ATResponse *p_response = NULL;
    const char *cmd;
    const char *prefix;
    char *line, *p;
    int commas;
    int skip;
    *count = 3;

/*
   RIL_REQUEST_GPRS_REGISTRATION_STATE: ����ֵ�У�responseStr[3]�����������
   
    ������������:
    1���ڳ�ʼ����ʼ������2G
    2���ϲ��CDMA��GSM�������磬
 */
// cmy@20111228: ����ѯ����ע��״̬
// cmy@20120419: ��ͼ��ѯλ�����õ�����ע��״̬�ķ���ֵ
//                  ��E1750��ѯ����״̬û����
    if (!isgsm || !modem_cmp(0x1234, 0x0033, "LCR 3.0.0")) {
    // cmy: for CDMA
        regInfo[0] = 1;
        regInfo[1] = -1;
        regInfo[2] = -1;

        return 0;
    }
    // data registration state
    cmd = "AT+CGREG?";
    prefix = "+CGREG:";
    if( !modem_cmp(0x12D1, 0x14AC, "E153") || 
        !modem_cmp(0x12D1, 0x1003, "E156G") ||
        !modem_cmp(0x12D1, 0x1C05, "E173")  ||
        !modem_cmp(0x12d1, 0x1506,"E303")   ||
        !modem_cmp(0x12d1, 0x1506,"E303C")  ||
        !modem_cmp(0x12d1,0x0117,"MF190")   || 
        !modem_cmp(0x12d1, 0x1506,"E3131")  ||
        !modem_cmp(0x20A6, 0x1105,"HSUPA")  ||
        !modem_cmp(0x21f5, 0x1101, NULL)    ||
	!modem_cmp(0x05C6, 0x9000, NULL))
    {
        cmd = "AT+CREG?";
        prefix = "+CREG:";
    }

    err = at_send_command_singleline(cmd, prefix, &p_response);

    if (err != 0 || p_response->success==0) goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    /* Ok you have to be careful here
     * The solicited version of the CREG response is
     * +CREG: n, stat, [lac, cid]
     * and the unsolicited version is
     * +CREG: stat, [lac, cid]
     * The <n> parameter is basically "is unsolicited creg on?"
     * which it should always be
     *
     * Now we should normally get the solicited version here,
     * but the unsolicited version could have snuck in
     * so we have to handle both
     *
     * Also since the LAC and CID are only reported when registered,
     * we can have 1, 2, 3, or 4 arguments here
     *
     * finally, a +CGREG: answer may have a fifth value that corresponds
     * to the network type, as in;
     *
     *   +CGREG: n, stat [,lac, cid [,networkType]]
     */

    /* count number of commas */
    commas = 0;
    for (p = line ; *p != '\0' ;p++) {
        if (*p == ',') commas++;
    }

    switch (commas) {
    case 0: /* +CREG: <stat> */
        err = at_tok_nextint(&line, &response[0]);
        if (err < 0) goto error;
        response[1] = -1;
        response[2] = -1;
        break;

    case 1: /* +CREG: <n>, <stat>   +CGREG: <n>, <stat>*/
        err = at_tok_nextint(&line, &skip);
        if (err < 0) goto error;
        err = at_tok_nextint(&line, &response[0]);
        if (err < 0) goto error;  
        
        if(!modem_cmp(0x05C6, 0x6000, "HSDPA Modem")
           || !modem_cmp(0x12D1, 0x1001, "E1750")
           || !modem_cmp(0x12D1, 0x1001, "3G HSDPA MODEM"))
        {
            response[0] = 1;
        }
    
        response[1] = -1;
        response[2] = -1;
        break;

    case 2: /* +CREG: <stat>, <lac>, <cid> */
        err = at_tok_nextint(&line, &response[0]);
        if (err < 0) goto error;
        err = at_tok_nexthexint(&line, &response[1]);
        if (err < 0) goto error;
        err = at_tok_nexthexint(&line, &response[2]);
        if (err < 0) goto error;
        break;
        
    case 3: /* +CREG: <n>, <stat>, <lac>, <cid> */
        err = at_tok_nextint(&line, &skip);
        if (err < 0) goto error;
        err = at_tok_nextint(&line, &response[0]);
        if (err < 0) goto error;
        err = at_tok_nexthexint(&line, &response[1]);
        if (err < 0) goto error;
        err = at_tok_nexthexint(&line, &response[2]);
        if (err < 0) goto error;
        break;
        
    /* special case for CGREG, there is a fourth parameter
     * that is the network type (unknown/gprs/edge/umts)
     */
    case 4: /* +CGREG: <n>, <stat>, <lac>, <cid>, <networkType> */
    case 5:
        err = at_tok_nextint(&line, &skip);
        if (err < 0) goto error;
        err = at_tok_nextint(&line, &response[0]);
        if (err < 0) goto error;
        err = at_tok_nexthexint(&line, &response[1]);
        if (err < 0) goto error;
        err = at_tok_nexthexint(&line, &response[2]);
        if (err < 0) goto error;
        err = at_tok_nexthexint(&line, &response[3]);
        if (err < 0) goto error;
        *count = 4;
        break;
    default:
        goto error;
    }

    at_response_free(p_response);
    return 0;
    
error:
    at_response_free(p_response);
    return -1;
}

static int queryRegistrationRat()
{
    int count = 0;
    int response[4]={0};
    int network_type = RADIO_TECHNOLOGY_UNKNOWN;

    if( !getRegistrationInfo(&response[0], &count)
        && count>3 )
    {
        network_type = response[3];
    }

    return network_type;
}

static void doSomeQuery()
{
#if 0
/*  cmy: ��ѯ��ǰ��PS/CS���񣬵��иı�ʱ���ϱ� RIL_UNSOL_RESTRICTED_STATE_CHANGED
    CS PS ״̬: ��PS/��CS/PS+CS/�޷���
 */
    int restricted_state_chg = 0;
    
    restricted_state_chg = queryRestrictedState();

    if(restricted_state_chg)
    {
        // ����״̬�����仯ʱ��Ҫ���ϲ�ȥ��ѯ������״̬
        RIL_onUnsolicitedResponse (
            RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED,
            NULL, 0);
    }
#endif

    // ���ڰ�����dongle���ڴ˲�ѯ�������ͣ��Ա㼰ʱ�����������͵ĸı�
    if(!modem_cmp(0x1BBB, 0x00B7, "HSPA Data Card")
        || !modem_cmp(0x12D1, 0x1001, "MU509")
        || !modem_cmp(0x19F5, 0x9013, "UW100")
        || !modem_cmp(0x1C9E, 0x9603, "HSPA USB MODEM")
        )
    {
        int rat = queryRegistrationRat();
        if( rat != s_reg_rat ) {
            LOGD("reg RAT: %d -> %d\n", s_reg_rat, rat);
            s_reg_rat = rat;
            RIL_onUnsolicitedResponse (
                RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED,
                NULL, 0);
        }
    }

}

/*
 GSM/WCDMA:
    0:  0,99
    1:  <9
    2:  <15
    3:  <21
    4:  >=21
    
  TD-SCDMA
    0:  0,199
    1:  <125
    2:  <143
    3:  <161
    4:  >=161

  CDMA/EVDO:
    0:  0
    1:  20
    2:  40
    3:  60
    4:  80
 */
static void requestSignalStrength(void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err;
	#if (RIL_VERSION >= 6)
    int response[13];
	#else
    int response[2];
	#endif
    char *line;
    int cur_network_type = 0;
	RIL_SignalStrength_v10 responseV10;

/* cmy: ����һЩ��ʱ��ѯ������
 */
    doSomeQuery();

// xxh ����modem��֧���źŲ�ѯ
    if( !modem_cmp(0x19D2, 0x0094, "AC580")
        || !modem_cmp(0x1D09, 0xAEF4, "E800")
        || !modem_cmp(0x12D1,0x140B,NULL)
        ||!modem_cmp(0x12D1, 0x140C, NULL))
       
    {
		response[0] = 15;
		response[1] = 99;
    }else if(!modem_cmp(0x19D2, 0xFFE8, "MC2718"))//xxh:����SPINRT�����ѯ�ź���AT+CSQ?

	{
		err = at_send_command_singleline("AT+CSQ?", "+CSQ:", &p_response);
		
		 if (err < 0 || p_response->success == 0) {
		 RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
		 goto error;
		 }
		
		 line = p_response->p_intermediates->line;
		
		 err = at_tok_start(&line);
		 if (err < 0) goto error;
		
	     err = at_tok_nextint(&line, &(response[0]));
		 if (err < 0) goto error;
		
		err = at_tok_nextint(&line, &(response[1]));
		if (err < 0) goto error;
		 response[1] = 99;
		
		 at_response_free(p_response);

		}
    else
    {
        if(!isgsm)
        {
    // ��ȡ��ǰ��������
            if(s_sys_mode==-1)
                cur_network_type = s_current_modem->extATFunc->getNetworkType();
            else
            {// s_sys_mode: 0, 2, 4, 8
                switch(s_sys_mode)
                {
                case 4: case 8: cur_network_type = RADIO_TECHNOLOGY_EVDO_A; break;
                default: cur_network_type = RADIO_TECHNOLOGY_1xRTT; break;
                }
            }
        }

        if( cur_network_type==RADIO_TECHNOLOGY_EVDO_A )
        {
            if(signalStrength_hdr[0] < 0 && signalStrength_hdr[0] < 0)
            {
                err = at_send_command_singleline("AT^HDRCSQ", "^HDRCSQ:", &p_response);
                if (err < 0 || p_response->success == 0) {
                    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
                    goto error;
                }

                line = p_response->p_intermediates->line;

                err = at_tok_start(&line);
                if (err < 0) goto error;

                err = at_tok_nextint(&line, &(response[0]));
                if (err < 0) goto error;

                switch(response[0])
                {
                case 0: break;
                case 20: response[0] = 4; break;
                case 40: response[0] = 12; break;
                case 60: response[0] = 18; break;
                case 80: response[0] = 26; break;
				case 99: response[0] = 34;break;
                default: response[0] = 99; break;
                }
                
                response[1] = 99;
                at_response_free(p_response);
        	} else {
        		LOGD("Sending stored CSQ values to RIL");
        		
        		response[0] = signalStrength_hdr[0];
        		response[1] = signalStrength_hdr[1];
        	}
        }
        else
        {
            if( signalStrength[0] < 0 && signalStrength[0] < 0 )
            {
             if(0 == modem_cmp(0x106C, 0x3718, NULL)){
                 response[0] = 15;
		         response[1] = 99;
             	}else{
                err = at_send_command_singleline("AT+CSQ", "+CSQ:", &p_response);

                if (err < 0 || p_response->success == 0) {
                    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
                    goto error;
                }

                line = p_response->p_intermediates->line;

                err = at_tok_start(&line);
                if (err < 0) goto error;

                err = at_tok_nextint(&line, &(response[0]));
                if (err < 0) goto error;

                err = at_tok_nextint(&line, &(response[1]));
                if (err < 0) goto error;
                response[1] = 99;

                at_response_free(p_response);
             		}
        	} else {
        		LOGD("Sending stored CSQ values to RIL");
        		
        		response[0] = signalStrength[0];
        		response[1] = signalStrength[1];
        	}
        }
    }

	#if (RIL_VERSION >= 6)
	//response[2]=-1;
	//response[3]=-1;
	//response[4]=-1;
	//response[5]=-1;
	//response[6]=-1;
	//response[7]=-1;
	//response[8]=-1;
	//response[9]=-1;
	//response[10]=-1;
	//response[11]=-1;
	//response[12]=-1;
	responseV10.GW_SignalStrength.signalStrength = response[0];
	responseV10.GW_SignalStrength.bitErrorRate = response[1];	
	//responseV10.CDMA_SignalStrength.dbm = -1;	
	//responseV10.CDMA_SignalStrength.ecio = -1;	
//	responseV10.EVDO_SignalStrength.dbm = -1;	
	//responseV10.EVDO_SignalStrength.ecio = -1;	
	//responseV10.EVDO_SignalStrength.signalNoiseRatio = -1;	
	responseV10.LTE_SignalStrength.signalStrength=99;	
	responseV10.LTE_SignalStrength.rsrp = 0x7FFFFFFF;	
	responseV10.LTE_SignalStrength.rsrq = 0x7FFFFFFF;	
	responseV10.LTE_SignalStrength.rssnr = 0x7FFFFFFF;	
	responseV10.LTE_SignalStrength.cqi = 0x7FFFFFFF;
	responseV10.TD_SCDMA_SignalStrength.rscp = 0x7FFFFFFF;
	#endif
	RIL_onRequestComplete(t, RIL_E_SUCCESS, &responseV10, sizeof(RIL_SignalStrength_v10));
	//RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(response));
    return;

error:
    LOGE("requestSignalStrength must never return an error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}
static void onUnsolicitedSignalStrength()
{

    ATResponse *p_response = NULL;
    int err;
    int response[2];
    char *line;

	RIL_SignalStrength_v10 responseV10;

    err = at_send_command_singleline("AT+CSQ", "+CSQ:", &p_response);

    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &(response[0]));
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &(response[1]));
    if (err < 0) goto error;
       
    LOGD("The requestSignalStrength =%d  profile=%d",response[0],response[1]);

	
	responseV10.GW_SignalStrength.signalStrength = response[0];
	responseV10.GW_SignalStrength.bitErrorRate = response[1];

	//responseV10.CDMA_SignalStrength.dbm = -1;
	//responseV10.CDMA_SignalStrength.ecio = -1;
	//responseV10.EVDO_SignalStrength.dbm = -1;
	//responseV10.EVDO_SignalStrength.ecio = -1;
	//responseV10.EVDO_SignalStrength.signalNoiseRatio = -1;
	responseV10.LTE_SignalStrength.signalStrength=99;
	responseV10.LTE_SignalStrength.rsrp = 0x7FFFFFFF;
	responseV10.LTE_SignalStrength.rsrq = 0x7FFFFFFF;
	responseV10.LTE_SignalStrength.rssnr = 0x7FFFFFFF;
	responseV10.LTE_SignalStrength.cqi = 0x7FFFFFFF;
	responseV10.TD_SCDMA_SignalStrength.rscp = 0x7FFFFFFF;
	
	RIL_onUnsolicitedResponse (RIL_UNSOL_SIGNAL_STRENGTH,&responseV10, sizeof(RIL_SignalStrength_v10));
    at_response_free(p_response);
    return;

error:
    LOGD("requestSignalStrength must never return an error when radio is on");
    at_response_free(p_response);
    return;
}

#define MODEM_POWER_INTERFACE       "/sys/class/rk291x_modem/modem_status"
static int setModemPower(int onoff)
{
    FILE*fp = fopen(MODEM_POWER_INTERFACE, "wr");

    LOGD("set modem power [%s]", onoff?"on":"off");
    
    if (fp != NULL)
    {
        if( fwrite(onoff?"1":"0", 1, 1, fp) <= 0 )
            LOGE("modem_status file write error!\n");

        fclose(fp);
    }
    else
    {
        LOGE("file %s open failed!", MODEM_POWER_INTERFACE);
    }
    
    return 0;
}

static void requestScreenState(void *data, size_t datalen, RIL_Token t)
{
	int err, screenState;

	assert (datalen >= sizeof(int *));
	screenState = ((int*)data)[0];

#ifdef DBG_1
    LOGD("[%s]: screenState=%d\n", __FUNCTION__, screenState);
#endif
	if(screenState == 1)
	{
		if (isgsm) {
			/* Screen is on - be sure to enable all unsolicited notifications again */
			err = at_send_command("AT+CREG=2", NULL);
			if (err < 0) goto error;
			err = at_send_command("AT+CGREG=2", NULL);
			if (err < 0) goto error;
		} else {
           // char value[64] = "";
           // property_get("net.gprs.ppp-exit", value, "");
           // if(!value[0])
          //  {
          //       LOGD("[%s] pppd: no response", __FUNCTION__);
          //  	 value[0]=1;
           // 	 property_set("net.gprs.ppp-exit", value);
                // RIL_requestTimedCallback (onDataCallListChanged, NULL, NULL);  
           // }
           // LOGD("[%s]  pppd reconncet", __FUNCTION__);
        }
	} else if (screenState == 0) {
		if (isgsm) {
			/* Screen is off - disable all unsolicited notifications */
			err = at_send_command("AT+CREG=0", NULL);
			if (err < 0) goto error;
			err = at_send_command("AT+CGREG=0", NULL);
			if (err < 0) goto error;
		} else {

		}
	} else {
		/* Not a defined value - error */
		goto error;
	}

	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
	return;

error:
	LOGE("ERROR: requestScreenState failed");
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

/* cmy: ��ȡ��ǰע�����������
   RIL_REQUEST_GPRS_REGISTRATION_STATE: ����ֵ�У�
        responseStr[0] GSM/GPRS��ע��״̬
        responseStr[3] �����������
   
   Available radio technologies for GSM, UMTS and CDMA.
    RADIO_TECHNOLOGY_UNKNOWN = 0;
    RADIO_TECHNOLOGY_GPRS = 1;
    RADIO_TECHNOLOGY_EDGE = 2;
    RADIO_TECHNOLOGY_UMTS = 3;
    RADIO_TECHNOLOGY_IS95A = 4;
    RADIO_TECHNOLOGY_IS95B = 5;
    RADIO_TECHNOLOGY_1xRTT = 6;
    RADIO_TECHNOLOGY_EVDO_0 = 7;
    RADIO_TECHNOLOGY_EVDO_A = 8;
    RADIO_TECHNOLOGY_HSDPA = 9;
    RADIO_TECHNOLOGY_HSUPA = 10;
    RADIO_TECHNOLOGY_HSPA = 11;

    ������������:
    1���ڳ�ʼ����ʼ������2G
    2���ϲ��CDMA��GSM�������磬

    ���Ը��������ϱ���^MODE��ֵ������������
    ����ʹ������AT^SYSINFO��ѯ��
 */
#define REG_STATE_LEN 15
#define REG_DATA_STATE_LEN 6
static void requestRegistrationState(int request, void *data,
                                        size_t datalen, RIL_Token t)
{
    int response[4]={0};
    //char* responseStr[4]={NULL};
    char **responseStr = NULL;
    int count = 0;
    int j;
    int numElements = 0;
    int network_type = RADIO_TECHNOLOGY_UNKNOWN;

    if (request == RIL_REQUEST_VOICE_REGISTRATION_STATE) {
	    numElements = REG_STATE_LEN;
    } else if (request == RIL_REQUEST_DATA_REGISTRATION_STATE) {
	    numElements = REG_DATA_STATE_LEN;
    } else {
    	assert(0);
	goto error;
    }
    responseStr = malloc(numElements * sizeof(char *));
    if (!responseStr) goto error;
	memset(responseStr, 0, numElements * sizeof(char *));

    getRegistrationInfo(&response[0], &count);

	//zte dongle
	if(s_current_modem->modem_info.vid==0x19d2){
		asprintf(&responseStr[0], "%d", response[0]);
		asprintf(&responseStr[1], "%04x", response[1]);
		asprintf(&responseStr[2], "%08x", response[2]);
	}else
	//huawei dongle
    asprintf(&responseStr[0], "%d", response[0]);
    asprintf(&responseStr[1], "%x", response[1]);
    asprintf(&responseStr[2], "%x", response[2]);

/*
   RIL_REQUEST_GPRS_REGISTRATION_STATE: ����ֵ�У�responseStr[3]�����������
   
    ������������:
    1���ڳ�ʼ����ʼ������2G
    2���ϲ��CDMA��GSM�������磬
 */
// cmy@20111228: ����ѯ����ע��״̬
// cmy@20120419: ��ͼ��ѯλ�����õ�����ע��״̬�ķ���ֵ
//                  ��E1750��ѯ����״̬û����
    if (!isgsm || !modem_cmp(0x1234, 0x0033, "LCR 3.0.0")) {
    // cmy: for CDMA
        if(!isgsm && request == RIL_REQUEST_GPRS_REGISTRATION_STATE)
        {
		    if(sim_pin){
			network_type=-1;
			 RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);		
					
			}else{
            network_type = s_current_modem->extATFunc->getNetworkType();
            if( network_type < 0)
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
                
            asprintf(&responseStr[3], "%d", network_type);
            count = 4;

            // cmy: ��ѯ��ǰ��������״̬
            queryRestrictedState();
			}
        }
        //RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, count*sizeof(char*));
	RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, numElements*sizeof(responseStr));
        return;
    }

    if (count > 3)
    {
        s_reg_rat = response[3];
        network_type = networkType2RAT(response[3]);
	if(!modem_cmp(0x05C6, 0x9000, NULL)) {
		network_type = 11;
	}
        asprintf(&responseStr[3], "%d", network_type);
    }

    /* cmy: ֻ�����Ѿ�ע������CREG/CGREGû�з���������뼼��������£�
        ����Ҫȥ��ȡ���뼼��
    */
    if( (response[0]==1 || response[0]==5) && network_type==RADIO_TECHNOLOGY_UNKNOWN )
    {
        network_type = s_current_modem->extATFunc->getNetworkType();
	if(!modem_cmp(0x05C6, 0x9000, NULL)) {
		network_type = 11;
	}
        if(network_type < 0)
            asprintf(&responseStr[3], "%d", RADIO_TECHNOLOGY_UNKNOWN);
        else
            asprintf(&responseStr[3], "%d", network_type);
        count = 4;

        // cmy: ��ѯ��ǰ��������״̬
        queryRestrictedState();
    }

/*
    cmy@20101206������BUG: ����һ�����Ӻ�s_reg_stat/s_greg_stat�Ѿ����ĳ���1
            ���������ӣ���ʼ״̬CREG/CGREG��״̬Ϊ2��֮��ͨ�������ϱ���ʽ��֪CREG/CGREG��״̬Ϊ1.
            ������s_reg_stat/s_greg_stat�Ѿ���1�����ᴥ���ϲ��������ѯ�������ϲ�����ΪCREG/CGREGΪ2.
 */

    if(request == RIL_REQUEST_REGISTRATION_STATE)
        s_reg_stat = response[0];
    else
        s_greg_stat = response[0];

     //RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, count*sizeof(char*));
     RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, numElements*sizeof(responseStr));
     for (j = 0; j < numElements; j++ ) {
        free(responseStr[j]);
        responseStr[j] = NULL;
     }
     free(responseStr);
     onUnsolicitedSignalStrength();
     return;
error:
    LOGE("requestRegistrationState must never return an error when radio is on");
    if (responseStr) {
        for (j = 0; j < numElements; j++) {
            free(responseStr[j]);
            responseStr[j] = NULL;
        }
        free(responseStr);
        responseStr = NULL;
    }
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

/* cmy: ��ȡ��ǰע���������Ϣ

    AT+COPS?
    +COPS: <mode>,<format>,<oper>,<act>
    
        <mode>:   ����ѡ��ģʽ 
        0:      �Զ����������������У�mode ȡֵΪ0 ʱ��������� format,oper����Ч��
        1:      �ֶ����� 
        2:      ȥע������ 
        3:      ��Ϊ+COPS?���������÷��صĸ�ʽ<format> 

        <format>:  ��Ӫ����Ϣ<oper>�ĸ�ʽ    
        0:  ���ַ�����ʽ����Ӫ����Ϣ<oper> 
        1:  ���ַ�����ʽ����Ӫ����Ϣ<oper> 
        2:  ���ָ�ʽ����Ӫ����Ϣ<oper> 

        <Act>�����߽��뼼����ȡֵ���£� 
        0: GSM/GPRS  ��ʽ 
        2:   UTRAN��ʽ 

    +COPS: 0,0,"�й��ƶ�",2
    +COPS: 0,1,"CMCC",2
    +COPS: 0,2,"46000",2
 */
static void requestOperator(void *data, size_t datalen, RIL_Token t)
{
    int err;
    int i;
    int skip;
    ATLine *p_cur;
    char *response[3];

    memset(response, 0, sizeof(response));

    ATResponse *p_response = NULL;

    OperatorInfo* info = NULL;
    
    /*
      cmy@20101023: ��mcc/mncֵ������operator_table��ʱ��ʹ�øñ��е�operator.
     */
    if( IMSI[0] == 0 )
        GetIMSI(IMSI);

	if( IMSI[0] != 0 )
	{
	    LOGD("IMSI is %s", IMSI);
	    char nmsi[10]={0};
	    strncpy(nmsi, IMSI, 5);
	    info = getOperator( nmsi );
	    LOGD("info: %p", info);
    }
   
    if(!modem_cmp(0x05C6, 0x9000, NULL)) {
    	response[0] = "China Unicom";
	response[1] = "China Unicom";
	response[2] = "46001";
    } else {
    if( info != NULL )
    {
		response[0]=info->lalphanumeric;
		response[1]=info->salphanumeric;
		response[2]=info->numeric;
		onUnsolicitedSignalStrength();
		LOGD("get operator form table: %s, %s, %s", response[0], response[1], response[2]);
    }
    else
    {
    	if(isgsm) {
/*
    Ŀǰ���ֲ���modem �� AT+COPS=3,x ָ��ֱ�ӷ���ʧ��
 */
            char cmd[64] = "";
            char *result = NULL;
            for(i=0; i<3; i++)
            {
                at_response_free(p_response);
                p_response = NULL;
                sprintf(cmd, "AT+COPS=3,%d", i);
                err = at_send_command(cmd, &p_response);
                if (err < 0|| p_response->success == 0) {
                    continue;
                }
                at_response_free(p_response);
                p_response = NULL;
                err = at_send_command_singleline("AT+COPS?", "+COPS:", &p_response);
                if (err < 0|| p_response->success == 0) {
                    continue;
                }

                char *line = p_response->p_intermediates->line;

                err = at_tok_start(&line);
                if (err < 0) goto error;

                err = at_tok_nextint(&line, &skip);
                if (err < 0) goto error;

                // If we're unregistered, we may just get
                // a "+COPS: 0" response
                if (!at_tok_hasmore(&line)) {
                    response[i] = NULL;
                    continue;
                }

                err = at_tok_nextint(&line, &skip);
                if (err < 0) goto error;

                // a "+COPS: 0, n" response is also possible
                if (!at_tok_hasmore(&line)) {
                    response[i] = NULL;
                    continue;
                }

                err = at_tok_nextstr(&line, &result);
                if (err < 0) goto error;

                asprintf(&response[i], "%s", result);
            }
            
            if( response[0] == NULL ) response[0] = response[1];
            if( response[1] == NULL ) response[1] = response[0];
            
            
            // cmy: ��֧������
            if(response[0]!=NULL )
            {
            	if( !strcmp(response[0], "�й��ƶ�") )
            	    response[0] = "China Mobile";
            	else if( !strcmp(response[0], "�й�����") )
            	    response[0] = "China Telecom";
            	else if( !strcmp(response[0], "�й���ͨ")
                        || !strcmp(response[0], "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF")
                        || !strcmp(response[0],"FFFFFFFFFFFFFFFF"))
            	    response[0] = "China Unicom";
            }
    	}
    }
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(response));
    at_response_free(p_response);

    return;
error:
    LOGE("requestOperator must not return error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

static int kill_pppd(int second)
{
	int i=0;
    int fd;

    if(!modem_cmp(0x1E89, 0x1E16, "E1916"))
    {
    // Ŀǰ���� E1916 ��Ҫ�� AT �ڷ��� 'ATH'ָ����������Ͽ�����
    // �������ATH����ô���´β���ʱ�ͻ�ʧ��
        at_send_command("ATH", NULL);
        sleep(2);
    }

    LOGD("kill pppd process!");
    while((fd = open("/sys/class/net/ppp0/iflink",O_RDONLY)) > 0) {
		if(i%5 == 0) {
			system("busybox1.11 killall pppd");
		}
		close(fd);
		if(i>second)
			return -1;
		i++;
		sleep(1);
	}
//end 
   // if(0 == modem_cmp(0x106C, 0x3718, NULL))
     //   at_emulate_exit();
    //end by xxh
    return 0;
}

 static int get_pid(char *Name)
 {
 char name[14] = {0};
 strncpy(name,Name,14);
 char szBuf[256] = {0};
 char cmd[20] = {0};
 char *p_pid = NULL;
 FILE *pFile = NULL;
 int  pid = 0;
 sprintf(cmd, "ps | grep %s", name);
 pFile = popen(cmd, "r");
if (pFile != NULL)  {
     while (fgets(szBuf, sizeof(szBuf), pFile))          {
         if(strstr(szBuf, name))         {
            p_pid = strstr(szBuf, " ");
           pid = strtoul(p_pid, NULL, 10);
            RLOGD("--- %s pid = %d ---",name,pid);
            break;
           }
         }
    }
 pclose(pFile);
 return pid;
 }
 

static void kill_rild()
{
     int rild_pid = get_pid("rild");
     int count = 5;
     while((rild_pid = get_pid("rild")) && count--){
         kill(rild_pid, SIGKILL);
         RLOGD("--- kill rild pid = %d ---",rild_pid);
         sleep(1);
     }
}

/*
 cmy: PPP�����ϵ�����
        1 ip-up�ű���ִ�� "net.ppp0.local-ip" ��Ϊ��
        2 "/sys/class/net/ppp0/operstate"�ļ�������Ϊ"unknown" ���� "up"

 ����ֵ: -1 ��ʱ�˳�
          0 ���ӳɹ�
 */
static int getpppstatus(char **ip_address)
{
    int timeout = 30;// 30S
    char line[64];
    //ip_address[0] = 0;
   // char ppp_exit_status[64];
	char ppp_exit_status[PROPERTY_VALUE_MAX];
    static char ip_local_address[PROPERTY_VALUE_MAX]; 
    while(timeout--){
        sleep(1);

        /* ȷ��pppd����û���˳�
               ��ִ��pppdʱ��net.gprs.ppp-exit ��Ϊ��
               ���˳�pppd��net.gprs.ppp-exit ��Ϊ��
         */
        property_get("net.gprs.ppp-exit", ppp_exit_status, "");
        if(ppp_exit_status[0] != '\0') break;

#if 0 // cmy: Ŀǰ���ָù��ܻ�����û�����ϣ���������ģ������������ʱ
        // ȷ��ģ����������
        int err = at_send_command("AT", NULL);
        if(err) break;
#endif
        FILE*fp = fopen("/sys/class/net/ppp0/operstate", "r");
        // �ļ������ڣ�˵����û��PPPЭ�̲��֣����ܴ�ʱ�ڲ���
        if(fp == NULL) continue;

        line[0] = 0;
        fgets(line, 63, fp);
        fclose(fp);

        RLOGD("[%s]: read=%s", __FUNCTION__, line);

        /*  operstate ������ֵ:
                down:       δ�����ϣ����ڽ���Э��
                up:         ������
                unknown:    ������
         */
        if( !strncmp(line, "up", strlen("up")) || !strncmp(line, "unknown", strlen("unknown")) )
        {
            property_get(RIL_LOCAL_IP, ip_local_address, "");
           
            RLOGD("[%s]: local-ip=%s", __FUNCTION__, ip_local_address);
            *ip_address=ip_local_address;
            
            // ip_address��Ϊ��
            if( ip_local_address[0] ) return 0;
        }
    }

    // ����ʧ�ܻ��߳�ʱ��ɱ��pppd����
    kill_pppd(25);

    return -1;
}
/**
* called by requestSetupDataCall add by xxh
*  
* return 0 for success
* return 1 for error;
* return 2 for ppp_error;
*/
static int setupDataCallEth(char**response_local_ip,const char* apn)
{
#define SUCCESS 0
#define ERROR 1
#define PPP_ERROR 2
    int fd;
    char buffer[20];
    char exit_code[PROPERTY_VALUE_MAX];
    char result_code[PROPERTY_VALUE_MAX];
    static char local_ip[PROPERTY_VALUE_MAX]; 
    static char local_pdns[PROPERTY_VALUE_MAX]={0};
    static char local_sdns[PROPERTY_VALUE_MAX]={0};
    static char local_gateway[PROPERTY_VALUE_MAX]={0};
    isEth=1;
    int retry = POLL_PPP_SYSFS_RETRY;
    int check_result_time = 0;
    int ready_for_connect = 0;
    int try_connect_numbers = 0;
    int err = 0;
    char *cmd;
    char ipaddr,gateway;
    uint32_t prefixLength;
    char* dns;
    char server;
    uint32_t lease;
    char domain;
    char mtu;
    char vendorInfo;

    LOGD("******** Enter setupDataCallEth ********");
    
   
    err = at_send_command("ATH", NULL);
  
    
    LOGD("requesting data connection to APN '%s'", apn);
    asprintf(&cmd, "AT+CGDCONT=1,\"IP\",\"%s\",,0,0", apn);
    err = at_send_command(cmd, NULL);
    free(cmd);
	if(!modem_cmp(0x05c6,0x9025,NULL)){
		
        err =at_send_command("AT^NETACT=1,0",NULL);

		if(err<0) LOGD("can not act =======");
	}

	//int lte_err=0;

	//lte_err=property_set("ctl.start", SERVICE_LTE);

	//if(lte_err<0){

	// LOGD("Can not start lteup");
    
	//}
    //pppd = 1;
    if(!modem_cmp(0x19D2, 0x1244, NULL)||!modem_cmp(0x05c6,0x9025,NULL)){
		property_set("dhcp.lte0.result", "");
		   property_set("dhcp.lte0.ipaddress", "");

	}else{
    property_set("dhcp.wwan0.result", "");
    property_set("dhcp.wwan0.ipaddress", "");
		}
    // Waitting for dhcpcd sucessfully
    do
    {
	    // Waitting for dhcpcd sucessfully
//		sleep(3);
		LOGW("******** Start dhcpcd********");
               
                //dhcp_start("lte0");
 
		if(!modem_cmp(0x19D2, 0x1244, NULL)||!modem_cmp(0x05c6,0x9025,NULL)){

			//dhcp_get_results("lte0", &ipaddr, &gateway, &prefixLength, &dns,  &server, &lease,&vendorInfo,&domain,&mtu);
		}else{		
		        //dhcp_get_results("wwan0", &ipaddr, &gateway,&prefixLength, &dns, &server, &lease, &vendorInfo,&domain,&mtu);		
		}
        // clear this counter
        check_result_time = 0;
		
		//readinterfaceprop("/sys/class/net/eth0");

        // this index start from 1
        LOGW("Check the lte0 stats for the %dth time", (POLL_PPP_SYSFS_RETRY - retry + 1));

        fd  = open(ETH_OPERSTATE_PATH, O_RDONLY);
        if (fd >= 0)
        {
            LOGD("open eth_operstate_path ok");
            buffer[0] = 0;
            read(fd, buffer, sizeof(buffer));
            close(fd);

            if(!strncmp(buffer, "up", strlen("up")) || !strncmp(buffer, "unknown", strlen("unknown")))
            {

                //waiting for dhcpcd completely
                do
                {
                    // Delay for max 20 seconds, the break to retry
                    if(check_result_time > 10)
                        break;

                    sleep(1);
                    check_result_time++;
		    if(!modem_cmp(0x19D2, 0x1244, NULL)||!modem_cmp(0x05c6,0x9025,NULL))
		    
			    err = property_get("dhcp.lte0.result", result_code, "");
		    else
			    err = property_get("dhcp.wwan0.result", result_code, "");
                }
                while (strcmp(result_code, "ok"));

                LOGW("Dhcpcd waste %d senconds in the %dth time", check_result_time, (POLL_PPP_SYSFS_RETRY - retry + 1));
                if(check_result_time > 21)
                    continue;

                // Should already get local IP address from PPP link after IPCP negotation
                // system property net.ppp0.local-ip is created by PPPD in "ip-up" script
                local_ip[0] = 0;
				if(!modem_cmp(0x19D2, 0x1244, NULL)||!modem_cmp(0x05c6,0x9025,NULL)){
					property_get("dhcp.lte0.ipaddress", local_ip, "");
					property_get("dhcp.lte0.dns1", local_pdns, "");
					property_get("dhcp.lte0.dns2", local_sdns, "");
					property_get("dhcp.lte0.gateway", local_gateway, "");
					
					LOGD("local_ip:%s",local_ip);
					LOGD("local_pdns:%s",local_pdns);
					LOGD("local_sdns:%s",local_sdns);
					property_set("net.dns1", local_pdns);
					property_set("net.dns2", local_sdns);
					property_set("net.lte0.dns1", local_pdns);
					property_set("net.lte0.dns2", local_sdns);
					property_set("net.lte0.gw", local_gateway);


				}

				else
				
				{

                                     property_get("dhcp.wwan0.ipaddress", local_ip, "");                
				     property_get("dhcp.wwan0.dns1", local_pdns, "");              
				     property_get("dhcp.wwan0.dns2", local_sdns, "");				
				     property_get("dhcp.wwan0.gateway", local_gateway, "");               
				     LOGD("local_ip:%s",local_ip);				
				     LOGD("local_pdns:%s",local_pdns);				
				     LOGD("local_sdns:%s",local_sdns);				
				     property_set("net.dns1", local_pdns);
				     property_set("net.dns2", local_sdns);				
				     property_set("net.wwan0.dns1", local_pdns);				
				     property_set("net.wwan0.dns2", local_sdns);				
				     property_set("net.wwan0.gw", local_gateway);
						
				}
                if((!strcmp(local_ip, "")) || (!strcmp(local_ip, "0.0.0.0")) || (!strcmp(local_pdns, "")) || (!strcmp(local_sdns, "")))
                {
                    LOGW("eth0 link is up but no local IP is assigned. Will retry %d times after %d seconds", \
                         retry, POLL_PPP_SYSFS_SECONDS);
                }
                else
                {
                    LOGD("eth0 link is up with local IP address %s", local_ip);
                    // other info like dns will be passed to framework via property (set in ip-up script)
                    //response[2] = local_ip;
                         *response_local_ip = local_ip;
                    // now we think et1 is ready
                    break;
                }
            }
            else
            {
                LOGW("eth0 link status in %s is %s. Will retry %d times after %d seconds", \
                     ETH_OPERSTATE_PATH, buffer, retry, POLL_PPP_SYSFS_SECONDS);
            }
        }
        else
        {
            LOGW("Can not detect eth state in %s. Will retry %d times after %d seconds", \
                 ETH_OPERSTATE_PATH, retry-1, POLL_PPP_SYSFS_SECONDS);
        }
        sleep(POLL_PPP_SYSFS_SECONDS);
		
		at_send_command("ATH", NULL);

         if(!modem_cmp(0x19D2, 0x1244, NULL)||!modem_cmp(0x05c6,0x9025,NULL)){
		//dhcp_stop("lte0");
	 }else{
		//dhcp_stop("wwan0");
	 }
	}	
    while (--retry);

    if(retry == 0)
    {
        goto ppp_error;
    }

    return SUCCESS;
error:
    return ERROR;
ppp_error:
    return PPP_ERROR;
}

/*
static void requestSetupDataCall(void *data, size_t datalen, RIL_Token t)
{
    at_send_command("ATD18259010520;", NULL);
}
*/

/*
     cmy: �ڲ�������ʱ��������ϲ��´����������ͷ��� AT^SYSCONFIG ���������Ӧ���ã�����:
            ֻ����2G/ֻ����3G/2G����/3G����
 */
static void requestSetupDataCall(void *data, size_t datalen, RIL_Token t)
{
	char *cmd = NULL;
	int err = 0;
	//char ip_address[32];
	char ip_address[PROPERTY_VALUE_MAX]; 
	//char ip_gw[32];
	 char ip_gw[PROPERTY_VALUE_MAX];
	char *response[3] = { "1", PPP_TTY_PATH, ip_address };
//	char *response[2] = { "1", PPP_TTY_PATH };
	int mypppstatus;
	char strTemp[32];
	ATResponse *p_response = NULL;
    //	char dialup[64] = {0};
	char* dialup = NULL;

    const char* radioTechnology = ((const char**)data)[0];// GSM or CDMA
    const char* profile = ((const char**)data)[1];
    const char* apn = ((const char**)data)[2];
    const char* user = ((const char**)data)[3];
    const char* password = ((const char**)data)[4];
    const char* authType = ((const char**)data)[5];
    char* option ;
	int setup_data_call_result = 0;
	char*response_local_ip[32];
	//canel by xxh
	#if 0
    if(!modem_cmp(0x1004, 0x61AA, NULL))
    {
        vl600_attach();
        /*
            set ip_address
            set net.[ip_address].gw
            set net.[ip_address].dns1
            set net.[ip_address].dns2
        */
    	RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(response));
    	at_response_free(p_response);
    	return;
    }
 //end by xxh
#endif
    if(0 == modem_cmp(0x04CC, 0x225A, "TD369"))
    {
        dialup= "ATDT*99***1#";
    }
    else if (!modem_cmp(0x106C, 0x3718, "UML290VW"))
    {
        dialup = "ATD*99***3#";
    }
    else
    {
        switch(s_current_modem->radioType)
        {
    	case RADIO_TYPE_CDMA: case RADIO_TYPE_EVDO:
    		dialup = "ATDT#777";
    		break;
    	case RADIO_TYPE_TDSCDMA:
    		dialup = "ATDT*98*1#";
    		break;
    	default:
    		dialup = "ATDT*99#";
    		break;
        }
    }

	if(isgsm) {
	// cmy: CGDCONT/CGACT���������Ӧ��ʧ��
	    if (modem_cmp(0x106C, 0x3718, "UML290VW") == 1)
	    {
            if (!modem_cmp(0x19D2, 0x0167, "MF820")){
                asprintf(&cmd, "AT+CGDCONT=1,\"IP\",\"%s\",\"0.0.0.0\",0,0", apn);
                  property_set("net.ppp0.local-ip", "");
            }
            else
                asprintf(&cmd, "AT+CGDCONT=1,\"IP\",\"%s\",,0,0", apn);
                 property_set("net.ppp0.local-ip", "");
    		err = at_send_command(cmd, &p_response);
    		free(cmd);
    		//FIXME check for error here
    	//	if(err || p_response->success == 0) goto error;
	    }
		if(0 == modem_cmp(0x05C6, 0x6000, "HSDPA Modem"))
		{
    		err = at_send_command("AT+CGATT=1", NULL);
		}
		//for LC6341
		if(0 == modem_cmp(0x1AB7, 0x6341, "LC6341"))
		{
    		// Set required QoS params to default
    		err = at_send_command("AT+CGEQREQ=1,2,128,2048,0,0,0,0,\"0E0\",\"0E0\",,0,0", NULL);
		}
		if(err) goto error;
                  if(!modem_cmp(0x1782, 0x0002, "V1.0.1-B7")){
                  //err=at_send_command("AT+SIPMODE=0",NULL);
                  //err=at_send_command("AT+SPACTCFUN=1,0",NULL);
                 // err=at_send_command("AT+SAUTOATT=1",NULL);
                 // err=at_send_command("AT+SATT=1,0",NULL);
                 }
                   else{
		// Set required QoS params to default
		//err = at_send_command("AT+CGQREQ=1", NULL);
		// Set minimum QoS params to default
	//	err = at_send_command("AT+CGQMIN=1", NULL);
               }
		// packet-domain event reporting
//		err = at_send_command("AT+CGEREP=1,0", NULL);
		// Hangup anything that's happening there now
		//WM66E����PDP����ʧ�ܺ󣬲�ȥ���ţ����˵�������
		if (modem_cmp(0x19D2, 0x0167, "MF820")
            && modem_cmp(0x106C, 0x3718, "UML290VW")
            && modem_cmp(0x1C9E, 0x9915, "HSDPA mobile station")   //smart bro WM66E
            && modem_cmp(0x1C9E, 0x9914, "HSDPA mobile station")) //smart bro WM66E
		{
            err = at_send_command("AT+CGACT=0,1", &p_response);
           // if(err || p_response->success == 0) goto error;
		}
		// Start data on PDP context 1
	} else if(s_first_dialup){
    	s_first_dialup =0; 
		if(!modem_cmp(0x15EB, 0x0001, "CBP7.1T")){
	    LOGD("send ATH command to modem");
	    err = at_send_command("ATH", NULL);
		sleep(2);
		}
	    LOGD("for cdma setup data, should sleep 8s.");
             property_set("net.ppp0.local-ip", "");
	    sleep(8);
	}

    if (!modem_cmp(0x05C6, 0x6000, "SEV859"))
    {
        asprintf(&cmd, "AT^PPPCFG=\"%s\",\"%s\"", user, password);
        err = at_send_command(cmd, &p_response);
        free(cmd);
    }
	if(!modem_cmp(0x12D1, 0x1404, "MC509-a")){

      asprintf(&cmd, "AT^PPPCFG=\"%s\",\"%s\"", user, password);
      err = at_send_command(cmd, &p_response);
      free(cmd);

	}
	if(!modem_cmp(0x15EB, 0x0001, "CBP7.1T")){
	err = at_send_command("ATH", NULL);
	LOGD("send ATH command to modem second time");
	}

	// The modem replies immediately even if it's not connected!
	// so wait a short time.
	sleep(1);
//	LOGD("[%s]: dial ok.", __FUNCTION__);

    char* conn_script;
    char* disconn_script;

/*
    cmy: ����Dongle���ڲ���ʱ���յ�'NO CARRIER'������ppp�˳�
 */
//    asprintf(&conn_script, "/system/bin/chat -v -s -S TIMEOUT 25 ABORT 'BUSY' ABORT 'NO CARRIER' ABORT 'ERROR' ABORT '+CME ERROR:' %s %s CONNECT", s_current_modem->connect_init_script, dialup);
   
	if(0 == modem_cmp(0x04CC, 0x225A, "TD369"))
	{
	   option="novj";
    	asprintf(&conn_script, "/system/bin/chat -v -s -S TIMEOUT 25 ABORT 'BUSY' ABORT 'NO ANSWER' %s %s CONNECT", s_current_modem->connect_init_script, dialup);
        asprintf(&disconn_script, "/system/bin/chat -v -s -S ABORT 'BUSY' ABORT 'ERROR' ABORT '+CME ERROR:' %s", s_current_modem->disconnect_script);
    	user="cmnet";
    	password="cmnet";
    	//asprintf(&cmd, "/etc/ppp/call-pppd \"%s %d\" \"%s\" \"%s\" \"%s\" \"%s\" &",s_current_modem->pppchannel_index, s_current_modem->baudrate, user, password, conn_script, disconn_script); 
    	 asprintf(&cmd, "/etc/ppp/call-pppd \"%s %d\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" &",s_current_modem->pppchannel_index, s_current_modem->baudrate, option,user, password, conn_script, disconn_script); 
	}else if(0 == modem_cmp(0x19D2, 0xFFF1, "MC271X"))
	{
	    option="novj";
		asprintf(&conn_script, "/system/bin/chat -v -s -S TIMEOUT 25 ABORT 'BUSY' ABORT 'NO ANSWER' %s %s CONNECT", s_current_modem->connect_init_script, dialup);
		asprintf(&disconn_script, "/system/bin/chat -v -s -S ABORT 'BUSY' ABORT 'ERROR' ABORT '+CME ERROR:' %s", s_current_modem->disconnect_script);
		//asprintf(&cmd, "/etc/ppp/call-pppd \"%s %d\" \"%s\" \"%s\" \"%s\" \"%s\" &",s_current_modem->pppchannel_index, s_current_modem->baudrate, user, password, conn_script, disconn_script); 
		asprintf(&cmd, "/etc/ppp/call-pppd \"%s %d\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" &",s_current_modem->pppchannel_index, s_current_modem->baudrate, option,user, password, conn_script, disconn_script); 
	}
	else if(0 == modem_cmp(0x12D1, 0x1506, "E303C") ||0== modem_cmp(0x12D1, 0x1506, "E303") || 0==modem_cmp(0x12D1, 0x1506, "E369")||!modem_cmp(0x12d1, 0x1506,"E3131")||!modem_cmp(0x12D1, 0x1506, "E173")||!modem_cmp(0x12D1, 0x140C, "E173")||!modem_cmp(0x12d1,0x1506,"E3256"))
	{
		 option="novjccomp";
		 asprintf(&conn_script, "/system/bin/chat -v -s -S TIMEOUT 25 ABORT 'BUSY' ABORT 'ERROR' ABORT '+CME ERROR:' %s %s CONNECT", s_current_modem->connect_init_script, dialup);
         asprintf(&disconn_script, "/system/bin/chat -v -s -S ABORT 'BUSY' ABORT 'ERROR' ABORT '+CME ERROR:' %s", s_current_modem->disconnect_script);
		 asprintf(&cmd, "/etc/ppp/call-pppd \"%s %d\"  \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" &",s_current_modem->pppchannel_index, s_current_modem->baudrate,option,user, password, conn_script, disconn_script); 
	}
    else
	{
	    option="novj";
        asprintf(&conn_script, "/system/bin/chat -v -s -S TIMEOUT 25 ABORT 'BUSY' ABORT 'ERROR' ABORT '+CME ERROR:' %s %s CONNECT", s_current_modem->connect_init_script, dialup);
        asprintf(&disconn_script, "/system/bin/chat -v -s -S ABORT 'BUSY' ABORT 'ERROR' ABORT '+CME ERROR:' %s", s_current_modem->disconnect_script);
        asprintf(&cmd, "/etc/ppp/call-pppd \"%s %d\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" &",s_current_modem->pppchannel_index, s_current_modem->baudrate, option,user, password, conn_script, disconn_script); 
	}

#ifdef ENABLE_STAY_AWAKE
 if(s_current_modem->radioType==RADIO_TYPE_CDMA || s_current_modem->radioType==RADIO_TYPE_EVDO||!modem_cmp(0x12D1, 0x1D09, "ET127/158/128-2")||!modem_cmp(0x12D1, 0x1D50, "ET302/306"))
    {
        LOGD("[%s]: acquire_wake_lock: %s\n", __FUNCTION__, WAKE_LOCK_ID);
        acquire_wake_lock(PARTIAL_WAKE_LOCK, WAKE_LOCK_ID);
    }
#endif
#if 0
    if(0 == modem_cmp(0x106C, 0x3718, NULL))
        at_emulate_enter();
#endif
	if(!modem_cmp(0x216F, 0x0047, "ALT3100") 
	    || !modem_cmp(0x19D2, 0x1244, "K4201-Z")
	    || !modem_cmp(0x05c6,0x9025, NULL)
	    || !modem_cmp(0x12d1,0x155e,NULL)
	    || !modem_cmp(0x12d1,0x155f,NULL)){
		

	  setup_data_call_result = setupDataCallEth(response_local_ip,apn);
	  if(setup_data_call_result == 1)
	     {
	          goto error;
	     }
	  response[1]=PPP_ETH_PATH;
	  response[2] = *response_local_ip;
	  //ip_address
	  char ip_gw[PROPERTY_VALUE_MAX]={0};
	  property_get("dhcp.wwan0.gateway", ip_gw, "");
	  LOGD("[%s]: local-gw=%s", __FUNCTION__, ip_gw);
	  LOGD("******** lte0 local ip %s",response[2]);

	}
	else
	{
	LOGD("[%s]: execute cmd:\n%s", __FUNCTION__, cmd);
	mypppstatus = system(cmd);
	free(cmd);
	free(conn_script);
	free(disconn_script);
	LOGD("[%s] mypppstatus=%d", __FUNCTION__, mypppstatus);
    char*response_local_ip[32];
	if ( (mypppstatus < 0) || getpppstatus(response_local_ip) ) //ip_address
    {
#ifdef ENABLE_STAY_AWAKE
        if(s_current_modem->radioType==RADIO_TYPE_CDMA || s_current_modem->radioType==RADIO_TYPE_EVDO||!modem_cmp(0x12D1, 0x1D09, "ET127/158/128-2")||!modem_cmp(0x12D1, 0x1D50, "ET302/306"))
        {
            LOGD("[%s]: release_wake_lock: %s\n", __FUNCTION__, WAKE_LOCK_ID);
            release_wake_lock(WAKE_LOCK_ID);
        }
#endif
        goto error;
    }
	//response[2]=ip_address;
	response[2] = *response_local_ip;
	property_get("net.ppp0.gw", ip_gw, "");
	LOGD("[%s]: local-gw=%s", __FUNCTION__, ip_gw);

    // �½����̣����ڼ���pppd������״��
#if 1
    LOGD("create thread for pppd state!");
    if( pthread_create(&s_tid_pppdState, NULL, pppdStateLoop, NULL) )
    {
        LOGD("thread create failed!");
    }
#endif
}
	char local_pdns[PROPERTY_VALUE_MAX]={0};
	char local_sdns[PROPERTY_VALUE_MAX]={0};
	char dns[PROPERTY_VALUE_MAX*2]={0};
	property_get("net.ppp0.dns1", local_pdns, "");
	property_get("net.ppp0.dns2", local_sdns, "");
	sprintf(dns,"%s %s",local_pdns,local_sdns);

#if (RIL_VERSION >= 6)
	{
    RIL_Data_Call_Response_v11 response_v11;
    response_v11.status = 0;
    response_v11.suggestedRetryTime = -1;
    response_v11.cid = 1;//response[0];
    response_v11.active = 2;
    response_v11.type = "IP";
    response_v11.addresses = response[2];
    response_v11.dnses = dns;//"8.8.8.8 8.8.8.8";//NULL;
    response_v11.gateways = ip_gw;
    response_v11.ifname = response[1];
    response_v11.pcscf = "";
    response_v11.mtu = 1500;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response_v11, sizeof(RIL_Data_Call_Response_v11));
	}
#else    
	RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(response));
#endif

	at_response_free(p_response);
	return;

error:
    LOGD("[%s]: failed", __FUNCTION__);
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	at_response_free(p_response);
    #if 0
    if(0 == modem_cmp(0x106C, 0x3718, NULL))
        at_emulate_exit();
    #endif
}

static void requestDeactivateDefaultPDP(void *data, size_t datalen, RIL_Token t)
{
	int err;
	char * cmd;
	char *netcfg;
	char * cid;
	int fd,i,fd2;
	ATResponse *p_response = NULL;

    pthread_cond_signal(&s_waitcond);
    
#if 1
// ��ʱ�Ƿ��б�Ҫ����AT����?
	cid = ((char **)data)[0];
if(!modem_cmp(0x19d2,0x1244,NULL)){

//err=dhcp_stop("lte0");
if(err<0)
{
LOGW("Can not down the lte0 link");
 goto error;
}
system("netcfg lte0 down");
RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
return;
}else{
	if (isgsm) {
	// AT+CGACT=0,1 
		asprintf(&cmd, "AT+CGACT=0,%s", cid);

		err = at_send_command(cmd, &p_response);
		free(cmd);
		at_response_free(p_response);
		}
	else
	{
//        err = at_send_command("ATH", NULL);
//        sleep(2);
	}
#else
    if(!isgsm)
	{
	// cmy: ����modemʹ��ATHָ���close ppp
        err = at_send_command("ATH", NULL);
        sleep(2);
	}
#endif

#ifdef ENABLE_STAY_AWAKE
    if(s_current_modem->radioType==RADIO_TYPE_CDMA || s_current_modem->radioType==RADIO_TYPE_EVDO||!modem_cmp(0x12D1, 0x1D09, "ET127/158/128-2")||!modem_cmp(0x12D1, 0x1D50, "ET302/306"))
    {
        LOGD("[%s]: release_wake_lock: %s\n", __FUNCTION__, WAKE_LOCK_ID);
        release_wake_lock(WAKE_LOCK_ID);
    }
#endif

    if( kill_pppd(25)<0 ) goto error;

	sleep(2);

	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
	return;
		}
error:
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void  requestEnterSimPin(int request,void*  data, size_t  datalen, RIL_Token  t)
{
	ATResponse   *p_response = NULL;
	int           err;
	char*         cmd = NULL;
	const char**  strings = (const char**)data;

   int count = datalen/sizeof(char*);
   if(strings[count-1] == NULL)
        --count;

	if(1/*isgsm*/) {
		if ( count == 1 ){ 	
			asprintf(&cmd, "AT+CPIN=\"%s\"", strings[0]);
		} 
		else if ( count == 2 ) {
// ʵ�ʲ��Է���Ҫ��CPIN�������⿪PUK��
                if(!modem_cmp(0x0e8d,0x00a2,"MTK2")){ //usi modem
                LOGE("USI request enter sim pin ====================");
                asprintf(&cmd, "AT+CPIN=\"%s\"", strings[0]);
                }
               else{
			   asprintf(&cmd, "AT+CPIN=\"%s\",\"%s\"", strings[0], strings[1]);
       //     asprintf(&cmd, "AT+CPWD=\"SC\",\"%s\",\"%s\"", strings[0], strings[1]);
              if(request == RIL_REQUEST_ENTER_SIM_PUK || request == RIL_REQUEST_ENTER_SIM_PUK2) { 
		             asprintf(&cmd, "AT+CPIN=\"%s\",\"%s\"", strings[0], strings[1]);
		         }else if(request == RIL_REQUEST_CHANGE_SIM_PIN || request == RIL_REQUEST_CHANGE_SIM_PIN2){
		            asprintf(&cmd, "AT+CPWD=\"SC\",\"%s\",\"%s\"", strings[0], strings[1]);
             }
		  }
		} 
		else

	    goto error;

// cmy: "AT+CPIN=..." ������� "+CPIN: ..."
//		err = at_send_command_singleline(cmd, "+CPIN:", &p_response);
		err = at_send_command(cmd, &p_response);
		free(cmd);

		if (err < 0 || p_response->success == 0) {
error:
			RIL_onRequestComplete(t, RIL_E_PASSWORD_INCORRECT, NULL, 0);
		} else {
			if ( count == 1 )//PIN����֤�ɹ�����ȴ�һ��ʱ��
			{
            // ȷ�Ͽ��շ�ATָ��
                LOGE("cpin correct====================");
                sleep(3);
				sim_pin = 0;
                if(check_at_ready("AT+CIMI", 80))
                {
                    LOGE("AT handshake failed!!!");
                    RIL_onRequestComplete(t, RIL_E_PASSWORD_INCORRECT, NULL, 0);
                    return;
                }

//				sleep(3);
			}
      if(!modem_cmp(0x0e8d,0x00a2,"MTK2")){
      	
      	 //  RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
	        // RIL_requestTimedCallback(atinit, NULL, &TIMEVAL_0);
	         LOGE("USI cpin correct====================");
           sleep(5);   //����pin�����Ҫ��modemһ��ʱ���ʼ��sim����Ϣ
           
      	} 
			RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
			/* Notify that SIM is ready */
			//setRadioState(RADIO_STATE_SIM_READY);
		}
		at_response_free(p_response);
	} else {
		RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
		//setRadioState(RADIO_STATE_SIM_READY);
	}
}

static void unsolicitedNitzTime(const char * s)
{
	int err;
	char * response = NULL;
	char * line = NULL;
	char * p = NULL;
	char * tz = NULL; /* Timezone */
	line = strdup(s);

	/* Higher layers expect a NITZ string in this format:
	 *  08/10/28,19:08:37-20,1 (yy/mm/dd,hh:mm:ss(+/-)tz,dst)
	 */

	if(strStartsWith(s,"+CTZV:")){

		/* Get Time and Timezone data and store in static variable.
		 * Wait until DST is received to send response to upper layers
		 */
		at_tok_start(&line);

		err = at_tok_nextstr(&line, &tz);
		if (err < 0) goto error;

		err = at_tok_nextstr(&line, &response);
		if (err < 0) goto error;

		strcat(response,tz);

		sNITZtime = response;
		return;

	}
	else if(strStartsWith(s,"+CTZDST:")){

		/* We got DST, now assemble the response and send to upper layers */
		at_tok_start(&line);

		err = at_tok_nextstr(&line, &tz);
		if (err < 0) goto error;

		asprintf(&response, "%s,%s", sNITZtime, tz);

		RIL_onUnsolicitedResponse(RIL_UNSOL_NITZ_TIME_RECEIVED, response, strlen(response));
		free(response);
		return;

	}
	else if(strStartsWith(s, "+HTCCTZV:")){
		at_tok_start(&line);

		err = at_tok_nextstr(&line, &response);
		if (err < 0) goto error;
		RIL_onUnsolicitedResponse(RIL_UNSOL_NITZ_TIME_RECEIVED, response, strlen(response));
		return;

	}

error:
	LOGE("Invalid NITZ line %s\n", s);
}

static void unsolicitedRSSI(const char * s)
{
	int err;
	int response[2];
	char * line = NULL;

	line = strdup(s);

	err = at_tok_start(&line);
	if (err < 0) goto error;

	err = at_tok_nextint(&line, &(response[0]));
	if (err < 0) goto error;

//	err = at_tok_nextint(&line, &(response[1]));
//	if (err < 0) goto error;
// cmy: ΪʲôҪ����2
//	response[0]*=2;
	response[1]=99;

	signalStrength[0]=response[0];
	signalStrength[1]=response[1];
//	free(line);

// cmy: ����RIL_UNSOL_SIGNAL_STRENGTH�ᵼ���ϲ㲻�ٶ�ʱ��ѯ�ź�ǿ��
//	RIL_onUnsolicitedResponse(RIL_UNSOL_SIGNAL_STRENGTH, response, sizeof(response));
	return;

error:
	/* The notification was for a battery event - do not send a msg to upper layers */
	return;
}

static void unsolicitedRSSILVL(const char * s)
{
	int err;
	int response;
	char * line = NULL;
	int value = 0;

	line = strdup(s);

	err = at_tok_start(&line);
	if (err < 0) goto error;

	err = at_tok_nextint(&line, &response);
	if (err < 0) goto error;

/*
    ת����GSM���õ���ֵ���Ա��ϲ�ͳһ����
 */
    switch(response)
    {
    case 0:
        value = 0;
        break;
    case 20:
        value = 4;
        break;
    case 40:
        value = 12;
        break;
    case 60:
        value = 18;
        break;
    case 80:
        value = 26;
        break;
	case 99:
		value=34;
		break;
    default:
        value = 0;
        break;
    }
    
	signalStrength[0]=value;
	signalStrength[1]=99;

// cmy: ����RIL_UNSOL_SIGNAL_STRENGTH�ᵼ���ϲ㲻�ٶ�ʱ��ѯ�ź�ǿ��
//	RIL_onUnsolicitedResponse(RIL_UNSOL_SIGNAL_STRENGTH, response, sizeof(response));
	return;

error:
	/* The notification was for a battery event - do not send a msg to upper layers */
	return;
}
static void unsolicitedHDRRSSI(const char * s){//add by xxh

	int err;
	int response[2];
	char * line = NULL;

	line = strdup(s);

	err = at_tok_start(&line);
	if (err < 0) goto error;

	err = at_tok_nextint(&line, &(response[0]));
	if (err < 0) goto error;
	response[1]=99;
	LOGD("[%s]: HDR Uosolicited signalStrenth=%d\n", __FUNCTION__, response[0]);

	signalStrength_hdr[0]=response[0];
	signalStrength_hdr[1]=response[1];
//	free(line);
// cmy: ����RIL_UNSOL_SIGNAL_STRENGTH�ᵼ���ϲ㲻�ٶ�ʱ��ѯ�ź�ǿ��
//	RIL_onUnsolicitedResponse(RIL_UNSOL_SIGNAL_STRENGTH, response, sizeof(response));
	return;
error:
	/* The notification was for a battery event - do not send a msg to upper layers */
	return;


}
static void unsolicitedHRSSILVL(const char * s)
{
	int err;
	int response;
	char * line = NULL;
	int value = 0;

	line = strdup(s);

	err = at_tok_start(&line);
	if (err < 0) goto error;

	err = at_tok_nextint(&line, &response);
	if (err < 0) goto error;

/*
    ת����GSM���õ���ֵ���Ա��ϲ�ͳһ����
 */
    switch(response)
    {
    case 0:
        value = 0;
        break;
    case 20:
        value = 4;
        break;
    case 40:
        value = 12;
        break;
    case 60:
        value = 18;
        break;
    case 80:
        value = 26;
        break;
	case 99:
		value = 34;
		break;
    default:
        value = 99;
        break;
    }
    
	signalStrength_hdr[0]=value;
	signalStrength_hdr[1]=99;

	return;
error:
	/* The notification was for a battery event - do not send a msg to upper layers */
	return;
}
static void requestNotSupported(RIL_Token t)
{
	RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
	return;
}

static void requestSetFacilityLock(void *data, size_t datalen, RIL_Token t)
{
	/* It must be tested if the Lock for a particular class can be set without
	 * modifing the values of the other class. If not, first must call
	 * requestQueryFacilityLock to obtain the previus value
	 */
	int err = 0;
	char *cmd = NULL;
	char *code = NULL;
	char *lock = NULL;
	char *password = NULL;
	//char *class = NULL;
	ATResponse *p_response = NULL;

	assert (datalen >=  (4 * sizeof(char **)));

	code = ((char **)data)[0];
	lock = ((char **)data)[1];
	password = ((char **)data)[2];
//	class = ((char **)data)[3];

//	asprintf(&cmd, "AT+CLCK=\"%s\",%s,\"%s\",%s", code, lock, password, class);
           LOGD(" code=%s   lock=%s  password=%s ",code, lock, password);
           asprintf(&cmd, "AT+CLCK=\"%s\",%s,\"%s\"", code, lock, password);
    err = at_send_command(cmd, &p_response);
    free(cmd);
    if (err < 0 || p_response->success == 0){
        goto error;
    }

	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
	at_response_free(p_response);
	return;

error:
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	at_response_free(p_response);
}

static void requestChangeBarringPassword(void *data, size_t datalen, RIL_Token t)
{
	int err = 0;
	char *cmd = NULL;
	char *string = NULL;
	char *old_password = NULL;
	char *new_password = NULL;

	assert (datalen >=  (3 * sizeof(char **)));

	string	   = ((char **)data)[0];
	old_password = ((char **)data)[1];
	new_password = ((char **)data)[2];

	asprintf(&cmd, "AT+CPWD=\"%s\",\"%s\",\"%s\"", string, old_password, new_password);
	err = at_send_command(cmd, NULL);

	free(cmd);

	if (err < 0) goto error;

	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
	return;

error:
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

/* cmy: Ŀǰ�ֶ�ѡ������ķ���
        1 �ɹ�����¼��ǰѡ�������
        2 ʧ�ܣ���������Ϊ��AT+COPS=0���Ҽ�¼Ϊ"�Զ�ѡ������"
        ��ǰ����ѡ��Ǽ��������ļ�:
            /data/data/com.android.phone/shared_prefs/com.android.phone_preferences.xml
 */
/* cmy: �ֶ�ѡ������(�й��ƶ����й���ͨ...)
    AT+COPS=1,2,"46000"
 */
static void requestSetNetworkSelectionManual(void *data, size_t datalen, RIL_Token t)
{
	char *operator = NULL;
	char *cmd = NULL;
	int err = 0;
	ATResponse *p_response = NULL;

    if( !isgsm )
    {
        RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
        return;
    }
    
	operator = (char *)data;
	asprintf(&cmd, "AT+COPS=1,2,\"%s\"", operator);
// cmy: ������30��ĳ�ʱʱ��
// ��ʱ����ʱ�� p_response = null
	err = at_send_command(cmd, &p_response);
//	LOGD("err=%d  p_response=%p", err, p_response);
    if (err < 0 || p_response == NULL || p_response->success==0 ){
        LOGD("network manual select failed, reset to auto select");
        at_send_command("AT+COPS=0", &p_response);
        goto error;
	}

	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
	at_response_free(p_response);
	free(cmd);
	return;

error:
	at_response_free(p_response);
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestResetRadio(RIL_Token t)
{
	int err = 0;

//	err = at_send_command("AT+CFUN=16", NULL);
	if(err < 0)
		RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	else
		RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);

	return;
}

static void requestSetSuppSVCNotification(void *data, size_t datalen, RIL_Token t)
{
	int err = 0;
	int enabled = 0;
	char *cmd = NULL;
	enabled = ((int *)data)[0];

	asprintf(&cmd, "AT+CSSN=%d,%d", enabled, enabled);
	err = at_send_command(cmd,NULL);
	free(cmd);
	if(err < 0)
		RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	else
		RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

static void requestSetLocationUpdates(void *data, size_t datalen, RIL_Token t)
{
	int err = 0;
	int updates = 0;
	char *cmd = NULL;
	ATResponse *p_response = NULL;
	updates = ((int *)data)[0] == 1? 2 : 1;

	asprintf(&cmd, "AT+CREG=%d", updates);

// cmy@20101223: 
//	err = at_send_command_singleline(cmd,"+CREG:",&p_response);
	err = at_send_command(cmd, &p_response);
	if(err < 0 || p_response->success == 0) goto error;

	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
	at_response_free(p_response);
	return;

error:
	at_response_free(p_response);
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestLastFailCause(RIL_Token t)
{
	ATResponse *p_response = NULL;
	int err = 0;
	int response = 0;
	char *tmp = NULL;
	char *line = NULL;

	err = at_send_command_singleline("AT+CEER", "+CEER:", &p_response);
	if(err < 0 || p_response->success == 0) goto error;

	line = p_response->p_intermediates->line;
	err = at_tok_start(&line);
	if(err < 0) goto error;

	err = at_tok_nextstr(&line, &tmp);
	if(err < 0) goto error;

	err = at_tok_nextint(&line, &response);
	if(err < 0) goto error;

	RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));
	at_response_free(p_response);
	return;

error:
	at_response_free(p_response);
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static int GetIMSI(char* imsi)
{
	ATResponse *p_response = NULL;
	char *line;
	char *response;
	char *part;
	int err;

    int loop = 0;
    int success = 0;
    /* We are looping here because the command fails on the first try.
       What needs to be done, is to trap the "+CME ERROR: 14" which means
       SIM BUSY and retry that. As a workaround for now, simply try, wait
       1 second, and try again, until a valid result is obtained. Usually only
       takes 2 tries.
    */
/*
    +CIMI: 460020059343258
           460019602709144
 */
    for(loop=2; loop>0; loop--)
    {
    	err = at_send_command_numeric("AT+CIMI", &p_response);
    	if (err < 0 || p_response->success == 0)
    	{
    	    at_response_free(p_response);
    	    p_response = NULL;

            if (!modem_cmp(0x19D2, 0xFFF1, "AC27XX"))
        	   // response = getCommandResponse("AT+CIMI", "CIMI:", 1);
        	   //����AC2787
               response = getCommandResponse("AT+CIMI", "^CIMI:", 1);
            else if(!modem_cmp(0x20A6, 0x1105, "Test number")){
				response="460039602709144";//�ֻ�modem
				}
	    else if(!modem_cmp(0x05C6, 0x9000, NULL)) {
	    			response="460016110776127";
	    }
            else{
        	    err = at_send_command_singleline("AT+CIMI", "+CIMI:", &p_response);
        	    if (err < 0 || p_response->success == 0) goto loop_error;
                line = p_response->p_intermediates->line;
                err = at_tok_start(&line);
                if(err < 0) goto loop_error;
                err = at_tok_nextstr(&line, &response);
                if(err < 0) goto loop_error;
            }
    	}
    	else
    	{
    	    response = p_response->p_intermediates->line;
    	}
    	strcpy(imsi, response);
    	
    	success=1;
    	break;
    	
loop_error:    	
	    at_response_free(p_response);
	    p_response = NULL;
    }
    
    if (success == 0) goto error;

	at_response_free(p_response);
	return 0;
	
error:
	at_response_free(p_response);
	return -1;
}

static void requestGetIMSI(RIL_Token t)
{
    if( GetIMSI(IMSI) != 0 )
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    else
        RIL_onRequestComplete(t, RIL_E_SUCCESS,IMSI, sizeof(char *));
}

/*
    other - ok
    NULL - failed
 */
static char* check_did(const char* did)
{
    if( strlen(did) < 8 )
        return NULL;
    
	if(did[0]=='0' && (did[1]=='x' || did[1]=='X') )
	    did += 2;

    int count = 0;
    char v = 0;
    while( (v = did[count++]) !=0 )
    {
        if( !((v>='0' && v<='9') || (v>='a' && v<='f') || (v>='A' && v<='F')) )
            break;
    }
//    LOGD("count=%d, did=%s", count, did);

    return (v==0)?did:NULL;
}

static void requestGetIMEI(RIL_Token t, int request)
{
	int err = 0;
	char *response = NULL;

/*
    +CGSN: 860061000450164
    860061000450164
	case RADIO_TYPE_GSM:
		case RADIO_TYPE_WCDMA:
		case RADIO_TYPE_TDSCDMA:

    +GSN:a3e2503b
    0xA3E2503B
 */
    if( s_current_modem->radioType==RADIO_TYPE_GSM ||
		s_current_modem->radioType==RADIO_TYPE_WCDMA||
		s_current_modem->radioType==RADIO_TYPE_TDSCDMA)
    {    

        response = getCommandResponse("AT+CGSN", "GSN:", 3);
    }
    else
    {
    /*
        AT> AT^MEID
        AT< ^MEID:00000000000000
    */
        response = getCommandResponse("AT^MEID", "^MEID:", 3);
        
    	if ( response==NULL )
    	    goto meid_err;

    	if( check_did(response)==NULL || atox(response,16)==0 )
    	    goto meid_err;

        RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(char *));
    	free(response);
    	return;

meid_err:
        free(response);
        LOGD("get meid failed or invalid meid!");
        response = getCommandResponse("AT+GSN", "GSN:", 3);
	}
    
	if (response==NULL)
	    goto error;

    if( check_did(response) == NULL )
        goto error;

    char *tResponse = check_did(response);
    
    RIL_onRequestComplete(t, RIL_E_SUCCESS, tResponse, sizeof(char *));
	free(response);

	if(nosimcard==1){

      setModemPower(0);   //��ѯ��IMEI���Źر�ģ��    
	}
	return;

// cmy: ����Modem(E169)��ѯIMEIʧ�ܣ��ڴ�����·���һ����ٵ�ֵ
#if 0
error:
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	at_response_free(p_response);
#else
error:
	free(response);
    if( isgsm )
    	{
        response = "123456789012345"; // IMEI
        LOGD("CURRENT IMEI IS 123456789012345!");
    	}
    else
        response = "12345678"; // ESN

	RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(char *));
//	at_response_free(p_response);
#endif
}

static void requestGetIMEISV(RIL_Token t, int request)
{
   char imeisv[4];

   if(!modem_cmp(0x12d1, 0x1506, "E1220")) {

	char *response = NULL;

	response = getCommandResponse("AT^ICCID?", "^ICCID:", 3);
	
	RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(char *));


	} else {
     	
	char imeisv[4] = "00";
     
	RIL_onRequestComplete(t, RIL_E_SUCCESS, imeisv, sizeof(char *));

	}
	
   return;
}

/* cmy: �Զ�ѡ������
 */
static void requestSetNetworkSelectionAutomatic(RIL_Token t)
{
	int err = 0;

    LOGD("Enter requestSetNetworkSelectionAutomatic");
//cmy: cdma not support 'cops' commands
    if(!isgsm)
    {
        LOGD("[%s]: CDMA not supported!", __FUNCTION__);
    	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    	return;
    }

// cmy: for AST
#if 1
// cmy: ���Ŀǰ�Ѿ����Զ�ѡ��ģʽ�������ٴ����ã�����ᵼ������ע������
    if( queryNetworkSelectionMode() != 0 )
    {
        err = at_send_command("AT+COPS=0", NULL);
    }
#endif    
    
	if(err < 0)
		RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	else
		RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);

	return;
}

static void requestSendUSSD(void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err = 0;
    int len;
    cbytes_t ussdRequest;
    char *cmd;
    bytes_t temp;
    char *newUSSDRequest;
    if(isgsm) {
        ussdRequest = (cbytes_t)(data);
        temp = malloc(strlen((char *)ussdRequest)*sizeof(char)+1);
        len = utf8_to_gsm8(ussdRequest,strlen((char *)ussdRequest),temp);
        newUSSDRequest = malloc(2*len*sizeof(char)+1);
        gsm_hex_from_bytes(newUSSDRequest,temp, len);
        newUSSDRequest[2*len]='\0';
        asprintf(&cmd, "AT+CUSD=1,\"%s\",15", newUSSDRequest);
        free(newUSSDRequest);
        free(temp);
        err = at_send_command(cmd, &p_response);
        free(cmd);
        if (err < 0 || p_response->success == 0) {
            goto error;
        } else {
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        }
        at_response_free(p_response);
    } else {
        RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
    }
    return;

error:
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestCancelUSSD(RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;

    err = at_send_command_numeric("AT+CUSD=2", &p_response);

    if (err < 0 || p_response->success == 0){
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
    else{
        RIL_onRequestComplete(t, RIL_E_SUCCESS,
                p_response->p_intermediates->line, sizeof(char *));
    }

    at_response_free(p_response);
    return;
}

#ifdef SUPPORT_SMS
static void requestSendSMS(void *data, size_t datalen, RIL_Token t)
{
    int err;
    const char *smsc;
    const char *pdu;
    int tpLayerLength;
    char *cmd1, *cmd2;
    RIL_SMS_Response response;
    ATResponse *p_response = NULL;

    smsc = ((const char **)data)[0];
    pdu = ((const char **)data)[1];

    tpLayerLength = strlen(pdu)/2;

    // "NULL for default SMSC"
    if (smsc == NULL) {
        smsc= "00";
    }

   LOGD("SMSC==%s                     PDU=%s",smsc,pdu);
    asprintf(&cmd1, "AT^HCMGS=%d", tpLayerLength);
    asprintf(&cmd2, "%s%s", smsc, pdu);

    err = at_send_command_sms(cmd1, cmd2, "^HCMGS:", &p_response);
	free(cmd1);free(cmd2);
    if (err != 0 || p_response->success == 0) goto error;

    memset(&response, 0, sizeof(response));

    /* FIXME fill in messageRef and ackPDU */

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(response));
    at_response_free(p_response);

    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}
static void requestWriteSmsToSim(void *data, size_t datalen, RIL_Token t)
{
    RIL_SMS_WriteArgs *p_args;
    char *cmd;
    int length;
    int err;
    ATResponse *p_response = NULL;

    p_args = (RIL_SMS_WriteArgs *)data;

    length = strlen(p_args->pdu)/2;
    asprintf(&cmd, "AT+CMGW=%d,%d", length, p_args->status);

    err = at_send_command_sms(cmd, p_args->pdu, "+CMGW:", &p_response);
	free(cmd);

    if (err != 0 || p_response->success == 0) goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);

    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}
#endif
#if 0
static void  unsolicitedUSSD(const char *s)
{
    char *line, *linestart;
    int typeCode, count, err, len;
    unsigned char *message;
    unsigned char *outputmessage;
    char *responseStr[2];

    LOGD("unsolicitedUSSD %s\n",s);

    linestart=line=strdup(s);
    err = at_tok_start(&line);
    if(err < 0) goto error;

    err = at_tok_nextint(&line, &typeCode);
    if(err < 0) goto error;

    if(at_tok_hasmore(&line)) {
        err = at_tok_nextstr(&line, &message);
        if(err < 0) goto error;
        outputmessage = malloc(strlen(message)*2+1);
        gsm_hex_to_bytes(message,strlen(message),outputmessage);
        responseStr[1] = malloc(strlen(outputmessage)*2+1);
        len = utf8_from_gsm8(outputmessage,strlen(outputmessage),responseStr[1]);
        responseStr[1][strlen(message)/2]='\0';
        free(outputmessage);
        count = 2;
    } else {
        responseStr[1]=NULL;
        count = 1;
    }
    free(linestart);
    asprintf(&responseStr[0], "%d", typeCode);

    RIL_onUnsolicitedResponse (RIL_UNSOL_ON_USSD, responseStr, count*sizeof(char*));
    return;

error:
    LOGE("unexpectedUSSD error\n");
}
#endif
static void requestNeighboringCellIds(void * data, size_t datalen, RIL_Token t)
{
    int i=0;
    int response[4]={0};
    int count = 0;
    RIL_NeighboringCell *p_cellIds;

    p_cellIds = (RIL_NeighboringCell *)alloca(sizeof(RIL_NeighboringCell));
    
    for (i=0; i<4; i++) {
        if( !getRegistrationInfo(&response[0], &count) )
            break;
    }
    if (i<4)
    {
        p_cellIds[0].rssi = 2;
        if(s_current_modem->modem_info.vid==0x19d2){
          asprintf(&(p_cellIds[0].cid), "%08x", response[2]);
		}else
        asprintf(&(p_cellIds[0].cid), "%x", response[2]);
        RIL_onRequestComplete(t, RIL_E_SUCCESS, &p_cellIds, sizeof(p_cellIds));
    }
    else
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);

 
}



/*** Callback methods from the RIL library to us ***/

/**
 * Call from RIL to us to make a RIL_REQUEST
 *
 * Must be completed with a call to RIL_onRequestComplete()
 *
 * RIL_onRequestComplete() may be called from any thread, before or after
 * this function returns.
 *
 * Will always be called from the same thread, so returning here implies
 * that the radio is ready to process another command (whether or not
 * the previous command has completed).
 */
static void
onRequest (int request, void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response;
    int err;

    RLOGD("onRequest: %s", requestToString(request));


    /* Ignore all requests except RIL_REQUEST_GET_SIM_STATUS
     * when RADIO_STATE_UNAVAILABLE.
     */
    if (sState == RADIO_STATE_UNAVAILABLE
        && request != RIL_REQUEST_GET_SIM_STATUS
    ) {
        RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
        return;
    }

    /* Ignore all non-power requests when RADIO_STATE_OFF
     * (except RIL_REQUEST_GET_SIM_STATUS)
     */
    if (sState == RADIO_STATE_OFF
        && !(request == RIL_REQUEST_RADIO_POWER
            || request == RIL_REQUEST_GET_SIM_STATUS)
    ) {
        RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
        return;
    }

    switch (request) {
        case RIL_REQUEST_GET_SIM_STATUS:
#if 0
		{
		    int simStatus;
		    simStatus = getSIMStatus();
		    RIL_onRequestComplete(t, RIL_E_SUCCESS, &simStatus, sizeof(simStatus));
		    break;
		} 
#else
        {
            RIL_CardStatus *p_card_status;
            char *p_buffer;
            int buffer_size;

            int result = getCardStatus(&p_card_status);
            if (result == RIL_E_SUCCESS) {
                p_buffer = (char *)p_card_status;
                buffer_size = sizeof(*p_card_status);
            } else {
                p_buffer = NULL;
                buffer_size = 0;
            }
            RIL_onRequestComplete(t, result, p_buffer, buffer_size);
            freeCardStatus(p_card_status);
            break;
        }
#endif       
/*	case RIL_REQUEST_VOICE_RADIO_TECH:
	{
		int tech = RADIO_TECH_HSPA;
		RIL_onRequestComplete(t, RIL_E_SUCCESS, &tech, sizeof(tech));
		break;
	}*/
        case RIL_REQUEST_GET_CURRENT_CALLS:
            requestGetCurrentCalls(data, datalen, t);
            break;
        case RIL_REQUEST_SIGNAL_STRENGTH:
            requestSignalStrength(data, datalen, t);
            break;

        case RIL_REQUEST_REGISTRATION_STATE:
        case RIL_REQUEST_GPRS_REGISTRATION_STATE:
            requestRegistrationState(request, data, datalen, t);
            break;

		case RIL_REQUEST_SCREEN_STATE:
			requestScreenState(data, datalen, t);
			break;

        case RIL_REQUEST_OPERATOR:
            requestOperator(data, datalen, t);
            break;
        case RIL_REQUEST_RADIO_POWER:
            requestRadioPower(data, datalen, t);
            break;
		case RIL_REQUEST_QUERY_FACILITY_LOCK:
			requestQueryFacilityLock(data, datalen, t);
			break;

        case RIL_REQUEST_SETUP_DATA_CALL:
            requestSetupDataCall(data, datalen, t);
            break;

		case RIL_REQUEST_DEACTIVATE_DATA_CALL:
			requestDeactivateDefaultPDP(data, datalen, t);
			break;

		case RIL_REQUEST_GET_IMSI:
			requestGetIMSI(t);
			break;

		case RIL_REQUEST_BASEBAND_VERSION:
			requestBasebandVersion(data, datalen, t);
			// requestBaseBandVersionT(request, data, datalen, t);
			break;

        case RIL_REQUEST_GET_IMEI:
            requestGetIMEI(t, request);
            break;
		case RIL_REQUEST_GET_IMEISV:
            requestGetIMEISV(t, request);
            break;
        case RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC:
			requestSetNetworkSelectionAutomatic(t);
            break;

        case RIL_REQUEST_DATA_CALL_LIST:
            requestDataCallList(data, datalen, t);
            break;

        case RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE:
            requestQueryNetworkSelectionMode(data, datalen, t);
            break;

		case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS:
			requestQueryAvailableNetworksxxh(data, datalen, t);
			break;

		case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE:
			requestGetPreferredNetworkType(data, datalen, t);
			break;

		case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE:
			requestSetPreferredNetworkType(data, datalen, t);
			break;

        case RIL_REQUEST_ENTER_SIM_PIN:
        case RIL_REQUEST_ENTER_SIM_PUK:
        case RIL_REQUEST_ENTER_SIM_PIN2:
        case RIL_REQUEST_ENTER_SIM_PUK2:
        case RIL_REQUEST_CHANGE_SIM_PIN:
        case RIL_REQUEST_CHANGE_SIM_PIN2:
            requestEnterSimPin(request,data, datalen, t);
            break;

		case RIL_REQUEST_ENTER_NETWORK_DEPERSONALIZATION:
			// NOTE: There isn't an AT command with this capability
			requestNotSupported(t);
			break;

		case RIL_REQUEST_SET_FACILITY_LOCK:
			requestSetFacilityLock(data, datalen, t);
			break;

		case RIL_REQUEST_CHANGE_BARRING_PASSWORD:
			requestChangeBarringPassword(data, datalen, t);
			break;

		case RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL:
			requestSetNetworkSelectionManual(data, datalen, t);
			break;

		case RIL_REQUEST_RESET_RADIO:
			requestResetRadio(t);
			break;

		case RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION:
			requestSetSuppSVCNotification(data, datalen, t);
			break;

		case RIL_REQUEST_SET_LOCATION_UPDATES:
			requestSetLocationUpdates(data, datalen, t);
			break;

		case RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE:
		case RIL_REQUEST_LAST_CALL_FAIL_CAUSE:
			requestLastFailCause(t);
			break;
       //case RIL_REQUEST_SIM_IO:
             //���Է��ֶ�CDMA��֧��
			//if(isgsm){
			// LOGD("=======huawei wcdma sim io======");	
			 //requestSIM_IO(data,datalen,t);
			//}
			
            //break;
        //case RIL_REQUEST_SEND_USSD:
           // requestSendUSSD(data, datalen, t);
            //break;
            
        //case RIL_REQUEST_CANCEL_USSD:
            //requestCancelUSSD(t);
            //break;

        case RIL_REQUEST_GET_NEIGHBORING_CELL_IDS:
            requestNeighboringCellIds(data, datalen, t);
            break;

#ifdef SUPPORT_SMS
	     case RIL_REQUEST_SEND_SMS:
	        requestSendSMS(data, datalen, t);
	         break;
		 case RIL_REQUEST_CDMA_SEND_SMS:
		 	//requestSendCdmaSms(data, datalen, t);
			break;
		 case RIL_REQUEST_WRITE_SMS_TO_SIM:
            requestWriteSmsToSim(data, datalen, t);
            break;
	     case RIL_REQUEST_DELETE_SMS_ON_SIM: {
            char * cmd;
            p_response = NULL;
            asprintf(&cmd, "AT+CMGD=%d", ((int *)data)[0]);
            err = at_send_command(cmd, &p_response);
            free(cmd);
            if (err < 0 || p_response->success == 0) {
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
            } else {
                RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            }
            at_response_free(p_response);
            break;
        }
#endif
        default:
			requestNotSupported(t);
            break;
    }
}

/**
 * Synchronous call from the RIL to us to return current radio state.
 * RADIO_STATE_UNAVAILABLE should be the initial state.
 */
static RIL_RadioState
currentState()
{
    return sState;
}
/**
 * Call from RIL to us to find out whether a specific request code
 * is supported by this implementation.
 *
 * Return 1 for "supported" and 0 for "unsupported"
 */

static int
onSupports (int requestCode)
{
    //@@@ todo

    return 1;
}

static void onCancel (RIL_Token t)
{
    //@@@todo

}

static const char * getVersion(void)
{
    return REFERENCE_RIL_VERSION;
}

static void
setRadioState(RIL_RadioState newState)
{
#ifdef DBG_1
    LOGD("[%s]: oldstate=%d  newstate=%d  s_closed=%d\n", __FUNCTION__, sState, newState, s_closed);
#endif

    RIL_RadioState oldState;

    pthread_mutex_lock(&s_state_mutex);

    oldState = sState;

    if (s_closed > 0) {
        // If we're closed, the only reasonable state is
        // RADIO_STATE_UNAVAILABLE
        // This is here because things on the main thread
        // may attempt to change the radio state after the closed
        // event happened in another thread
        newState = RADIO_STATE_UNAVAILABLE;
    }

    if (sState != newState || s_closed > 0) {
        sState = newState;

        pthread_cond_broadcast (&s_state_cond);
    }

    pthread_mutex_unlock(&s_state_mutex);


    /* do these outside of the mutex */
    if (sState != oldState) {
        RIL_onUnsolicitedResponse (RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED,
                                    NULL, 0);

        /* FIXME onSimReady() and onRadioPowerOn() cannot be called
         * from the AT reader thread
         * Currently, this doesn't happen, but if that changes then these
         * will need to be dispatched on the request thread
         */
        if (sState == RADIO_STATE_ON) {
            onRadioPowerOn();
        }
    }
}

/** Returns SIM_NOT_READY on error */
static SIM_Status 
getSIMStatus()
{
    ATResponse *p_response = NULL;
    int err;
    int ret;
    char *cpinLine;
    char *cpinResult;
    static int sim_absent = 0;
#ifdef DBG_1
    LOGD("[%s]: sState=%d\n", __FUNCTION__, sState);
#endif

    if (sState == RADIO_STATE_OFF || sState == RADIO_STATE_UNAVAILABLE) {
        ret = SIM_NOT_READY;
        goto done;
    }

    if(!modem_cmp(0x1E89, 0x1E16, NULL) && !strcmp(getModelID(), "172"))
        err = at_send_command_singleline("AT^CPIN?", "^CPIN:", &p_response);
    else
        err = at_send_command_singleline("AT+CPIN?", "+CPIN:", &p_response);
    
#if 1
    if (err < 0) {
        ret = SIM_NOT_READY;
        goto done;
    }
#else
    if (err < 0 || p_response->success == 0) {
        ret = SIM_NOT_READY;
        goto done;
    }
#endif

    switch (at_get_cme_error(p_response)) {
    case CME_SUCCESS:
        break;

    case CME_SIM_NOT_INSERTED:
// cmy: ����W��ʽ��Modem��δ��SIM��ʱ����   +CME ERROR: 13
    case CME_SIM_FAILED:
        ret = SIM_ABSENT;
        goto done;

    default:
        if(tryCount > 5)
            ret = SIM_ABSENT;
        else
            ret = SIM_NOT_READY;
        goto done;
    }

    /* CPIN? has succeeded, now look at the result */

    cpinLine = p_response->p_intermediates->line;
    err = at_tok_start (&cpinLine);

    if (err < 0) {
        ret = SIM_NOT_READY;
        goto done;
    }

    err = at_tok_nextstr(&cpinLine, &cpinResult);

    if (err < 0) {
        ret = SIM_NOT_READY;
        goto done;
    }

    if (0 == strncmp (cpinResult, "SIM PIN", strlen("SIM PIN"))) {
        ret = SIM_PIN;
		sim_pin =1;
        goto done;
    } else if (0 == strncmp (cpinResult, "SIM PUK", strlen("SIM PUK"))) {
        ret = SIM_PUK;
        goto done;
    } else if (0 == strncmp (cpinResult, "PH-NET PIN", strlen("PH-NET PIN"))) {
        return SIM_NETWORK_PERSONALIZATION;
    } else if (0 != strncmp (cpinResult, "READY", strlen("READY")))  {
        /* we're treating unsupported lock types as "sim absent" */
        ret = SIM_ABSENT;
        goto done;
    }

    at_response_free(p_response);
    p_response = NULL;
    cpinResult = NULL;
	sim_absent = 0;
    ret = SIM_READY;

done:
	LOGD("########check sim ready or not ready");
    if(SIM_ABSENT == ret && s_current_modem->inner==1)//�������������ر�����ģ�飬���͹���
	{
        //setModemPower(0);
     LOGD("########Inner 3G modem has no sim card,will shut down power later..");
     nosimcard=1;
	}
    at_response_free(p_response);

    RLOGD("getSIMStatus return: %d", ret);
    if(ret == SIM_ABSENT ){
    RLOGD("find no sim card,check sim card slot or insert sim card");	    
    }
    return ret;
}


/**
 * Get the current card status.
 *
 * This must be freed using freeCardStatus.
 * @return: On success returns RIL_E_SUCCESS
 */
static int getCardStatus(RIL_CardStatus **pp_card_status) {
    
    static RIL_AppStatus app_status_array[] = {
        // SIM_ABSENT = 0
        { RIL_APPTYPE_UNKNOWN, RIL_APPSTATE_UNKNOWN, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // SIM_NOT_READY = 1
        { RIL_APPTYPE_SIM, RIL_APPSTATE_DETECTED, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // SIM_READY = 2
        { RIL_APPTYPE_SIM, RIL_APPSTATE_READY, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // SIM_PIN = 3
        { RIL_APPTYPE_SIM, RIL_APPSTATE_PIN, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // SIM_PUK = 4
        { RIL_APPTYPE_SIM, RIL_APPSTATE_PUK, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_BLOCKED, RIL_PINSTATE_UNKNOWN },
        // SIM_NETWORK_PERSONALIZATION = 5
        { RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_NETWORK,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // RUIM_ABSENT = 6
        { RIL_APPTYPE_UNKNOWN, RIL_APPSTATE_UNKNOWN, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // RUIM_NOT_READY = 7
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_DETECTED, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // RUIM_READY = 8
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_READY, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // RUIM_PIN = 9
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_PIN, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // RUIM_PUK = 10
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_PUK, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_BLOCKED, RIL_PINSTATE_UNKNOWN },
        // RUIM_NETWORK_PERSONALIZATION = 11
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_NETWORK,
           NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
	// ISIM_ABSENT = 12
	{ RIL_APPTYPE_UNKNOWN, RIL_APPSTATE_UNKNOWN, RIL_PERSOSUBSTATE_UNKNOWN,
	   NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
	// ISIM_NOT_READY = 13
	{ RIL_APPTYPE_ISIM, RIL_APPSTATE_DETECTED, RIL_PERSOSUBSTATE_UNKNOWN,
	   NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
	// ISIM_READY = 14
	{ RIL_APPTYPE_ISIM, RIL_APPSTATE_READY, RIL_PERSOSUBSTATE_READY,
	   NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
	// ISIM_PIN = 15
	{ RIL_APPTYPE_ISIM, RIL_APPSTATE_PIN, RIL_PERSOSUBSTATE_UNKNOWN,
	   NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
	// ISIM_PUK = 16
	{ RIL_APPTYPE_ISIM, RIL_APPSTATE_PUK, RIL_PERSOSUBSTATE_UNKNOWN,
	   NULL, NULL, 0, RIL_PINSTATE_ENABLED_BLOCKED, RIL_PINSTATE_UNKNOWN },
	// ISIM_NETWORK_PERSONALIZATION = 17
	{ RIL_APPTYPE_ISIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_NETWORK,
	   NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
    };
    RIL_CardState card_state;
    int num_apps;

    int sim_status = getSIMStatus();
    if (sim_status == SIM_ABSENT) {
        card_state = RIL_CARDSTATE_ABSENT;
        num_apps = 0;
    } else {
        card_state = RIL_CARDSTATE_PRESENT;
        num_apps = 3;
    }

    // Allocate and initialize base card status.
    RIL_CardStatus_v6 *p_card_status = malloc(sizeof(RIL_CardStatus_v6));
    p_card_status->card_state = card_state;
    p_card_status->universal_pin_state = RIL_PINSTATE_UNKNOWN;
    p_card_status->gsm_umts_subscription_app_index = RIL_CARD_MAX_APPS;
    p_card_status->cdma_subscription_app_index = RIL_CARD_MAX_APPS;
    p_card_status->ims_subscription_app_index = RIL_CARD_MAX_APPS;
    p_card_status->num_applications = num_apps;

    // Initialize application status
    int i;
    for (i = 0; i < RIL_CARD_MAX_APPS; i++) {
        p_card_status->applications[i] = app_status_array[SIM_ABSENT];
    }

    // Pickup the appropriate application status
    // that reflects sim_status for gsm.
    if (num_apps != 0) {
        // Only support one app, gsm
        p_card_status->num_applications = 3;
        p_card_status->gsm_umts_subscription_app_index = 0;
        p_card_status->cdma_subscription_app_index = 1;
	p_card_status->ims_subscription_app_index = 2;

        // Get the correct app status
        p_card_status->applications[0] = app_status_array[sim_status];
        p_card_status->applications[1] = app_status_array[sim_status + RUIM_ABSENT];
	p_card_status->applications[2] = app_status_array[sim_status + ISIM_ABSENT];
    }

    *pp_card_status = p_card_status;
    return RIL_E_SUCCESS;
}

/**
 * Free the card status returned by getCardStatus
 */
static void freeCardStatus(RIL_CardStatus *p_card_status) {
    free(p_card_status);
}

/*
    cmy@20101029:���жϸ�SIM���Ƿ�֧��
        ��������֧��
        ���������˿���֧��
 */
static int isCardNotSupported()
{
#if 1
    int isNotSupported = 0;
    LOGD("isCardNotSupported: %d", isNotSupported);
    return isNotSupported;
#else
    int isNotSupported = 1;

    ATResponse *p_response = NULL;
    char * line;
    int err;
    
    err = at_send_command_singleline("AT+ZGETMCCLOCK", "+ZCCRTB:", &p_response);
    
    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &isNotSupported);
    if (err < 0) goto error;
    
error:
    at_response_free(p_response);
    LOGD("isCardNotSupported: %d", isNotSupported);
    return isNotSupported;
#endif
}

/**
 * SIM ready means any commands that access the SIM will work, including:
 *  AT+CPIN, AT+CSMS, AT+CNMI, AT+CRSM
 *  (all SMS-related commands)
 */
static void pollSIMState (void *param)
{
    ATResponse *p_response;
    int ret;

    if (sState != RADIO_STATE_UNAVAILABLE) {
        // no longer valid to poll
        return;
    }
	//India EVDO no need sim card by return SIM Ready
	if(!modem_cmp(0x12D1, 0x140B,NULL)
		||!modem_cmp(0x19D2, 0xFFE8,NULL)
		||!modem_cmp(0x19D2, 0xFFF1, "AC27XX")
		||!modem_cmp(0x1bbb, 0x0012,"MDM")){
			RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);
			return;
	}else{

    switch(getSIMStatus()) {
        case SIM_ABSENT:
        case SIM_PIN:
        case SIM_PUK:
        case SIM_NETWORK_PERSONALIZATION:
        default:
            RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);
            return;

        case SIM_NOT_READY:
            ++tryCount;
            RIL_requestTimedCallback (pollSIMState, NULL, &TIMEVAL_SIMPOLL);
            return;

        case SIM_READY:
        /* ���ڲ��ܹ�֧�ֵ�SIM�������ϲ㷢��֪ͨ
         */
            if( isCardNotSupported() )
            {
                setRadioState(RADIO_STATE_UNAVAILABLE);

//                int isSimNotSupport = 1;
//                RIL_onUnsolicitedResponse(RIL_UNSOL_SIM_NOT_SUPPORT,
//                                          &isSimNotSupport,
//                                          sizeof(int) );
            }
            else
            {
                RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);
                
//                int isSimNotSupport = 0;
//                RIL_onUnsolicitedResponse(RIL_UNSOL_SIM_NOT_SUPPORT,
//                                          &isSimNotSupport,
//                                          sizeof(int) );
            }
        return;
    }
	}
}

/** returns 1 if on, 0 if off, and -1 on error */
static int isRadioOn()
{
    ATResponse *p_response = NULL;
    int err;
    char *line;
    char ret;

    err = at_send_command_singleline("AT+CFUN?", "+CFUN:", &p_response);

    if (err < 0 || p_response->success == 0) {
        // assume radio is off
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

   /* if( 0 == modem_cmp(0x12d1, 0x1003, NULL) )
    {
        if( (err=at_tok_nextint(&line, &ret)) >= 0 )
        {
             if(ret == 0)
                ret = 1;
            LOGE("=======change CFUN value ======");
        }
    }
    else*/
        err = at_tok_nextbool(&line, &ret);

    if (err < 0) goto error;

    at_response_free(p_response);

    LOGD("[%s]: ret = %d\n", __FUNCTION__,(int)ret);
    return (int)ret;

error:

    at_response_free(p_response);
    LOGD("[%s]: error ret = -1\n", __FUNCTION__);
    return -1;
}

extern void closeSocket();

static void closeATReader()
{

    LOGD("Close AT Reader!");
	if(isEth){
     system("netcfg lte0 down");
	 LOGD("Close eth inferface!");
	}else
    kill_pppd(25);
    kill_rild();
    at_close();
    s_closed = 1;

//    LOGD("free wait cond");
//    pthread_cond_signal(&s_waitcond);

    s_current_modem = NULL;
    setRadioState (RADIO_STATE_UNAVAILABLE);
}

/* Called on command or reader thread */
static void onATReaderClosed()
{
    LOGI("on AT channel closed\n");
    closeATReader();
//    closeSocket();
    LOGD("END onATReaderClosed");
}

static void modemEarlyInit()
{
    LOGD("modem early init");
	/*  echo off */
	at_send_command("ATE0", NULL);

	/*  No auto-answer */
	at_send_command("ATS0=0", NULL);

	/*  set DTR according to service */
	at_send_command("AT&D1", NULL);

	/*  Extended errors */
	at_send_command("AT+CMEE=1", NULL);
	if(!modem_cmp(0x201e, 0x1022,NULL))
	{
		
	at_send_command("AT^SWVER", NULL);
		
	at_send_command("AT^SDEN=27", NULL);

	}
     //xxh:����AT����ָʾ��
	if(!modem_cmp(0x230D, 0x000D, NULL))

	{
	
	 at_send_command("AT*ELED=1,1,0",NULL);

	}
	if(!modem_cmp(0x12D1, 0x1001, "3G HSDPA MODEM")){

	at_send_command("AT+ZSNT=0,0,2",NULL);

	}

	//if(!modem_cmp(0x0e8d,0x00A2,NULL)){

      //LOGD("[%s]: acquire_USI_wake_lock: %s\n", __FUNCTION__, WAKE_LOCK_ID);
        //acquire_wake_lock(PARTIAL_WAKE_LOCK, WAKE_LOCK_ID);
//	}
	if(!modem_cmp(0x1C9E, 0x9603, NULL)){


	at_send_command("AT+LCTNVUART=0",NULL);//xxh:ģ������ʱ��
	at_send_command("AT+SLEEPEN=1",NULL);

	}
          if(!modem_cmp(0x1782, 0x0002, "V1.0.1-B7")){
             at_send_command("AT+SIPMODE=0",NULL);
          //        err=at_send_command("AT+SAUTOATT=1",NULL);
        //          err=at_send_command("AT+SATT=1,0",NULL);
        }
	at_send_command("AT+CREG=2",NULL);
	at_send_command("AT+CGREG=2",NULL);
	at_send_command("AT+CREG?",NULL);
	at_send_command("AT+CGREG?",NULL);

}

/**
 * Initialize everything that can be configured while we're still in
 * AT+CFUN=0
 */
static void initializeCallback(void *param)
{
// ����radio״̬ΪRADIO_STATE_OFF���ϲ��յ�״̬�ı����Ϣ����ʾ��radioͼ��
    //setRadioState(RADIO_STATE_ON);

// ����AT����
    if(at_handshake())
    {
        LOGE("AT handshake failed!!!");
        return;
    }
#if GET_USI_IMEI
   int fd;
   int i;
   if(!modem_cmp(0x0e8d,0x00a2,NULL)){

     //fd = open(MODEM_CONTRL_PATH, O_RDWR|O_EXCL);

       //   if(fd < 0)          {
 

 //    LOGD("Can't open usi1 fd=0x%x", fd);              
 
  // }
   if(ioctl(fd,BP_IOCTL_GET_IMEI, (unsigned long)IMEI_value) < 0)             

    {                
      LOGD("mt6229 ioctl failed\n");
    }
     for(i=0;i<16;i++){                    
     LOGD("bd_addr%x = %x",i,IMEI_value[i]);
     close(fd);
   }
}
#endif
// modem ���ڳ�ʼ������ʱӦ���Ͳ������� modem's RF/SIM ״̬��ָ��
    modemEarlyInit();

    /* assume radio is off on error */
    /*
        cmy: ��RF��ʱ(+CFUN=1)����ѯSIM����״̬(��Ч��SIM������������)
     */
    //if(!modem_cmp(0x12d1,0x1003,NULL)){
     //LOGE("=======E220 AT+CFUN enale========");
     //at_send_command("AT+CFUN=1",NULL);
   
    //}
    //setRadioState(RADIO_STATE_OFF);
    if (isRadioOn() > 0) {
        setRadioState (RADIO_STATE_ON);
    }

// cmy: ��ʼ�趨��������״̬Ϊ: ������
    s_restricted_state=RIL_RESTRICTED_STATE_NONE;
    RIL_onUnsolicitedResponse(RIL_UNSOL_RESTRICTED_STATE_CHANGED,
                              &s_restricted_state,
                              sizeof(int) );

    //int tech = RADIO_TECH_HSPA;
    //RIL_onUnsolicitedResponse(RIL_UNSOL_VOICE_RADIO_TECH_CHANGED,&tech, sizeof(tech));

    LOGD("[%s]: === END ===", __FUNCTION__);
}

static void waitForClose()
{
    pthread_mutex_lock(&s_state_mutex);

    while (s_closed == 0) {
        pthread_cond_wait(&s_state_cond, &s_state_mutex);
    }

    pthread_mutex_unlock(&s_state_mutex);
}

static int getRegistrationState(char* line, int with_n, int* state, int* rat)
{
    int err;
    int skip;
    int lac;
    int cid;
    *state = -1;
    *rat = -1;
	char *p;
	int commas=0;
    /* +CGREG: <stat>, <lac>, <cid>, <networkType> */
	//         1,D9000, 583114F,12
    /* or */
    /* +CGREG: <n>, <stat>, <lac>, <cid>, <networkType> */

	/* +CGREG:  <n>,<state>,<lac>,<cid>*/
	// +CGREG:   2, 1 , D9000, 583114F
	// +CGREG:   1, "A5F4", "01EFD83F", 6
	for (p = line ; *p != '\0' ;p++) {		
		if (*p == ',') commas++;		
		}	

    err = at_tok_start(&line);
    if (err < 0) goto end;
    if (with_n)
    {
        err = at_tok_nextint(&line, &skip);
        if (err < 0) goto end;
    }
	LOGD("comms is: %d\n", commas);

	switch(commas){

		case 0: /* +CGREG: <stat> */ 
			 err = at_tok_nexthexint(&line, state);
			 if (err < 0) goto end;
        case 1: /* +CGREG: <n>, <stat> */		
			 err = at_tok_nextint(&line, &skip);
             if (err < 0) goto end;
	         err = at_tok_nexthexint(&line, state);
             if (err < 0) goto end;
	        break;
        case 2:
	     //+CGREG: <stat>, <lac>, <cid>,
	       err = at_tok_nexthexint(&line, state);
	       if (err < 0) goto end;
           err = at_tok_nexthexint(&line, &lac);
           if (err < 0) goto end;
           err = at_tok_nexthexint(&line, &cid);
	       if (err < 0) goto end;
	       break;
        case 3:
	//	 +CGREG:  <n>,<state>,<lac>,<cid>*/
        //       +CGREG:   1,"A53F","0000C618",6
	//              CGREG: 1, "A5F4", "01EFD83F", 6

            if(!modem_cmp(0x1782, 0x0002, "V1.0.1-B7")
				||!modem_cmp(0x0E8D, 0x00A2, "MTK2")
				||!modem_cmp(0x2001, 0x7D01, "DMW-156")){
            err = at_tok_nexthexint(&line, state);
            if(err <0 )  goto end;
            err = at_tok_nexthexint(&line, &lac);
            if (err < 0) goto end;
            err = at_tok_nexthexint(&line, &cid);
            if (err < 0) goto end;            
            err = at_tok_nexthexint(&line, rat);
             if (err < 0) goto end;
            }
           else{
		   	 //+CGREG:   2, 1 , D9000, 583114F
	    err = at_tok_nextint(&line, &skip);
           if (err < 0) goto end;
	       err = at_tok_nexthexint(&line, state);
           if (err < 0) goto end;
           err = at_tok_nexthexint(&line, &lac);
           if (err < 0) goto end;
           err = at_tok_nexthexint(&line, &cid);
           if (err < 0) goto end;
          }
		   break;
        case 4:
 	//CGREG: <n>, <stat>, <lac>, <cid>, <networkType> */
		  err = at_tok_nextint(&line, &skip);
          if (err < 0) goto end;
	      err = at_tok_nexthexint(&line, state);
          if (err < 0) goto end;
          err = at_tok_nexthexint(&line, &lac);
          if (err < 0) goto end;
          err = at_tok_nexthexint(&line, &cid);
          if (err < 0) goto end;
          err = at_tok_nexthexint(&line, rat);
          if (err < 0) goto end;
		 	break;
	    default:					
		goto end; 
	}
end:
    return 0;
}

/**
 * Called by atchannel when an unsolicited line appears
 * This is called on atchannel's reader thread. AT commands may
 * not be issued here
 */
/*
    cmy: ��ԭ����˼��������״̬�����仯ʱ�Ż��ϱ� +CREG��+CGREG��^MODE ����Ϣ����Ŀǰ����
    ���ֳ��Ҳ�û����ô��(EM770/TDM330��)���ڲ��������󣬾��������ϱ�CREG��CGREG����������߼���Щ�ж�
    ֻ�е�����״̬���ϴβ�ͬʱ���ϱ�״̬
 */
/*
    cmy: �����������ͱ仯ʱ,�������ϱ� RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED
    �ϲ����յ����¼�ʱ�������η�����������:
        OPERATOR
        GPRS_REGISTRATION_STATE
        REGISTRATION_STATE
        QUERY_NETWORK_SELECTION_MODE
 */
static void onUnsolicited (const char *s, const char *sms_pdu)
{
    char *line = NULL;
    int err;

    LOGD("onUnsolicited <<<<<<<<<<<<<<<<< %s", s);

    /* Ignore unsolicited responses until we're initialized.
     * This is OK because the RIL library will poll for initial state
     */
    if (sState == RADIO_STATE_UNAVAILABLE) {
        return;
    }

	if (strStartsWith(s, "%CTZV:")
			|| strStartsWith(s,"+CTZV:")
			|| strStartsWith(s,"+CTZDST:")
			|| strStartsWith(s,"+HTCCTZV:")) {
		unsolicitedNitzTime(s);
    } else if (strStartsWith(s,"NO CARRIER")) {
        RIL_onUnsolicitedResponse (
            RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED,
            NULL, 0);
        RIL_requestTimedCallback (onDataCallListChanged, NULL, NULL); //TODO use new function
	 }else if (strStartsWith(s,"^RSSI:")) {
		unsolicitedRSSI(s);
	} else if(!isgsm && strStartsWith(s,"^HDRRSSI:"))	{
	   unsolicitedHDRRSSI(s);
    } else if( !isgsm && strStartsWith(s, "^RSSILVL:") ) {
        unsolicitedRSSILVL(s);
    } else if( !isgsm && strStartsWith(s, "^HRSSILVL:") ) {
        unsolicitedHRSSILVL(s);
    } else if (0 && strStartsWith(s,"+CREG:")){
        int newStat = atoi(s+strlen("+CREG:"));
        int new_reg_rat = -1;
        if(newStat != s_reg_stat)
        {
            LOGD("reg state: %d -> %d\n", s_reg_stat, newStat);
            if( (s_reg_stat==1||s_reg_stat==5) && !(newStat==1||newStat==5) )
            {// ��ע��״̬����ע��仯Ϊδע��ʱ����ʱ�����������Ѿ���Ч��Ӧ��ע�������²���
                s_pppd_exception = 1;
            }
            s_reg_stat = newStat;
            RIL_onUnsolicitedResponse (
                RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED,
                NULL, 0);
            RIL_requestTimedCallback (onDataCallListChanged, NULL, NULL);
        }
        // ����ģ�����������ͷ����ı�ʱ�����ϱ�CREG�������յ����ϱ�ʱ��Ӧ֪ͨ�ϲ��Ա㼰ʱ�ı���������
        else if( new_reg_rat  != s_reg_rat )
        {
            LOGD("reg RAT: %d -> %d\n", s_reg_rat, new_reg_rat);
            s_reg_rat = new_reg_rat;
            RIL_onUnsolicitedResponse (
                RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED,
                NULL, 0);
        }
    } else if (strStartsWith(s,"+CGREG:")){
        int newStat = -1;
        int new_reg_rat = -1;
        int with_n = 0;
        if (!modem_cmp(0x19D2, 0xFFEB, "0"))
            with_n = 1;
      
        getRegistrationState(s, with_n, &newStat, &new_reg_rat);
	//���������ϱ���������������ϱ���������״̬����Ӧ�öϿ�PPP����
	 /* +CGREG: <stat>, <lac>, <cid>, <networkType> */
	//           1,D9000, 583114F,12 
	/* +CGREG:  <n>,<state>,<lac>,<cid>*/
	// +CGREG:   2, 1 , D9000, 583114F
        if(newStat != s_greg_stat)
        {
            LOGD("greg: %d -> %d\n", s_greg_stat, newStat);
            if( (s_greg_stat==1||s_greg_stat==5) && !(newStat==1||newStat==5) )
            {// ��ע��״̬����ע��仯Ϊδע��ʱ����ʱ�����������Ѿ���Ч��Ӧ��ע�������²���
                s_pppd_exception = 1;
            }
            s_greg_stat = newStat;
            RIL_onUnsolicitedResponse (
                RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED,
                NULL, 0);
            RIL_requestTimedCallback (onDataCallListChanged, NULL, NULL);
        }
        // ����ģ�����������ͷ����ı�ʱ�����ϱ�CREG�������յ����ϱ�ʱ��Ӧ֪ͨ�ϲ��Ա㼰ʱ�ı���������
        else if( new_reg_rat != s_reg_rat )
        {
            LOGD("reg RAT: %d -> %d\n", s_reg_rat, new_reg_rat);
            s_reg_rat = new_reg_rat;
            RIL_onUnsolicitedResponse (
                RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED,
                NULL, 0);
        }
    } else if (strStartsWith(s,"^MODE:")){
        int newStat = atoi(s+strlen("^MODE:"));
        if(newStat != s_sys_mode)
        {
            LOGD("mode: %d -> %d\n", s_sys_mode, newStat);
            s_sys_mode = newStat;
            RIL_onUnsolicitedResponse (
                RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED,
                NULL, 0);
            RIL_requestTimedCallback (onDataCallListChanged, NULL, NULL);
        }
    } else if (strStartsWith(s, "+CGEV:")) {
        /* Really, we can ignore NW CLASS and ME CLASS events here,
         * but right now we don't since extranous
         * RIL_UNSOL_DATA_CALL_LIST_CHANGED calls are tolerated
         */
        /* can't issue AT commands here -- call on main thread */
        RIL_requestTimedCallback (onDataCallListChanged, NULL, NULL);
#ifdef WORKAROUND_FAKE_CGEV
    } else if (strStartsWith(s, "+CME ERROR: 150")) {
        RIL_requestTimedCallback (onDataCallListChanged, NULL, NULL);
#endif /* WORKAROUND_FAKE_CGEV */
//	} else if (!isgsm && strStartsWith(s, "^SIMST:")) {// CDMA�����ϱ�SIM��״̬
	} else if ( strStartsWith(s, "^SIMST:") ) {
	    // �����ϱ�SIM��״̬�ı����Ϣ
        int newStat = atoi(s+strlen("^SIMST:"));
        if(newStat==255)
            RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);
    } else if (strStartsWith(s, "+CUSD:")) {
        //unsolicitedUSSD(s);
#ifdef SUPPORT_SMS
    }else if (strStartsWith(s, "+CDS:")) {//�յ�����
		RIL_onUnsolicitedResponse (RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT,sms_pdu, strlen(sms_pdu));
    }else if (strStartsWith(s, "+CMT:")) {
        RIL_onUnsolicitedResponse (
            RIL_UNSOL_RESPONSE_NEW_SMS,
            sms_pdu, strlen(sms_pdu));
#endif
    }
}

// cmy: �ȴ�AT����ķ��س�ʱ������onATTimeout������
#if 0
static void onATTimeout()
{
    LOGI("AT command timeout\n");
}
#else
/* Called on command thread */
static void onATTimeout()
{
    LOGI("AT channel timeout; closing\n");

    onATReaderClosed();
    
    /* FIXME cause a radio reset here */
    
    // reset modem
// �ر�modem��Ȼ�󽫳�ʱ��Ϣ�����ϱ����ϲ㣬���ϲ�����Ƿ��ٴδ��豸
    LOGD("reset modem");
//    reset_modem(cur_dev_path);
//    turn_on_modem(s_cur_devpath, 0);
    LOGD("END onATTimeout");
}
#endif

static void usage(char *s)
{
#ifdef RIL_SHLIB
    fprintf(stderr, "reference-ril requires: -p <tcp port> or -d /dev/tty_device\n");
#else
    fprintf(stderr, "usage: %s [-p <tcp port>] [-d /dev/tty_device]\n", s);
    exit(-1);
#endif
}

#if 0
/*
    cmy: ��ȡģ���ͺ�
 */
static void getModelIDCallback(void* param)
{
    char* model_id = *(char**)param;
    int err;
    
//	LOGD("[%s] delay %ds before first AT cmd", __FUNCTION__, DELAY_BEFORE_FIRST_ATCMD);
//	sleep(DELAY_BEFORE_FIRST_ATCMD);

    err = at_handshake();
	LOGD("[%s]: at_handshake result=%d\n", __FUNCTION__, err);
    if(err != 0) return;

// ��һ����ģ�鷢��ָ��󣬺ܿ��ܻ��յ�������أ���Щ���ض���modem�����ϱ�
// ����߼ӵ���ʱ����ֹ��ȡmodem idʱ��ȡ���������ϱ����ַ���
//    sleep(3);
    *(char**)param = getModelID();
    
    LOGD("[%s]: model_id: %s\n", __FUNCTION__, *(char**)param);

    closeATReader();
}
#endif

#if 0
// �ɹ����� model id
// ʧ�ܷ��� NULL
char* getCurModelID(RIL_ModemInterface* modem)
{
    int fd = -1;
    char* cur_model_id = NULL;
    int ret = 0;
    const char* at_channel = modem->atchannel_index;// s_cur_atchannel;
    
    if(!delayed_before_open_at){
        LOGD("[%s]: sleep %ds befor open %s", __FUNCTION__, modem->initWaitTime, at_channel);
        sleep(modem->initWaitTime);
        delayed_before_open_at = 1;
    }

    fd = open (at_channel, O_RDWR);
    LOGD("[%s]: opened fd=%d", __FUNCTION__, fd);
    if ( fd >= 0 ) {
        /* disable echo on serial ports */
        struct termios  ios;
        tcgetattr( fd, &ios );
        ios.c_lflag = 0;  /* disable ECHO, ICANON, etc... */
        tcsetattr( fd, TCSANOW, &ios );
    }
    else
        return NULL;

    s_closed = 0;
    ret = at_open(fd, NULL);

    if (ret < 0) {
        LOGE ("AT error %d on at_open\n", ret);
        return NULL;
    }

    LOGD("[%s]: Get model ID", __FUNCTION__);
    RIL_requestTimedCallback(getModelIDCallback, &cur_model_id, &TIMEVAL_0);
    
    sleep(1);

    waitForClose();

    LOGD("[%s]: cur_model_id: %s\n", __FUNCTION__, cur_model_id);
    
    return cur_model_id;
}
#endif

static RIL_ModemInterface* s_match_modem_list[MAX_MATCH_MODEM_ATCHANNEL][MAX_SUPPORT_MODEM];

static void dump_modem(RIL_ModemInterface* modem_if)
{
    if (modem_if)
        LOGD("VID/PID=0x%04X/0x%04X, ID: %s, AT=%s, PPP=%s", modem_if->modem_info.vid,
                    modem_if->modem_info.pid, 
                    modem_if->modem_info.model_id,
                    modem_if->atchannel_index, modem_if->pppchannel_index);
    else
        LOGD("NULL MODEM");
}

static int rematchModem(const char* modemID, const char* atchannel)
{
    int i=0;
    int j=0;
    int count = 0;

    for (i=0;i<MAX_MATCH_MODEM_ATCHANNEL;i++)
    {
        if (s_match_modem_list[i][0] == NULL)
            break;
        
        if (!strcmp(atchannel, s_match_modem_list[i][0]->atchannel_index))
        {
            for (j=0; j<MAX_SUPPORT_MODEM; j++)
            {
                if (s_match_modem_list[i][j]==NULL)
                    break;
                
                if (!strcmp(modemID, s_match_modem_list[i][j]->modem_info.model_id))
                {
                    count++;
                    s_current_modem = s_match_modem_list[i][j];
                    dump_modem(s_current_modem);
                    break;
                }
            }
        }
    }

    return count;
}
 int open_at_port()
 {
	      LOGD("open %s ...", s_current_modem->atchannel_index);
	      int fd = open (s_current_modem->atchannel_index, O_RDWR);
	  
	     if ( fd >= 0 ) {
		          /* disable echo on serial ports */
		          struct termios  ios;
		          tcgetattr( fd, &ios );
		          if(!modem_cmp(0x1004, 0x61AA, NULL))
		         {
			              LOGE ("setup VL600 serial port");
			              ios.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
			              ios.c_oflag &= ~(OPOST);
			              ios.c_cflag &= ~(CSIZE | PARENB);
			              ios.c_cflag |= CS8;
			              ios.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
			              ios.c_cc[VMIN] = 1;
			              ios.c_cc[VTIME] = 0;
			              cfsetospeed(&ios, B115200);
			             cfsetispeed(&ios, B115200);
			              tcsetattr( fd, TCSAFLUSH, &ios );
			         }
			        else
					         {
					             ios.c_lflag = 0;
						            /* disable ECHO, ICANON, etc... */
						            //xxh:3G Mutil lib bug
						                         ios.c_cc[VMIN]=1;
						             tcsetattr( fd, TCSANOW, &ios );
				                         RLOGE("E:/dev/ttyUSB* open sucess.........");
						        }
						     }
						 
						     return fd;
						 }
static int checkAtChannel()
{
    int fd = -1;
    int ret = 0;
    int i = 0;
    int err = 0;
    
    fd = open_at_port();
    if (fd < 0) {
        LOGE ("open failed");
        return -1;
    }

    s_closed = 0;
    ret = at_open(fd, NULL);
    if (ret < 0) 
    {
        LOGE ("AT error %d on at_open\n", ret);
        close(fd);
        return -2;
    }

// ����AT����
    err = at_handshake();

    close(fd);
    at_close();
    s_closed = 1;

    return (err==0)?0:-3;
}

static void add_to_match_modem_list(RIL_ModemInterface* modem_if)
{
    int i=0;
    int j=0;

    RIL_ModemInterface** match_modem_at = NULL;

    for (i=0;i<MAX_MATCH_MODEM_ATCHANNEL;i++)
    {
        match_modem_at = s_match_modem_list[i];
        if (match_modem_at[0]!=NULL)
        {
            if (!strcmp(modem_if->atchannel_index,
                        match_modem_at[0]->atchannel_index)) {
                // Add to list
                while (match_modem_at[++j]!=NULL)
                    ;
                match_modem_at[j] = modem_if;
                break;
            }
        } else {
            match_modem_at[0] = modem_if;
            break;
        }
    }
}

static int findModem(int vid, int pid, const char* modemID, RIL_ModemInterface** modem)
{
    int match_count=0;
    int index = 0;
    int i=0;
    int j=0;

    for (i=0;i<MAX_MATCH_MODEM_ATCHANNEL;i++)
    {
        for (j=0;j<MAX_SUPPORT_MODEM;j++)
            s_match_modem_list[i][j] = NULL;
    }

// �õ�vid/pidƥ���ģ���б�
    while(s_support_modem_lists[index].modem_info.model_id != NULL)
    {
        if(s_support_modem_lists[index].modem_info.vid == vid
            && s_support_modem_lists[index].modem_info.pid == pid)
        {
            if(modemID == NULL || 
                !strcmp(s_support_modem_lists[index].modem_info.model_id, modemID))
            {
//                dump_modem(s_support_modem_lists+index);
                add_to_match_modem_list(s_support_modem_lists+index);
                match_count++;
            }
        }
        ++index;
    }
    
    *modem = s_match_modem_list[0][0];
    return match_count;
}

/*
    �ɹ�����RIL_ModemInterface�����򷵻�NULL
 */
static RIL_ModemInterface* matchModem(int vid, int pid)
{
    RIL_ModemInterface* match_modem = NULL;
    int match_count=0;
    
    match_count = findModem(vid, pid, NULL, &match_modem);
    
    LOGD("[%s]: match model count=%d", __FUNCTION__, match_count);
    
    if(match_count <= 0)
        return NULL;

    if(match_count > 1)
    {
        sMmodemRematch = 1;
        LOGD("Need rematch modem later!");
    }
    else if(!is_support_modem_type(match_modem->radioType))
		match_modem = NULL;
	
    return match_modem;
}

/*-------------------------------------------------------*/

/** ���� check cpu ���͵��ļ��ڵ�·���ִ�. */ 
#define FILE_FOR_CPU_CHECKING "/proc/addr_kmsg"

/** ���� check cpu �� ioctl ��ʶ��. */
#define IOCTL_CHECK_CPU_TYPE            0xffffffff

/**  
 * rk CPU ���͵ĳ�����ʶ.
 * �������, û�� ������ public �� ͷ�ļ���, ������ͬ rk28_dsp.c ��һ��. 
 */ 
#define TYPE_RK2816                                     2816
#define TYPE_RK2818                                     2818

/*
    �ɹ����� 0
 */
int cpu_checking()
{
    int fd = -1;

    unsigned int cpu_type = -1;
    int result = 0;

    if ( 0 > (fd = open(FILE_FOR_CPU_CHECKING, O_RDONLY) ) )
    {
//        LOGD("Failed to open '%s', error is '%s'.", FILE_FOR_CPU_CHECKING, strerror(errno) );
        goto EXIT;
    }

    if ( 0 > (result = ioctl(fd, IOCTL_CHECK_CPU_TYPE, &cpu_type) ) )       // ...... ... unsigned int*
    {
//        LOGD("Failed to perform ioctl, error is '%s'.", strerror(errno) );
        goto EXIT;
    }

//    LOGD("Current cpu type is : %d", cpu_type);

    if( cpu_type == TYPE_RK2818 )
        return 0;

    return -2;

EXIT:
    close(fd);

    return -1;
}

int get_tty_id(const char* tty_path, int *vid, int* pid, int isACM)
{
    char linkto[260]="";
    int len=0;
    int prev_dir_deep = 0;

    len = readlink(tty_path, linkto, 260);
    if(len < 0) return -1;

	char* plink = linkto;
	while( plink[0] == '.' && plink[1] == '.' && plink[2] == '/' )
	{
		plink += 3;
        prev_dir_deep++;
	}
    plink-=5;
    memcpy(plink, "/sys/", 5);

	ALOGD("device path: %s", plink);

    int i;
    char* pend = plink;
    for(i=isACM?3:4; i>0; i--)
    {
        pend = strrchr(plink, '/');
        *pend = '\0';
    }
    
    RLOGD("USB device path: %s", plink);

    FILE* fp = NULL;
    char buf[5] = "";
    strcat(plink, "/idVendor");
    RLOGD("TTY Device Vendor path: %s", plink);
    fp = fopen(plink, "r");
    if(fp == NULL)
        return -2;
    if(fread(buf, 1, 4, fp) != 4)
    {
        fclose(fp);
        return -2;
    }
    fclose(fp);
    *vid = atox(buf, 16);

    plink[strlen(plink)-9] = '\0';
    strcat(plink, "/idProduct");
    RLOGD("TTY Device Product path: %s", plink);
    fp = fopen(plink, "r");
    if(fp == NULL)
        return -3;
    if(fread(buf, 1, 4, fp) != 4)
    {
        fclose(fp);
        return -3;
    }
    fclose(fp);
    *pid = atox(buf, 16);

    return 0;
}
#if 0
int open_at_port()
{
    LOGD("open %s ...", s_current_modem->atchannel_index);
    int fd = open (s_current_modem->atchannel_index, O_RDWR);

    if ( fd >= 0 ) {
        /* disable echo on serial ports */
        struct termios  ios;
        tcgetattr( fd, &ios );
        if(!modem_cmp(0x1004, 0x61AA, NULL))
        {
            LOGE ("setup VL600 serial port");
            ios.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
            ios.c_oflag &= ~(OPOST);
            ios.c_cflag &= ~(CSIZE | PARENB);
            ios.c_cflag |= CS8;
            ios.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
            ios.c_cc[VMIN] = 1;
            ios.c_cc[VTIME] = 0;
            cfsetospeed(&ios, B115200);
            cfsetispeed(&ios, B115200);
            tcsetattr( fd, TCSAFLUSH, &ios );
        }
        else
        {
            ios.c_lflag = 0;
            /* disable ECHO, ICANON, etc... */
            //xxh:3G Mutil lib bug
			ios.c_cc[VMIN]=1;
            tcsetattr( fd, TCSANOW, &ios );
			RLOGE("E:/dev/ttyUSB* open sucess.........");
        }
    }

    return fd;
}
#endif
static void *mainLoop(void *param)
{
    int fd;
    int ret;
    
    AT_DUMP("== ", "entering mainLoop()", -1 );
    at_set_on_reader_closed(onATReaderClosed);
    at_set_on_timeout(onATTimeout);
    
    // Load operator table (for CDMA)
    loadOperatorTable(OPERATOR_TABLE_PATH);

    for (;;) 
    {
        LOGI("Initial var!");
        fd = -1;
        s_current_modem = NULL;
        IMSI[0] = 0;
		tryCount = 0;
        // ��signalStrength��գ���ֹmodem��ʱʹ��signalStrength��������ֵ
        signalStrength[0] = -1;
        signalStrength[1] = -1;
        signalStrength_hdr[0] = -1;
        signalStrength_hdr[1] = -1;
        s_reg_stat=0;
        s_reg_rat=-1;
        s_greg_stat=0;
        s_sys_mode=-1;
        sMmodemRematch = 0;
        s_pppd_exception = 0;
        s_first_dialup = 1;
		nosimcard =0;
		sim_pin=0;
        pthread_cond_signal(&s_waitcond);

#ifdef ENABLE_STAY_AWAKE
        LOGD("[%s]: release_wake_lock: %s\n", __FUNCTION__, WAKE_LOCK_ID);
        release_wake_lock(WAKE_LOCK_ID);
#endif

        while  (fd < 0)
        {
            int vid=-1;
            int pid=-1;
            const char* dev_path = NULL;
            LOGD("Wait device...");
            sleep(3);
            /*
                �ȴ��豸����
             */
            if( access("/sys/class/tty/ttyUSB0", 0) == 0 )
            {
                
               LOGD("Found a device, get id");
                if( 0 != get_tty_id("/sys/class/tty/ttyUSB0", &vid, &pid, 0) )
                {
                    LOGD("Get device id failed!");
                    continue;
                }
            }
			#if SUPPORT_MU509&&SUPPORT_MT6229
               if( access("/sys/class/tty/ttyUSB244", 0) == 0 )
            {
               
                RLOGD("Found a inner 3G modem device, get id");
                if( 0 != get_tty_id("/sys/class/tty/ttyUSB244", &vid, &pid, 0) )
                {
                    LOGD("Get inner device id failed!");
                    continue;
                }
            }
			#endif
            else if( access("/sys/class/tty/ttyACM0", 0) == 0 )
            {
               
                RLOGD("Found a device, get id");
                if( 0 != get_tty_id("/sys/class/tty/ttyACM0", &vid, &pid, 1) )
                {
                    LOGD("Get device id failed!");
                    continue;
                }
            }

            if(vid==-1 || pid==-1)
            {
                // û���ҵ��豸
#if (!MULTI_RIL_LIB)
                continue;
#else
               exit(0);
             

#endif
            }
            LOGD("tty Device id is: %04X/%04X", vid, pid);

            // ����VID/PID��ƥ��ģ���ͺ�
            LOGD("Searching modem table...");
            s_current_modem = matchModem(vid, pid);

            if( s_current_modem == NULL )
            {
                ALOGE("E: Not support modem!!!!");
                continue;
            }

            RLOGD("AT[%s] PPP[%s]", s_current_modem->atchannel_index,
                                        s_current_modem->pppchannel_index);

            RLOGD("[%s]: sleep %ds befor open %s", __FUNCTION__, s_current_modem->initWaitTime, s_current_modem->atchannel_index);
    //        sleep(s_current_modem->initWaitTime);
            int wait_count = 10*s_current_modem->initWaitTime;
            while(wait_count>0)
           {
              if( access(s_current_modem->atchannel_index, 0) != 0 )
                {
                   // LOGE("E: modem disconnected!!!!");
#if (!MULTI_RIL_LIB)
                    continue;
#else
                    exit(0);
#endif
               }
                sleepMsec(100);
                --wait_count;
            }
            if (s_match_modem_list[1][0] != NULL)
            {// �����ͬvid/pid��3gʹ�ò�ͬ��AT�ڣ����ڴ��жϵ�ǰģ��ʹ���ĸ�AT��
                int at_index = 0;
               RLOGD("Check AT Channel...");
                while (s_match_modem_list[at_index][0])
                {
                    s_current_modem = s_match_modem_list[at_index][0];
                    if(0==checkAtChannel())
                        break;
                    ++at_index;
                }
            }
            
            fd = open_at_port();
            if (fd < 0) {
 #if (!MULTI_RIL_LIB)
                RLOGE ("opening AT interface. retrying...");
                /* never returns */
				sleep(2);
#else
                LOGE ("open at port failed.");
               exit(1);
#endif
            }
        }


        RLOGI("Device %s has been opened, fd=%d", s_current_modem->atchannel_index, fd);
        s_closed = 0;
        ret = at_open(fd, onUnsolicited);
        if (ret < 0) 
        {
           RLOGE ("AT error %d on at_open\n", ret);
            return 0;
        }
        if( pthread_create(&s_tid_checkTtyState, NULL, checkTtyStateLoop, NULL) )
	{
	      LOGD("s_tid_pppdState:thread create failed!\n");
	}
        RIL_requestTimedCallback(initializeCallback, NULL, &TIMEVAL_0);
        // Give initializeCallback a chance to dispatched, since
        // we don't presently have a cancellation mechanism
        sleep(1);
        waitForClose();
#if (!MULTI_RIL_LIB)
        RLOGI("Re-opening after close");
#else
        exit(1);
#endif
    }
} 

#if 1//#ifdef RIL_SHLIB

pthread_t s_tid_mainloop;

const RIL_RadioFunctions *RIL_Init(const struct RIL_Env *env, int argc, char **argv)
{
    int ret;
    pthread_attr_t attr;

    s_rilenv = env;

    LOGD("Enter RIL_Init");

    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&s_tid_mainloop, &attr, mainLoop, NULL);

    return &s_callbacks;
}
#else /* RIL_SHLIB */
int main (int argc, char **argv)
{
    int ret;
    int fd = -1;
    int opt;

    while ( -1 != (opt = getopt(argc, argv, "p:d:"))) {
        switch (opt) {
            case 'p':
                s_port = atoi(optarg);
                if (s_port == 0) {
                    usage(argv[0]);
                }
                LOGI("Opening loopback port %d\n", s_port);
            break;

            case 'd':
                s_device_path = optarg;
                LOGI("Opening tty device %s\n", s_device_path);
            break;

            case 's':
                s_device_path   = optarg;
                s_device_socket = 1;
                LOGI("Opening socket %s\n", s_device_path);
            break;

            default:
                usage(argv[0]);
        }
    }

    if (s_port < 0 && s_device_path == NULL) {
        usage(argv[0]);
    }

    RIL_register(&s_callbacks);

    mainLoop(NULL);

    return 0;
}

#endif /* RIL_SHLIB */
