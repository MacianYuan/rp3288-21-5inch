#include <telephony/ril.h>
#include "atchannel.h"
#include "at_tok.h"
#include "extend_at_func.h"
#include <string.h>
#define LOG_TAG "RIL"
#include <utils/Log.h>

int networkType2RAT(int networkType)
{
    int response = RADIO_TECHNOLOGY_UNKNOWN;
    switch(networkType)
    {
    case 0:
        response = RADIO_TECHNOLOGY_GPRS;
        break;
    case 1:
        response = RADIO_TECHNOLOGY_GPRS;
        break;
    case 2:
        response = RADIO_TECHNOLOGY_UMTS;
        break;
    case 3:
        response = RADIO_TECHNOLOGY_EDGE;
        break;
    case 4:
        response = RADIO_TECHNOLOGY_HSDPA;
        break;
    case 5:
        response = RADIO_TECHNOLOGY_HSUPA;
        break;
    case 6:
        response = RADIO_TECHNOLOGY_HSPA;
        break;
    case 7:
        response = RADIO_TECHNOLOGY_LTE;
        break;
    default:
        response = RADIO_TECHNOLOGY_UNKNOWN;
        break;
    }
    return response;
}

/*
    从COPS指令中查询access technology，与 CS/PS 服务状态
    
    CS/PS 服务状态: 只有当modem没有提供相应命令时，才使用COPS命令
    access technology: 只有当CREG/CGREG没有返回该值，且modem没有提供相应命令时才使用COPS

    返回值:
        0: 命令成功
        -1: 命令失败
 */
int queryOperator(int *network, int *restricted)
{
    int err;
	char *line;
	ATResponse *p_response = NULL;
	int skip;
	char* str_skip;
	int net;

	if(restricted) *restricted = RIL_RESTRICTED_STATE_CS_ALL|RIL_RESTRICTED_STATE_PS_ALL;
	if(network) *network = RADIO_TECHNOLOGY_UNKNOWN;

    err = at_send_command_singleline("AT+COPS?", "+COPS:", &p_response);

    if (err < 0 || p_response->success == 0) {
       	at_response_free(p_response);
    	return -1;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);

    if (err < 0) goto end;

    err = at_tok_nextint(&line, &skip);
    if (err < 0) goto end;
	
   	err = at_tok_nextint(&line, &skip);
    if (err < 0) goto end;

    err = at_tok_nextstr(&line, &str_skip);
    if (err < 0) goto end;

    err = at_tok_nextint(&line, &net);/*0,1,2--> GSM ,GSM-COMPACT,UTRAN*/
	if(err < 0 ) goto end;

    if(network) *network = networkType2RAT(net);
    if(restricted) *restricted = RIL_RESTRICTED_STATE_NONE;

end:
   	at_response_free(p_response);
	return 0;
}

// 返回值   成功: 0     失败: -1
int querySystemInfo(AT_SYSTEM_INFO* sys_info)
{
    int err;
    ATResponse *p_response = NULL;
    char *line, *p;
    int commas;

    LOGD("[%s]: === ENTER ===", __FUNCTION__);

    memset(sys_info, 0, sizeof(AT_SYSTEM_INFO));

    err = at_send_command_singleline("AT^SYSINFO", "^SYSINFO:", &p_response);

    if (err < 0 || p_response->success == 0) goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    /* count number of commas */
    commas = 0;
    for (p = line ; *p != '\0' ;p++) {
        if (*p == ',') commas++;
    }
	//AT< ^SYSINFO:2,3,0,5,1,,7
	//at<         ^2,255,0,8,1,,11
	LOGD("[%s]: SYSCOMMD=%d", __FUNCTION__, commas);

    if(commas < 4) goto error;

    err = at_tok_nextint(&line, &sys_info->srv_status);
    if (err < 0) goto error;
    err = at_tok_nextint(&line, &sys_info->srv_domain);
    if (err < 0) goto error;
    err = at_tok_nextint(&line, &sys_info->roam_status);
    if (err < 0) goto error;
    err = at_tok_nextint(&line, &sys_info->sys_mode);
    if (err < 0) goto error;
    err = at_tok_nextint(&line, &sys_info->sim_state);
    if (err < 0) goto error;

    if(commas >= 6)
    {
        at_tok_nextint(&line, &sys_info->reserve);
		LOGD("[%s]: RESERVE=%d", __FUNCTION__, sys_info->reserve);
        err = at_tok_nextint(&line, &sys_info->sys_submode);
		LOGD("[%s]: subsysmode=%d", __FUNCTION__, sys_info->sys_submode);
        if (err < 0) goto error;
    }

    at_response_free(p_response);
    return 0;
    
error:
    LOGD("[%s]: err=%d", __FUNCTION__, err);
    at_response_free(p_response);
    return -1;
}

