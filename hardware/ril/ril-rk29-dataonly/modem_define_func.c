#include <telephony/ril.h>
#include "atchannel.h"
#include "at_tok.h"
#include "modem_define_func.h"

#define LOG_TAG "RIL"
#include <utils/Log.h>

/* cmy: 某些模块可能有着不同的初始化方式
 */
void initModem_gsm()
{
    LOGD("[%s]: === Enter", __FUNCTION__);
    
    ATResponse *p_response = NULL;
    int err;

// 呼叫相关设置
	/*  显示主叫号码 */
	at_send_command("AT+CLIP=1", NULL);
    
	/*  Call Waiting notifications */
	at_send_command("AT+CCWA=1", NULL);

	/*  No connected line identification */
	at_send_command("AT+COLP=0", NULL);

	/*  USSD unsolicited */
	at_send_command("AT+CUSD=1", NULL);

// 短信相关设置
	/*  SMS PDU mode */
	at_send_command("AT+CMGF=0", NULL);

	/*  +CSSU unsolicited supp service notifications */
	at_send_command("AT+CSSN=0,1", NULL);

    // cmy: 设置新短信通知（包括+CMT、+CMTI、+CDSI3 种）上报给 TE 的方式
    at_send_command("AT+CNMI=1,2,0,0,0", NULL);

    // 选择短信服务
    at_send_command("AT+CSMS=1", NULL);

// 其它设置
	/*  HEX character set */
	at_send_command("AT+CSCS=\"IRA\"", NULL);

	/*  Extra stuff */
//	at_send_command("AT+FCLASS=0", NULL);

// 网络相关设置
    // cmy: 网络注册的状态发生改变时，上报+CREG:<stat>
	/*  Network registration events */
	err = at_send_command("AT+CREG=2", &p_response);
	/* some handsets -- in tethered mode -- don't support CREG=2 */
	if (err < 0 || p_response->success == 0)
		at_send_command("AT+CREG=1", NULL);

	at_response_free(p_response);

// cmy: 网络注册的状态发生改变时，上报+CGREG:<stat>
	/*  GPRS registration events */
	err=at_send_command("AT+CGREG=2", &p_response);
//	E1750(0x1614,0x800)do not support AT+CGREG=2
     if (err < 0 || p_response->success == 0)
	at_send_command("AT+CGREG=1", NULL);
	 at_response_free(p_response);

// 发现部分模块，在发完上述命令后，再发其它命令会有回显
// 在此再次取消回显
	at_send_command("ATE0", NULL);
    
	LOGD("[%s]: === Level", __FUNCTION__);
}

void initModem_AD3812(void) //lintao 2010-8-4
{
	LOGD("[%s]: === Enter", __FUNCTION__);
	 
	ATResponse *p_response = NULL;
	int err;

// 呼叫相关设置
	/*  显示主叫号码 */
	at_send_command("AT+CLIP=1", NULL);

// 短信相关设置
	/*  SMS text mode */
	/* lintao: just only support text mode*/
	at_send_command("AT+CMGF=0", NULL);

	/* lintaos: set notify for new SMS(including +CMT、+CMTI、+CDSI)*/
	at_send_command("AT+CNMI=1,2,0,0,0", NULL);

    // 选择短信服务
    at_send_command("AT+CSMS=1", NULL);

// 网络相关设置
    // cmy: 网络注册的状态发生改变时，上报+CREG:<stat>
	/*  Network registration events */
	err = at_send_command("AT+CREG=2", &p_response);
	/* some handsets -- in tethered mode -- don't support CREG=2 */
	if (err < 0 || p_response->success == 0)
		at_send_command("AT+CREG=1", NULL);

	at_response_free(p_response);

    // cmy: 网络注册的状态发生改变时，上报+CGREG:<stat>
	/*  GPRS registration events */
	at_send_command("AT+CGREG=2", NULL);

// 发现部分模块，在发完上述命令后，再发其它命令会有回显
// 在此再次取消回显
	at_send_command("ATE0", NULL);
    
	LOGD("[%s]: === Level", __FUNCTION__);
}

