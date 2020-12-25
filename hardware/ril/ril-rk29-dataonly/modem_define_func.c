#include <telephony/ril.h>
#include "atchannel.h"
#include "at_tok.h"
#include "modem_define_func.h"

#define LOG_TAG "RIL"
#include <utils/Log.h>

/* cmy: ĳЩģ��������Ų�ͬ�ĳ�ʼ����ʽ
 */
void initModem_gsm()
{
    LOGD("[%s]: === Enter", __FUNCTION__);
    
    ATResponse *p_response = NULL;
    int err;

// �����������
	/*  ��ʾ���к��� */
	at_send_command("AT+CLIP=1", NULL);
    
	/*  Call Waiting notifications */
	at_send_command("AT+CCWA=1", NULL);

	/*  No connected line identification */
	at_send_command("AT+COLP=0", NULL);

	/*  USSD unsolicited */
	at_send_command("AT+CUSD=1", NULL);

// �����������
	/*  SMS PDU mode */
	at_send_command("AT+CMGF=0", NULL);

	/*  +CSSU unsolicited supp service notifications */
	at_send_command("AT+CSSN=0,1", NULL);

    // cmy: �����¶���֪ͨ������+CMT��+CMTI��+CDSI3 �֣��ϱ��� TE �ķ�ʽ
    at_send_command("AT+CNMI=1,2,0,0,0", NULL);

    // ѡ����ŷ���
    at_send_command("AT+CSMS=1", NULL);

// ��������
	/*  HEX character set */
	at_send_command("AT+CSCS=\"IRA\"", NULL);

	/*  Extra stuff */
//	at_send_command("AT+FCLASS=0", NULL);

// �����������
    // cmy: ����ע���״̬�����ı�ʱ���ϱ�+CREG:<stat>
	/*  Network registration events */
	err = at_send_command("AT+CREG=2", &p_response);
	/* some handsets -- in tethered mode -- don't support CREG=2 */
	if (err < 0 || p_response->success == 0)
		at_send_command("AT+CREG=1", NULL);

	at_response_free(p_response);

// cmy: ����ע���״̬�����ı�ʱ���ϱ�+CGREG:<stat>
	/*  GPRS registration events */
	err=at_send_command("AT+CGREG=2", &p_response);
//	E1750(0x1614,0x800)do not support AT+CGREG=2
     if (err < 0 || p_response->success == 0)
	at_send_command("AT+CGREG=1", NULL);
	 at_response_free(p_response);

// ���ֲ���ģ�飬�ڷ�������������ٷ�����������л���
// �ڴ��ٴ�ȡ������
	at_send_command("ATE0", NULL);
    
	LOGD("[%s]: === Level", __FUNCTION__);
}

void initModem_AD3812(void) //lintao 2010-8-4
{
	LOGD("[%s]: === Enter", __FUNCTION__);
	 
	ATResponse *p_response = NULL;
	int err;

// �����������
	/*  ��ʾ���к��� */
	at_send_command("AT+CLIP=1", NULL);

// �����������
	/*  SMS text mode */
	/* lintao: just only support text mode*/
	at_send_command("AT+CMGF=0", NULL);

	/* lintaos: set notify for new SMS(including +CMT��+CMTI��+CDSI)*/
	at_send_command("AT+CNMI=1,2,0,0,0", NULL);

    // ѡ����ŷ���
    at_send_command("AT+CSMS=1", NULL);

// �����������
    // cmy: ����ע���״̬�����ı�ʱ���ϱ�+CREG:<stat>
	/*  Network registration events */
	err = at_send_command("AT+CREG=2", &p_response);
	/* some handsets -- in tethered mode -- don't support CREG=2 */
	if (err < 0 || p_response->success == 0)
		at_send_command("AT+CREG=1", NULL);

	at_response_free(p_response);

    // cmy: ����ע���״̬�����ı�ʱ���ϱ�+CGREG:<stat>
	/*  GPRS registration events */
	at_send_command("AT+CGREG=2", NULL);

// ���ֲ���ģ�飬�ڷ�������������ٷ�����������л���
// �ڴ��ٴ�ȡ������
	at_send_command("ATE0", NULL);
    
	LOGD("[%s]: === Level", __FUNCTION__);
}

void initModem_cdma()
{
    LOGD("[%s]: === Enter", __FUNCTION__);
    
    ATResponse *p_response = NULL;
    int err;

	/*  ��ʾ���к��� */
	//at_send_command("AT+CLIP=1", NULL);
    
	/*  SMS text mode */
	// cmy: just only support text mode
	//at_send_command("AT+CMGF=1", NULL);

// cmy: �����¶���֪ͨ������+CMT��+CMTI��+CDSI3 �֣��ϱ��� TE �ķ�ʽ
    //at_send_command("AT+CNMI=1,2,0,0,0", NULL);

// ���ֲ���ģ�飬�ڷ�������������ٷ�����������л���
// �ڴ��ٴ�ȡ������
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
	
	// �����������
		/*	��ʾ���к��� */
		at_send_command("AT+CLIP=1", NULL);
		
		/*	Call Waiting notifications */
		at_send_command("AT+CCWA=1", NULL);
	
		/*	No connected line identification */
		at_send_command("AT+COLP=0", NULL);
	
		/*	USSD unsolicited */
		at_send_command("AT+CUSD=1", NULL);
	
	// �����������
		/*	SMS PDU mode */
		at_send_command("AT+CMGF=0", NULL);
	
		/*	+CSSU unsolicited supp service notifications */
		at_send_command("AT+CSSN=0,1", NULL);
	
		// cmy: �����¶���֪ͨ������+CMT��+CMTI��+CDSI3 �֣��ϱ��� TE �ķ�ʽ
		at_send_command("AT+CNMI=1,2,0,0,0", NULL);
	
		// ѡ����ŷ���
		at_send_command("AT+CSMS=1", NULL);
	
	// ��������
		/*	HEX character set */
		at_send_command("AT+CSCS=\"IRA\"", NULL);
	
		/*	Extra stuff */
//		at_send_command("AT+FCLASS=0", NULL);
	
	// �����������
		at_send_command("AT+CREG=1", NULL);
	
		/*	GPRS registration events */
		at_send_command("AT+CGREG=1", NULL);
		//at_send_command("AT+CGATT=1",NULL);
		at_send_command("ATE0", NULL);
		
		LOGD("[%s]: === Level", __FUNCTION__);


}