// 返回值   成功: 0     失败: -1
int checkCardStatus_zte(char* network, char* srv_domain_str)
{
    int err;
    char *line;
	ATResponse *p_response = NULL;
	char* p_value;

	LOGD("[%s]: === ENTER ===", __FUNCTION__);
	
	network[0]=0;
	srv_domain_str[0]=0;
    
    err = at_send_command_singleline("AT+ZPAS?", "+ZPAS:", &p_response);
    if (err < 0 || p_response->success == 0) goto error;
    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &p_value);
    if (err < 0) goto error;
    strcpy(network, p_value);
    err = at_tok_nextstr(&line, &p_value);
    if (err < 0) goto error;
    strcpy(srv_domain_str, p_value);

    at_response_free(p_response);
    return 0;
    
error:
	at_response_free(p_response);
	LOGE("ERROR: [%s ]failed\n", __FUNCTION__);
	return -1;
}


/***************************************************************
cmy@20100611: 获取首选网络类型系列函数 getPreferredNetworkType_xxx
     返回值  正常: 首选网络类型，其它错误: -1
***************************************************************/

// for china mobile
extern int getCurModelRadio();
extern int modem_cmp(int vid, int pid, const char* name);
int getPreferredNetworkType_cnm()
{
	int err;
	ATResponse *p_response = NULL;
	int response = 0;
	char *line;
	int mode, acqorder, roam, srvdomain;
    int skip;

    LOGD("[%s]: === ENTER ===", __FUNCTION__);

    if( !modem_cmp(0x12D1, 0x1001, "MU509")
		||!modem_cmp(0x12d1, 0x1506, "E1220")
		||!modem_cmp(0x12d1,0x1001,"E3131")
		||!modem_cmp(0x19F5, 0x9013, "UW100")
		||!modem_cmp(0x1C9E, 0x9603, "HSPA USB MODEM")
		||!modem_cmp(0x12D1, 0x140c, "E303")
		||!modem_cmp(0x12D1, 0x1506, "E357")
		||!modem_cmp(0x12D1, 0x140C, "E1552")
		||!modem_cmp(0x12D1, 0x1001, "K3806")
		||!modem_cmp(0x12d1, 0x1001, "K3770")
		||!modem_cmp(0x12d1,0x1001,"K3773")
		||!modem_cmp(0x19F5, 0x9013, "MW160")
		||!modem_cmp(0x12d1,0x140c,"E261")
		||!modem_cmp(0x12d1,0x1003,"E160E")
		||!modem_cmp(0x12d1,0x1506,"E303C")
		||!modem_cmp(0x12d1,0x1001,"E3131C")
                ||!modem_cmp(0x12d1,0x1506,"MU609")
		||(getCurModelRadio()==3))//国外3G vodafon K3770 UM100
    {
        err = at_send_command_singleline("AT^SYSCFG?", "^SYSCFG:", &p_response);
    }
    else
    {
        err = at_send_command_singleline("AT^SYSCONFIG?", "^SYSCONFIG:", &p_response);
    }
    
	if (err < 0 || p_response->success == 0) goto error;

	line = p_response->p_intermediates->line;

	err = at_tok_start(&line);

	if (err < 0) goto error;

    err = at_tok_nextint(&line, &mode);
    if(err < 0) goto error;
    err = at_tok_nextint(&line, &acqorder);
    if(err < 0) goto error;

    switch(mode)
    {
    case 2:// 自动选择
        switch(acqorder)
        {
        case 0: response = NETWORK_MODE_GLOBAL; break; // 自动选择
        case 1: response = NETWORK_MODE_GSM_UMTS; break; // GSM PREF
        case 2: default: response = NETWORK_MODE_WCDMA_PREF; break; // UTRAN PREF
        }
        break;
    case 13: response = NETWORK_MODE_GSM_ONLY; break; // GSM ONLY
    case 14:case 15: response = NETWORK_MODE_WCDMA_ONLY; break; // WCDMA/TD-SCDMA ONLY
    default: goto error; break; // ERROR
    }

    LOGD("[%s]: return network mode=%d", __FUNCTION__, response);

	at_response_free(p_response);
	
	return response;

error:
	at_response_free(p_response);
	LOGE("ERROR: [%s ]failed\n", __FUNCTION__);

	return -1;
}

/***************************************************************
cmy@20100611: 设置首选网络类型系列函数 setPreferredNetworkType_xxx
     返回值  正常: 0，网络类型错误: -1，其它错误: -2
xxh@ SYSCONFIG指令和SYSCFG指令分别对应TD-SCDMA和WCDMA     
***************************************************************/