void initModem_cdma()
{
    LOGD("[%s]: === Enter", __FUNCTION__);
    
    ATResponse *p_response = NULL;
    int err;

	/*  显示主叫号码 */
	//at_send_command("AT+CLIP=1", NULL);
    
	/*  SMS text mode */
	// cmy: just only support text mode
	//at_send_command("AT+CMGF=1", NULL);

// cmy: 设置新短信通知（包括+CMT、+CMTI、+CDSI3 种）上报给 TE 的方式
    //at_send_command("AT+CNMI=1,2,0,0,0", NULL);

// 发现部分模块，在发完上述命令后，再发其它命令会有回显
// 在此再次取消回显
	at_send_command("ATE0", NULL);
    
	LOGD("[%s]: === Level", __FUNCTION__);
}

void initModem_LC6341() //malei
{
    LOGD("[%s]: === Enter", __FUNCTION__);
    
    ATResponse *p_response = NULL;
    int err;
	
	at_send_command("ATV1", NULL);
	//at_send_command("AT^DCCOM=1,1,0", NULL);

	at_send_command("AT^DUSBDEG=1,1", NULL);
    at_send_command("AT^DUSBDEG=2,1",NULL);
	at_send_command("AT^DUSBDEG=3,1",NULL);
	at_send_command("AT^DUSBDEG=4,1",NULL);
	at_send_command("AT^DUSBDEG=5,1",NULL);


	at_send_command("AT+CGMR", NULL);

	at_send_command("AT^DTSER", NULL);
	
	at_send_command("AT+CMER=2,0,0,2", NULL);

	at_send_command("AT+CREG=1", NULL);
    
	at_send_command("AT+CGREG=1", NULL);

	at_send_command("AT^DCTA=1", NULL);

	at_send_command("AT^DSCI=1", NULL);

	at_send_command("AT^DCPI=1",NULL);
	
	at_send_command("AT^DDTM?",NULL);

	//at_send_command("AT^DSTM=1,0,2",NULL);
		
}
void initModem_QCOM(void)
{
	  LOGD("[%s]: === Enter", __FUNCTION__);
		
		ATResponse *p_response = NULL;
		int err;
	
	// 呼叫相关设置
		/*	显示主叫号码 */
		at_send_command("AT+CLIP=1", NULL);
		
		/*	Call Waiting notifications */
		at_send_command("AT+CCWA=1", NULL);
	
		/*	No connected line identification */
		at_send_command("AT+COLP=0", NULL);
	
		/*	USSD unsolicited */
		at_send_command("AT+CUSD=1", NULL);
	
	// 短信相关设置
		/*	SMS PDU mode */
		at_send_command("AT+CMGF=0", NULL);
	
		/*	+CSSU unsolicited supp service notifications */
		at_send_command("AT+CSSN=0,1", NULL);
	
		// cmy: 设置新短信通知（包括+CMT、+CMTI、+CDSI3 种）上报给 TE 的方式
		at_send_command("AT+CNMI=1,2,0,0,0", NULL);
	
		// 选择短信服务
		at_send_command("AT+CSMS=1", NULL);
	
	// 其它设置
		/*	HEX character set */
		at_send_command("AT+CSCS=\"IRA\"", NULL);
	
		/*	Extra stuff */
//		at_send_command("AT+FCLASS=0", NULL);
	
	// 网络相关设置
		at_send_command("AT+CREG=1", NULL);
	
		/*	GPRS registration events */
		at_send_command("AT+CGREG=1", NULL);
		//at_send_command("AT+CGATT=1",NULL);
		at_send_command("ATE0", NULL);
		
		LOGD("[%s]: === Level", __FUNCTION__);


}