extern int getCurModelRadio();
int setPreferredNetworkType_cnm(int rat)
{
	int err;
	ATResponse *p_response = NULL;
	char * cmd = NULL;
	int mode, acqorder;
	int roam=2, srvdomain=4;

	LOGD("[%s]: === ENTER ===", __FUNCTION__);

// 在该函数中应该调用 getPreferredNetworkType_cnm
    int cur_type = getPreferredNetworkType_cnm();// getPreferredNetworkType_zte();

    LOGD("[%s]: new preferred network type is: %d", __FUNCTION__, rat);
    LOGD("[%s]: current preferred network type is: %d", __FUNCTION__, cur_type);
    
    if(cur_type == rat) return 0;

    switch(rat)
    {
    case NETWORK_MODE_WCDMA_PREF: mode = 2; acqorder = 2; break;
    case NETWORK_MODE_GSM_ONLY: mode = 13; acqorder = 1; break;
    case NETWORK_MODE_WCDMA_ONLY: mode=(getCurModelRadio() == 3)?14:15; acqorder = 2; break;
    case NETWORK_MODE_GSM_UMTS: mode = 2; acqorder = 1; break;
    case NETWORK_MODE_GLOBAL: mode = 2; acqorder = 0; break;
    default: goto error_mode;
    }

    if(getCurModelRadio()==3){  //wcdma网络

        if( !modem_cmp(0x12D1, 0x1001, "MU509")
			||!modem_cmp(0x12d1, 0x1506, "E1220")
			||!modem_cmp(0x12d1,0x140c,"E261")
			||!modem_cmp(0x12d1,0x1001,"E3131")
			||!modem_cmp(0x12d1,0x1506,"MU609")
			)
          {
           asprintf(&cmd, "AT^SYSCFG=%d,%d, 40000000,%d,%d", mode, acqorder, roam, srvdomain);
          //asprintf(&cmd, "AT^SYSCFG=%d,%d, 3fffffff,%d,%d", mode, acqorder, roam, srvdomain);
    	   err = at_send_command(cmd, &p_response);
    	   free(cmd);
    	   if (err < 0|| p_response->success == 0) goto error;
          }
		else if(!modem_cmp(0x19F5, 0x9013, "UW100")
             || !modem_cmp(0x1C9E, 0x9603, "HSPA USB MODEM")
             || !modem_cmp(0x19F5, 0x9013, "MW160"))//内置模块think mw100
    	   {
    		asprintf(&cmd, "AT^SYSCFG=%d,%d, 3FFFFFFF ,%d,%d", mode, acqorder, roam, srvdomain);
    		err = at_send_command(cmd, &p_response);
    		free(cmd);
    		if (err < 0|| p_response->success == 0) goto error;
           }
		else if(!modem_cmp(0x12D1, 0x1506, "E357")
              || !modem_cmp(0x12D1, 0x140c, "E303")
              ||!modem_cmp(0x12D1, 0x140C, "E1552")
              ||!modem_cmp(0x12D1, 0x1001, "K3806")
              ||!modem_cmp(0x12d1, 0x1001, "K3770")
              ||!modem_cmp(0x12d1,0x1001,"K3772")
              ||!modem_cmp(0x12d1,0x1001,"K3773")
              ||!modem_cmp(0x12d1,0x1506,"E303C")
              ||!modem_cmp(0x12d1,0x1001,"E3131C")
              ||!modem_cmp(0x12d1,0x1003,"E160E"))//国外两个vodafone 3G dongle
    	   {
    		asprintf(&cmd, "AT^SYSCFG=%d,%d, 40000000 ,%d,%d", mode, acqorder, roam, srvdomain);
    		err = at_send_command(cmd, &p_response);
    		free(cmd);
    		if (err < 0|| p_response->success == 0) goto error;
           }
		else 
         LOGD("WCDMA Net work to GSM network......");
		 asprintf(&cmd, "AT^SYSCFG=%d,%d, 40000000,2,4", mode, acqorder);
	     err = at_send_command(cmd, &p_response);
		 free(cmd);
		 if (err < 0|| p_response->success == 0) goto error;
		
	}else  //td网络
     {
    	/* Need to unregister from NW before changing preferred RAT */
    	//err = at_send_command("AT+COPS=2", NULL);
    	//if (err < 0) goto error;

        asprintf(&cmd, "AT^SYSCONFIG=%d,%d,%d,%d", mode, acqorder, roam, srvdomain);
    	err = at_send_command(cmd, &p_response);
    	free(cmd);
    	if (err < 0|| p_response->success == 0) goto error;

    	/* Register on the NW again */
    	//err = at_send_command("AT+COPS=0", NULL);
    	//if (err < 0) goto error;
    }

	at_response_free(p_response);
	return 0;
error_mode:
	at_response_free(p_response);
	return -1;
error:
	at_response_free(p_response);
	return -2;
}

/***************************************************************
cmy@20100611: 获取网络类型系列函数 getNetworkType_xxx
     返回值  网络类型
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
    RADIO_TECHNOLOGY_TDSCDMA,
***************************************************************/
// 对于^SYSINFO命令，其它返回值的最后两个字段reserve/sys_submode，在中国移动中，可能有也可能没有，
// 但在中国电信的AT指令集中，该命令最后两个字段不使用
int getNetworkType_cnm()
{// 该函数可用于 China Mobile, China Telecom, China Unicom
    int network_type = RADIO_TECHNOLOGY_UNKNOWN;
    int err;
    AT_SYSTEM_INFO sys_info;

    LOGD("[%s]: === ENTER ===", __FUNCTION__);
    
    err = querySystemInfo(&sys_info);
    
    if(err < 0) goto error;

// 获取系统模式，如CDMA/WCDMA/TD-SCDMA
    switch(sys_info.sys_mode)
    {
    case 2: network_type = RADIO_TECHNOLOGY_1xRTT; break;   // CDMA
    case 3: network_type = RADIO_TECHNOLOGY_GPRS; break;    // GSM/GPRS
    case 4:  // HDR
    case 8:  // CDMA/HDR HYBRID
        network_type = RADIO_TECHNOLOGY_EVDO_A;
        break;
    case 5:  // WCDMA
    case 15: // TD-SCDMA
        network_type = RADIO_TECHNOLOGY_UMTS;
        break;
    case 0: default: network_type = RADIO_TECHNOLOGY_UNKNOWN; break; // Unknown
    }
	LOGD("[%s]: sys_info.sys_submode: %d", __FUNCTION__, sys_info.sys_submode);

// 获取系统子模式，如GPRS/HUDPA/HSPA等
    if(sys_info.sys_submode > 0)
    {// 有sys_submode ，则使用它来确定网络类型
    LOGD("[%s]: sys_info.sys_submode: %d", __FUNCTION__, sys_info.sys_submode);
        switch(sys_info.sys_submode)
        {
        case 1: case 2: network_type = RADIO_TECHNOLOGY_GPRS; break;    // GSM/GPRS
        case 3: network_type = RADIO_TECHNOLOGY_EDGE; break;            // EDGE
        case 4:  // WCDMA
        case 8:  // TD-SCDMA
            network_type = RADIO_TECHNOLOGY_UMTS;
            break;
        case 5: network_type = RADIO_TECHNOLOGY_HSDPA; break; // HSDPA
        case 6: network_type = RADIO_TECHNOLOGY_HSUPA; break; // HSUPA
        case 7: network_type = RADIO_TECHNOLOGY_HSPA; break; // HSPA
        case 9: network_type = RADIO_TECHNOLOGY_HSPAP; break;//HSPA+
        case 11:network_type = RADIO_TECHNOLOGY_HSPA;break;
        case 13:network_type = RADIO_TECHNOLOGY_EHRPD; break;//HDR HYBRID
        default: network_type = RADIO_TECHNOLOGY_UNKNOWN; break; // Unknown
        }
    }

    LOGD("[%s]: network type: %d", __FUNCTION__, network_type);
    return network_type;

error:
    LOGD("[%s]: err=%d", __FUNCTION__, err);
    return RADIO_TECHNOLOGY_UNKNOWN;
}

/*
    当前系统服务域
    返回值: 成功: >=0,   失败: -1
 */
int getRestrictedState_cnm()
{
    int err;
    int new_restricted_state;
    AT_SYSTEM_INFO sys_info;

    LOGD("[%s]: === ENTER ===", __FUNCTION__);
    
    err = querySystemInfo(&sys_info);

    if(err < 0) goto error;

//    if(sys_info.srv_status)// 服务有效
//    {
    switch(sys_info.srv_domain){
    case 1:
        new_restricted_state = RIL_RESTRICTED_STATE_PS_ALL;
        break;
    case 2:
        new_restricted_state = RIL_RESTRICTED_STATE_CS_ALL;
        break;
    case 3:
    case 255: // CDMA not support
        new_restricted_state = RIL_RESTRICTED_STATE_NONE;
        break;
    case 0: default:
        new_restricted_state = RIL_RESTRICTED_STATE_CS_ALL|RIL_RESTRICTED_STATE_PS_ALL;
        break;
    }
//    }

    return new_restricted_state;

error:
    LOGD("[%s]: err=%d", __FUNCTION__, err);
    return -1;
}


// for china telecom
int getPreferredNetworkType_cnt()
{
	int err;
	ATResponse *p_response = NULL;
	int response = 0;
	char *line;
	int mode;

	LOGD("[%s]: === ENTER ===", __FUNCTION__);

    err = at_send_command_singleline("AT^PREFMODE?", "^PREFMODE:", &p_response);
    
	if (err < 0 || p_response->success == 0) goto error;

	line = p_response->p_intermediates->line;

	err = at_tok_start(&line);

	if (err < 0) goto error;

    err = at_tok_nextint(&line, &mode);
    if(err < 0) goto error;
    switch(mode)
    {
    case 2: response = NETWORK_MODE_CDMA_NO_EVDO; break;
    case 4: response = NETWORK_MODE_EVDO_NO_CDMA; break;
    case 8: response = NETWORK_MODE_CDMA; break;
    default: goto error; break;
    }

    LOGD("[%s]: return network mode=%d", __FUNCTION__, response);

	at_response_free(p_response);
	return response;

error:
	at_response_free(p_response);
	LOGE("ERROR: [%s ]failed\n", __FUNCTION__);
	return -1;
}

int setPreferredNetworkType_cnt(int rat)
{
	int err;
	ATResponse *p_response = NULL;
	char * cmd = NULL;
	int mode;

    LOGD("[%s]: === ENTER ===", __FUNCTION__);
    
    switch(rat)
    {
    case NETWORK_MODE_CDMA_NO_EVDO: mode = 2; break;
    case NETWORK_MODE_EVDO_NO_CDMA: mode = 4; break;
    case NETWORK_MODE_CDMA: case NETWORK_MODE_GLOBAL: mode = 8; break;
    default: goto error_mode;
    }

    asprintf(&cmd, "AT^PREFMODE=%d", mode);
	err = at_send_command(cmd, &p_response);
	free(cmd);

	if (err < 0|| p_response->success == 0) goto error;

	at_response_free(p_response);
	return 0;
error_mode:
	at_response_free(p_response);
	return -1;
error:
	at_response_free(p_response);
	return -2;
}

// for ZTE
int getPreferredNetworkType_zte()
{
	int err;
	ATResponse *p_response = NULL;
	int response = 0;
	char *line;
	int mode, acqorder, roam, srvdomain;
    int skip;

    LOGD("[%s]: === ENTER ===", __FUNCTION__);
    
    err = at_send_command_singleline("AT+ZSNT?", "+ZSNT:", &p_response);
    
	if (err < 0 || p_response->success == 0) goto error;

	line = p_response->p_intermediates->line;

	err = at_tok_start(&line);

	if (err < 0) goto error;

    err = at_tok_nextint(&line, &mode);
    if(err < 0) goto error;
    err = at_tok_nextint(&line, &skip);
    if(err < 0) goto error;
    err = at_tok_nextint(&line, &acqorder);
    if(err < 0) goto error;

    switch(mode)
    {
    case 0:
        {
            switch(acqorder)
            {
            case 0: response = NETWORK_MODE_GLOBAL; break; // 自动选择
            case 1: response = NETWORK_MODE_GSM_UMTS; break; // GSM PREF
            case 2: default: response = NETWORK_MODE_WCDMA_PREF; break; // UTRAN PREF
            }
        }
        break;
    case 1:response = NETWORK_MODE_GSM_ONLY; break; // GSM ONLY
    case 2:response = NETWORK_MODE_WCDMA_ONLY; break; // WCDMA/TD-SCDMA ONLY
    default:goto error;break;  // 错误的值
    }
    
    LOGD("[%s]: return network mode=%d", __FUNCTION__, response);

	at_response_free(p_response);
	
	return response;

error:
	at_response_free(p_response);
	LOGE("ERROR: [%s ]failed\n", __FUNCTION__);

	return -1;
}

int setPreferredNetworkType_zte(int rat)
{
	int err;
	ATResponse *p_response = NULL;
	char * cmd = NULL;
	int mode, acqorder;

	LOGD("[%s]: === ENTER ===", __FUNCTION__);

    int cur_type = getPreferredNetworkType_zte();

    LOGD("[%s]: new preferred network type is: %d", __FUNCTION__, rat);
    LOGD("[%s]: current preferred network type is: %d", __FUNCTION__, cur_type);
    
    if(cur_type == rat) return 0;
    
    switch(rat)
    {
    case NETWORK_MODE_WCDMA_PREF: mode = 2; acqorder = 0; break;
    case NETWORK_MODE_GSM_ONLY: mode = 1; acqorder = 0; break;
    case NETWORK_MODE_WCDMA_ONLY: mode = 2; acqorder = 2; break;
    case NETWORK_MODE_GSM_UMTS: mode = 0; acqorder = 1; break;
    case NETWORK_MODE_GLOBAL: mode = 0; acqorder = 0; break;
    default: goto error_mode;
    }

	/* Need to unregister from NW before changing preferred RAT */
//	err = at_send_command("AT+COPS=2", NULL);
//	if (err < 0) goto error;

    asprintf(&cmd, "AT+ZSNT=%d,%d,%d", mode, 0, acqorder);
	err = at_send_command(cmd, &p_response);
	free(cmd);
	if (err < 0|| p_response->success == 0) goto error;

	/* Register on the NW again */
//	err = at_send_command("AT+COPS=0", NULL);
//	if (err < 0) goto error;

	at_response_free(p_response);
	return 0;
error_mode:
	at_response_free(p_response);
	return -1;
error:
	at_response_free(p_response);
	return -2;
}

int getNetworkType_zte()
{
    int network_type = RADIO_TECHNOLOGY_UNKNOWN;

    int err;
    char network[32];
    char srv_domain_str[32];

    LOGD("[%s]: === ENTER ===", __FUNCTION__);

    err = checkCardStatus_zte(network, srv_domain_str);
    if(err) goto error;

    if( !strcmp(network, "EDGE") )
        network_type = RADIO_TECHNOLOGY_EDGE;
    else if( !strcmp(network, "GPRS") )
        network_type = RADIO_TECHNOLOGY_GPRS;
    else if( !strcmp(network, "GSM") )
        network_type = RADIO_TECHNOLOGY_GPRS;
	else if( !strcmp(network,"3G"))
		network_type = RADIO_TECHNOLOGY_UMTS;
    else if( !strcmp(network, "HSDPA") )
        network_type = RADIO_TECHNOLOGY_HSDPA;
    else if( !strcmp(network, "HSUPA") )
        network_type = RADIO_TECHNOLOGY_HSUPA;
    else if( !strcmp(network, "HSPA") )
        network_type = RADIO_TECHNOLOGY_HSPA;
    else if( !strcmp(network, "UMTS") )
        network_type = RADIO_TECHNOLOGY_UMTS;
	else if(!strcmp(network, "HSPA+"))
		network_type=RADIO_TECHNOLOGY_HSPAP;
    else if( !strcmp(network, "LTE") )
        network_type = RADIO_TECHNOLOGY_LTE;
    else // "No Service"、"Limited Service"、other
        network_type = RADIO_TECHNOLOGY_UNKNOWN;

    LOGD("[%s]: network type: %d", __FUNCTION__, network_type);
    return network_type;

error:
    LOGD("[%s]: err=%d", __FUNCTION__, err);
    return RADIO_TECHNOLOGY_UNKNOWN;
}

int getRestrictedState_zte()
{
    int err;
    int new_restricted_state;
    char network[32];
    char srv_domain_str[32];

    LOGD("[%s]: === ENTER ===", __FUNCTION__);

    err = checkCardStatus_zte(network, srv_domain_str);
    if(err < 0) goto error;

    if( !strcmp(srv_domain_str, "CS_ONLY") )
        new_restricted_state = RIL_RESTRICTED_STATE_PS_ALL;
    else if( !strcmp(srv_domain_str, "PS_ONLY") )
        new_restricted_state = RIL_RESTRICTED_STATE_CS_ALL;
    else if( !strcmp(srv_domain_str, "CS_PS") )
        new_restricted_state = RIL_RESTRICTED_STATE_NONE;
    else
        new_restricted_state = RIL_RESTRICTED_STATE_CS_ALL|RIL_RESTRICTED_STATE_PS_ALL;

    return new_restricted_state;
    
error:
    LOGD("[%s]: err=%d", __FUNCTION__, err);
    return -1;
}


// for ZTE AD3812
int getPreferredNetworkType_zte_ad3812()
{
	int err;
	ATResponse *p_response = NULL;
	int response = 0;
	char *line;
	int mode;

    LOGD("[%s]: === ENTER ===", __FUNCTION__);

    err = at_send_command_singleline("AT+ZMDS?", "+ZMDS:", &p_response);
    
	if (err < 0 || p_response->success == 0) goto error;

	line = p_response->p_intermediates->line;

	err = at_tok_start(&line);

	if (err < 0) goto error;

    err = at_tok_nextint(&line, &mode);
    if(err < 0) goto error;

    switch(mode)
    {
    case 4:response = NETWORK_MODE_GLOBAL; break; // 自动选择
    case 13:response = NETWORK_MODE_GSM_ONLY; break; // GSM ONLY
    case 14:response = NETWORK_MODE_WCDMA_ONLY; break; // WCDMA/TD-SCDMA ONLY
    default:goto error;break;  // 错误的值
    }
    
    LOGD("[%s]: return network mode=%d", __FUNCTION__, response);

    at_response_free(p_response);

    return response;

error:
	at_response_free(p_response);
	LOGE("ERROR: [%s ]failed\n", __FUNCTION__);

	return -1;
}

//lintao
int setPreferredNetworkType_zte_ad3812(int rat)
{
	int err;
	ATResponse *p_response = NULL;
	char * cmd = NULL;
	int mode, acqorder;
	int cur_type;

	LOGD("[%s]: === ENTER ===", __FUNCTION__);

  	cur_type = getPreferredNetworkType_zte_ad3812();

	LOGD("[%s]: new preferred network type is: %d", __FUNCTION__, rat);
	LOGD("[%s]: current preferred network type is: %d", __FUNCTION__, cur_type);
    
    if(cur_type == rat) return 0;

    switch(rat)
    {
    case NETWORK_MODE_WCDMA_PREF:case NETWORK_MODE_GSM_UMTS:case NETWORK_MODE_GLOBAL:
        mode = 4; break;
    case NETWORK_MODE_GSM_ONLY: mode = 13; break;
    case NETWORK_MODE_WCDMA_ONLY: mode = 14; break;
    default: goto error_mode;
    }

    asprintf(&cmd, "AT+ZMDS=%d", mode);
	err = at_send_command(cmd, &p_response);
	free(cmd);
	if (err < 0|| p_response->success == 0) goto error;

	at_response_free(p_response);
	return 0;
error_mode:
	at_response_free(p_response);
	return -1;
error:
	at_response_free(p_response);
	return -2;
}

int getNetworkType_generic()
{
/* 通常，接入技术是由CREG/CGREG命令返回，如果那两个命令没有返回，
    则在这里可通过COPS查询
*/
    int network_type;

	LOGD("[%s]: === ENTER ===", __FUNCTION__);
	
    queryOperator(&network_type, NULL);
    
    LOGD("[%s]: network type: %d", __FUNCTION__, network_type);
    return network_type;
}

int getRestrictedState_generic()
{
    int restricted;
    LOGD("[%s]: === ENTER ===", __FUNCTION__);
    
    queryOperator(NULL, &restricted);
    
    LOGD("[%s]: restricted state: %d", __FUNCTION__, restricted);
    return restricted;
}

int getPreferredNetworkType_generic()
{// 标准指令集中并没有指令实现该功能
    LOGD("[%s]: === ENTER ===", __FUNCTION__);
    return NETWORK_MODE_WCDMA_PREF;
}

int setPreferredNetworkType_generic(int rat)
{
	int cur_type;

    LOGD("[%s]: === ENTER ===", __FUNCTION__);

  	cur_type = getPreferredNetworkType_generic();

	LOGD("[%s]: new preferred network type is: %d", __FUNCTION__, rat);
	LOGD("[%s]: current preferred network type is: %d", __FUNCTION__, cur_type);
    
    if(cur_type == rat) return 0;

// 标准指令集中并没有指令实现该功能
	return -2;
}

int getNetworkType_lc6341()
{// 该函数可用于 China Mobile, China Telecom, China Unicom
#if 1
    int network_type = RADIO_TECHNOLOGY_UNKNOWN;

    int err;
    ATResponse *p_response = NULL;
    int response[2];	
    char *line, *p;
    int commas;

    LOGD("[%s]: === ENTER ===", __FUNCTION__);

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

    at_response_free(p_response);

	if(response[0] <99)
	{
		network_type = RADIO_TECHNOLOGY_GPRS; 
	}else if(response[0]>100)
	{
		network_type = RADIO_TECHNOLOGY_UMTS;
	}

    LOGD("[%s]: network type: %d", __FUNCTION__, network_type);
    return network_type;

error:
    LOGD("[%s]: err=%d", __FUNCTION__, err);
    at_response_free(p_response);
    return RADIO_TECHNOLOGY_UNKNOWN;

#else
	return RADIO_TECHNOLOGY_UMTS;
#endif
}

int getRestrictedState_lc6341()
{
    int err;
    int new_restricted_state;

    LOGD("[%s]: === ENTER ===", __FUNCTION__);

    new_restricted_state = RIL_RESTRICTED_STATE_NONE;

    return new_restricted_state;

error:
    LOGD("[%s]: err=%d", __FUNCTION__, err);
    return -1;
}

// for ARCHOS G9
int getPreferredNetworkType_archos_g9()
{
	int err;
	ATResponse *p_response = NULL;
	int response = 0;
	char *line;
	int mode, order, roam, srvdomain;
    int skip;

    LOGD("[%s]: === ENTER ===", __FUNCTION__);
    
    err = at_send_command_singleline("AT+SYSSEL?", "+SYSSEL:", &p_response);
    
	if (err < 0 || p_response->success == 0) goto error;

	line = p_response->p_intermediates->line;

	err = at_tok_start(&line);

	if (err < 0) goto error;

    err = at_tok_nextint(&line, &skip);
    if(err < 0) goto error;
    err = at_tok_nextint(&line, &mode);
    if(err < 0) goto error;
    err = at_tok_nextint(&line, &order);
    if(err < 0) goto error;

    switch(mode)
    {
    case 0:// 自动选择
        switch(order)
        {
        case 0: response = NETWORK_MODE_GLOBAL; break; // 自动选择
        case 2: response = NETWORK_MODE_GSM_UMTS; break; // GSM PREF
        case 1: default: response = NETWORK_MODE_WCDMA_PREF; break; // UTRAN PREF
        }
        break;
    case 1: response = NETWORK_MODE_GSM_ONLY; break; // GSM ONLY
    case 2: response = NETWORK_MODE_WCDMA_ONLY; break; // WCDMA/TD-SCDMA ONLY
    default: goto error; break; // ERROR
    }

    LOGD("[%s]: return network mode=%d", __FUNCTION__, response);

	at_response_free(p_response);
	
	return response;

error:
	at_response_free(p_response);
	LOGE("ERROR: [%s ]failed\n", __FUNCTION__);

	return -1;
}

int setPreferredNetworkType_archos_g9(int rat)
{
	int err;
	ATResponse *p_response = NULL;
	char * cmd = NULL;
	int mode, acqorder;

	LOGD("[%s]: === ENTER ===", __FUNCTION__);

    int cur_type = getPreferredNetworkType_archos_g9();

    LOGD("[%s]: preferred network type %d -> %d", __FUNCTION__, cur_type, rat);
    
    if(cur_type == rat) return 0;

    switch(rat)
    {
    case NETWORK_MODE_WCDMA_PREF: mode = 0; acqorder = 1; break;
    case NETWORK_MODE_GSM_ONLY: mode = 1; acqorder = -1; break;
    case NETWORK_MODE_WCDMA_ONLY: mode=2; acqorder = -1; break;
    case NETWORK_MODE_GSM_UMTS: mode = 0; acqorder = 2; break;
    case NETWORK_MODE_GLOBAL: mode = 0; acqorder = 0; break;
    default: goto error_mode;
    }

#if 0
	/* Need to unregister from NW before changing preferred RAT */
	err = at_send_command("AT+COPS=2", NULL);
	if (err < 0) goto error;
#endif

    if(acqorder >= 0)
        asprintf(&cmd, "AT+SYSSEL=,%d,%d,", mode, acqorder);
    else
        asprintf(&cmd, "AT+SYSSEL=,%d,,", mode);
	err = at_send_command(cmd, &p_response);
	free(cmd);
	if (err < 0|| p_response->success == 0) goto error;

#if 0
	/* Register on the NW again */
	err = at_send_command("AT+COPS=0", NULL);
	if (err < 0) goto error;
#endif

	at_response_free(p_response);
	return 0;
error_mode:
	at_response_free(p_response);
	return -1;
error:
	at_response_free(p_response);
	return -2;
}



int getPreferredNetworkType_SCV_SPW9P()
{

    int err;
    ATResponse *p_response = NULL;
	int response = 0;
	char *line;
	int mode;

    LOGD("[%s]: === ENTER ===", __FUNCTION__);

    err = at_send_command_singleline("AT+ZMDS?", "+ZMDS:", &p_response);
    
	if (err < 0 || p_response->success == 0) goto error;

	line = p_response->p_intermediates->line;

	err = at_tok_start(&line);

	if (err < 0) goto error;

    err = at_tok_nextint(&line, &mode);
    if(err < 0) goto error;

    switch(mode)
    {
    case 4:response = NETWORK_MODE_GLOBAL; break; // 自动选择
    
    case 13:response = NETWORK_MODE_GSM_ONLY; break; // GSM ONLY
    
    case 14:response = NETWORK_MODE_WCDMA_ONLY; break; // WCDMA/TD-SCDMA ONLY
    
    default:goto error;break;  
    }
    
    LOGD("[%s]: return network mode=%d", __FUNCTION__, response);

    at_response_free(p_response);

    return response;

error:
	at_response_free(p_response);
	LOGE("ERROR: [%s ]failed\n", __FUNCTION__);

	return -1;




}


int setPreferredNetworkType_SCV_SPW9P(int rat)
{

int err;
	ATResponse *p_response = NULL;
	char * cmd = NULL;
	int mode, acqorder;
	int cur_type;

	LOGD("[%s]: === ENTER ===", __FUNCTION__);

  	cur_type = getPreferredNetworkType_SCV_SPW9P();

	LOGD("[%s]: new preferred network type is: %d", __FUNCTION__, rat);
	LOGD("[%s]: current preferred network type is: %d", __FUNCTION__, cur_type);
    
    if(cur_type == rat) return 0;

    switch(rat)
    {
    case NETWORK_MODE_WCDMA_PREF:case NETWORK_MODE_GSM_UMTS:case NETWORK_MODE_GLOBAL:
        mode = 4; break;
    case NETWORK_MODE_GSM_ONLY: mode = 13; break;
    case NETWORK_MODE_WCDMA_ONLY: mode = 14; break;
    default: goto error_mode;
    }

    asprintf(&cmd, "AT+ZMDS=%d", mode);
	err = at_send_command(cmd, &p_response);
	free(cmd);
	if (err < 0|| p_response->success == 0) goto error;

	at_response_free(p_response);
	return 0;
error_mode:
	at_response_free(p_response);
	return -1;
error:
	at_response_free(p_response);
	return -2;




}

int getQualmNetworkType()
{
    int err;
	ATResponse *p_response = NULL;
	int response = 0;
	char *line;
    int networkType;

    LOGD("[%s]: === ENTER ===", __FUNCTION__);
	//+NETMODE: 8,5,0,0,0,1,2,1,0,0,5
    
    err = at_send_command_singleline("AT+NETMODE?", "+NETMODE:", &p_response);
    
	if (err < 0 || p_response->success == 0) goto error;

	line = p_response->p_intermediates->line;

	err = at_tok_start(&line);

	if (err < 0) goto error;

    err = at_tok_nextint(&line, &networkType);

    switch(networkType)
    {  
    case 0:response=RADIO_TECHNOLOGY_UNKNOWN;break;
    case 1: response = RADIO_TECHNOLOGY_GPRS; break; 
    case 2: response = RADIO_TECHNOLOGY_EDGE; break;
    case 3: response = RADIO_TECHNOLOGY_UMTS; break;
	case 5: response = RADIO_TECHNOLOGY_EVDO_0; break;
	case 6: response = RADIO_TECHNOLOGY_EVDO_A; break;
    case 7: response = RADIO_TECHNOLOGY_1xRTT; break;
    case 8: response = RADIO_TECHNOLOGY_HSDPA; break;
    case 9: response = RADIO_TECHNOLOGY_HSUPA; break; 
    case 10: response = RADIO_TECHNOLOGY_HSPA; break;
			   
    default: goto error; break; // ERROR
    }

    LOGD("[%s]: return network type=%d", __FUNCTION__, response);

	at_response_free(p_response);
	
	return response;

error:
	at_response_free(p_response);
	LOGE("ERROR: [%s ]failed\n", __FUNCTION__);

	return -1;
}



