#include "agps.h"
#include "tdgnss.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>

#define NUM_GEP_TALL         28
#define NUM_GEP				1
#define NUM_PRN				2
#define NUM_WN				3
#define NUM_H				4
#define NUM_IODE			5
#define NUM_CRS				6
#define NUM_N				7
#define NUM_M0				8
#define NUM_CUC				9
#define NUM_E				10
#define NUM_CUS				11
#define NUM_A				12
#define NUM_TOE				13
#define NUM_CIC				14
#define NUM_OMEGA0			15
#define NUM_CIS				16
#define NUM_I0				17
#define NUM_CRC				18
#define NUM_OMEGA			19
#define NUM_OMEGAD			20
#define NUM_IDOT			21
#define NUM_IODC			22
#define NUM_AF0				23
#define NUM_AF1				24
#define NUM_AF2				25
#define NUM_TOC				26
#define NUM_TGD				27
#define NUM_URAI			28

//RMI 
#define NUM_RMI_TALL        8
#define NUM_RMI				1
#define NUM_TIMEPRI			2
#define NUM_TIME			3
//#define NUM_PS				4
//#define NUM_S				5
//#define NUM_PE				6
//#define NUM_E				7




unsigned char isbig=0;
#define big2lt_4(x) (((x&0xff)<<24)|((x&0xff00)<<8)|((x&0xff0000)>>8)|((x&0xff000000)>>24))  
#define big2lt_2(x) (((x&0xff)<<8)|((x&0xff00)>>8)) 
char t_buf[500];
unsigned char sdbpbuf[73];
struct {
	unsigned char prn;
	unsigned char h;
	unsigned char urai;
	unsigned char fit;
	unsigned char tgd;
	unsigned short int iodc;
	unsigned short int toc;
	unsigned char af2;
	unsigned short int af1;
	unsigned int af0;
	unsigned char iode;
	unsigned short int crs;
	unsigned short int n;
	unsigned int m0;
	unsigned short int cuc;
	unsigned int e;
	unsigned short int cus;
	unsigned int a;
	unsigned short int toe;
	unsigned short int cic;
	unsigned int omega0;
	unsigned int omega;
	unsigned short int cis;
	unsigned int i0;
	unsigned short int crc;
	unsigned short int w;
	unsigned int omegadot;
	unsigned short int idot;
	
}cep_item;

struct {
	unsigned char s;
	unsigned short int year;
	unsigned char mon;
	unsigned char day;
	unsigned char h;
	unsigned char min;
	unsigned char sec;
	unsigned int msec;
	unsigned int timepric;
}cep_time;


//char cep_str[]="$CCGEP,01,76e,00,00,f44a,2bac,4ef7559e,f5ed,02dd52f4,177d,a10d3be8,5d2a,ffd4,93904923,000a,27505cc8,148c,13797189,ffaade,0202,028,00ce12,000b,00,5d2a,0b,0*12";
//char rmi_str[]="$CCRMI,015020.000,110106,6128.199830,S,02348.461695,E,0*7c";
static unsigned int hex2dec(char *hex);
static int c2i(char ch);


unsigned short int  FletCher16(const unsigned char* pbuf, unsigned short int len)
{
  unsigned short int cs1,cs2;
  cs1= cs2= 0;
  while(len --)
  {
    cs1 += *pbuf++;
    cs2 += cs1;
  }
  return (cs2 << 8) | (cs1 & 0xFF);
}


char  *check_gps_cep(char *s,int size)
{
	char *p;
	p = strstr(s,"$CCGEP");
	if((!p)||(p>(s+size)))
		return 0;
	return p;
}

static int check_item(char *src)
{
	
//	char sbody[]= "Presetptz,nPreset1=hello,nPreset2=ttttt,end*12";
	
	char delim[] = ",";
	char *ptoken = NULL,*temp;
	int cnt=0;
	memset(t_buf,0,sizeof(t_buf));
	memcpy(t_buf,src,strlen(src));
	ptoken = strtok(t_buf,delim);
	while(NULL!=ptoken)
	{
		cnt++;
	 	//printf("\n[%d]:%s",cnt,ptoken);
		temp = ptoken;
	  	ptoken = strtok(NULL,delim);
	}
	//printf("\n %d end:%s\n",cnt,temp);
	if((cnt == NUM_GEP_TALL)&&(strtok(temp,"*")))
	{	
		//printf("\nok\n");
		return 0;
	}else{
		//printf("\nfail!\n");
		return -1;
	}	
	
	#if 0
	char *p = s,*result;
	result = strtok(s,",");
	int i=0;
	result = strtok(s,",");
	for(i=0;i<10;i++){
		printf("\nresult[%d] :%s\n",i,result);
		result = strtok(result,",");
		//result = strtok(NULL,",");
	}
	return 0;
	#endif
}

static char *get_cep_item(char *src,int nitem)
{
	char delim[] = ",";
	char *ptoken = NULL;
	int cnt=0;
	memset(t_buf,0,sizeof(t_buf));
	memcpy(t_buf,src,strlen(src));
	ptoken = strtok(t_buf,delim);
	while(NULL!=ptoken)
	{
		cnt++;
		if(cnt == nitem){
			//printf("\nget:%s\n",ptoken);
			if(cnt==NUM_URAI){
				return strtok(ptoken,"*");
			}
			return ptoken;
		}
		if(cnt>NUM_GEP_TALL)
			return NULL;
	 	//printf("\n[%d]:%s",cnt,ptoken);
		//temp = ptoken;
	  	ptoken = strtok(NULL,delim);
	}
	return NULL;
}
static int sdbp_pack_header(unsigned char *buf)
{	
	buf[0]=0x23;buf[1]=0x3E;buf[2]=0x04;
	buf[3]=0x21;buf[4]=0x3F;buf[5]=0x00;
	return 6;
}

static int sdbp_pack_prn(char *s,unsigned char *buf,int pos)
{	
	//char *s;
	cep_item.prn = (unsigned char)atoi(get_cep_item(s,NUM_PRN));
	//printf("prn:%x\n",cep_item.prn);
	memcpy(buf+pos,&cep_item.prn,sizeof(cep_item.prn));
	return(pos+sizeof(cep_item.prn));
	//sdbpbuf[6]=hex2dec(s);
}

static int sdbp_pack_svhealth(char *s,unsigned char *buf,int pos)
{	
	//char *s;
	cep_item.h = hex2dec(get_cep_item(s,NUM_H));
	//printf("h:%x\n",cep_item.h);
	memcpy(buf+pos,&cep_item.h,sizeof(cep_item.h));
	return(pos+sizeof(cep_item.h));
}
static int sdbp_pack_uraindex(char *s,unsigned char *buf,int pos)
{	
	cep_item.urai= hex2dec(get_cep_item(s,NUM_URAI));
	//printf("urai:%x\n",cep_item.urai);
	memcpy(buf+pos,&cep_item.urai,sizeof(cep_item.urai));
	return(pos+sizeof(cep_item.urai));
}

static int sdbp_pack_fitinterval(char *s,unsigned char *buf,int pos)
{	
    (void)s;
	cep_item.fit=0x0;
	memcpy(buf+pos,&cep_item.fit,sizeof(cep_item.fit));
	return(pos+sizeof(cep_item.fit));
}
static int sdbp_pack_tgd(char *s,unsigned char *buf,int pos)
{	
	cep_item.tgd = hex2dec(get_cep_item(s,NUM_TGD));
	//printf("tgd:%x\n",cep_item.tgd);
	memcpy(buf+pos,&cep_item.tgd,sizeof(cep_item.tgd));
	return(pos+sizeof(cep_item.tgd));
}
static int sdbp_pack_iodc(char *s,unsigned char *buf,int pos)
{	
	cep_item.iodc=hex2dec(get_cep_item(s,NUM_IODC));
	//cep_item.iodc = big2lt_2(cep_item.iodc);
	//printf("iodc:%x\n",cep_item.iodc);
	memcpy(buf+pos,&cep_item.iodc,sizeof(cep_item.iodc));
	return(pos+sizeof(cep_item.iodc));
}

static  int sdbp_pack_toc(char *s,unsigned char *buf,int pos)
{
	cep_item.toc = hex2dec(get_cep_item(s,NUM_TOC));
	//cep_item.toc = big2lt_2(cep_item.toc);
	//printf("toc:%x\n",cep_item.toc);
	memcpy(buf+pos,&cep_item.toc,sizeof(cep_item.toc));
	return(pos+sizeof(cep_item.toc));
}

static  int sdbp_pack_af2(char *s,unsigned char *buf,int pos)
{	
	cep_item.af2= hex2dec(get_cep_item(s,NUM_AF2));
	//printf("af2:%x\n",cep_item.af2);
	memcpy(buf+pos,&cep_item.af2,sizeof(cep_item.af2));
	return(pos+sizeof(cep_item.af2));
}

static  int sdbp_pack_af1(char *s,unsigned char *buf,int pos)
{	
	cep_item.af1= hex2dec(get_cep_item(s,NUM_AF1));
	//cep_item.af1 = big2lt_2(cep_item.af1);
	//printf("af1:%x\n",cep_item.af1);
	memcpy(buf+pos,&cep_item.af1,sizeof(cep_item.af1));
	return(pos+sizeof(cep_item.af1));
}
static  int sdbp_pack_af0(char *s,unsigned char *buf,int pos)
{	
	cep_item.af0= (unsigned int)hex2dec(get_cep_item(s,NUM_AF0));
	//cep_item.af0 = big2lt_4(cep_item.af0);
	//printf("af0:%x\n",cep_item.af0);
	
	memcpy(buf+pos,(char*)&cep_item.af0,sizeof(cep_item.af0));
	//printf("\ncep_item.af0=0x%x size=%d\n",cep_item.af0,sizeof(cep_item.af0));
	
	return(pos+sizeof(cep_item.af0));
}
static  int sdbp_pack_iode(char *s,unsigned char *buf,int pos)
{	
	cep_item.iode = hex2dec(get_cep_item(s,NUM_IODE));
	//printf("iode:%x\n",cep_item.iode);
	memcpy(buf+pos,&cep_item.iode,sizeof(cep_item.iode));
	return(pos+sizeof(cep_item.iode));
}

static  int sdbp_pack_crs(char *s,unsigned char *buf,int pos)
{
	cep_item.crs = hex2dec(get_cep_item(s,NUM_CRS));
	//cep_item.crs = big2lt_2(cep_item.crs);
	//printf("crs:%x\n",cep_item.crs);
	memcpy(buf+pos,&cep_item.crs,sizeof(cep_item.crs));
	return(pos+sizeof(cep_item.crs));
}

static  int sdbp_pack_n(char *s,unsigned char *buf,int pos)
{	
	cep_item.n = hex2dec(get_cep_item(s,NUM_N));
	//cep_item.n = big2lt_2(cep_item.n);
	//printf("n:%x\n",cep_item.n);
	memcpy(buf+pos,&cep_item.n,sizeof(cep_item.n));
	return(pos+sizeof(cep_item.n));
}

static  int sdbp_pack_m0(char *s,unsigned char *buf,int pos)
{	
	cep_item.m0= hex2dec(get_cep_item(s,NUM_M0));
	//cep_item.m0 = big2lt_4(cep_item.m0);
	//printf("m0:%x\n",cep_item.m0);
	memcpy(buf+pos,&cep_item.m0,sizeof(cep_item.m0));
	return(pos+sizeof(cep_item.m0));
}
static  int sdbp_pack_cuc(char *s,unsigned char *buf,int pos)
{	
	cep_item.cuc= hex2dec(get_cep_item(s,NUM_CUC));
	//cep_item.cuc = big2lt_2(cep_item.cuc);
	//printf("cuc:%x\n",cep_item.cuc);
	memcpy(buf+pos,&cep_item.cuc,sizeof(cep_item.cuc));
	return(pos+sizeof(cep_item.cuc));
}
static  int sdbp_pack_e(char *s,unsigned char *buf,int pos)
{	
	cep_item.e = hex2dec(get_cep_item(s,NUM_E));
	//cep_item.e = big2lt_4(cep_item.e);
	//printf("e:%x\n",cep_item.e);
	memcpy(buf+pos,&cep_item.e,sizeof(cep_item.e));
	return(pos+sizeof(cep_item.e));
}

static  int sdbp_pack_cus(char *s,unsigned char *buf,int pos)
{
	cep_item.cus = hex2dec(get_cep_item(s,NUM_CUS));
	//cep_item.cus = big2lt_2(cep_item.cus);
	//printf("cus:%x\n",cep_item.cus);
	memcpy(buf+pos,&cep_item.cus,sizeof(cep_item.cus));
	return(pos+sizeof(cep_item.cus));
}

static  int sdbp_pack_a(char *s,unsigned char *buf,int pos)
{	
	cep_item.a= hex2dec(get_cep_item(s,NUM_A));
	//cep_item.a = big2lt_4(cep_item.a);
//	printf("a:%x\n",cep_item.a);
	memcpy(buf+pos,&cep_item.a,sizeof(cep_item.a));
	return(pos+sizeof(cep_item.a));
}
static  int sdbp_pack_toe(char *s,unsigned char *buf,int pos)
{	
	cep_item.toe= hex2dec(get_cep_item(s,NUM_TOE));
	//cep_item.toe = big2lt_2(cep_item.toe);
//	printf("toe:%x\n",cep_item.toe);
	memcpy(buf+pos,&cep_item.toe,sizeof(cep_item.toe));
	return(pos+sizeof(cep_item.toe));
}
static  int sdbp_pack_cic(char *s,unsigned char *buf,int pos)
{	
	cep_item.cic = hex2dec(get_cep_item(s,NUM_CIC));
	//cep_item.cic = big2lt_2(cep_item.cic);
//	printf("cic:%x\n",cep_item.cic);
	memcpy(buf+pos,&cep_item.cic,sizeof(cep_item.cic));
	return(pos+sizeof(cep_item.cic));
}

static  int sdbp_pack_omega0(char *s,unsigned char *buf,int pos)
{	
	cep_item.omega0 = hex2dec(get_cep_item(s,NUM_OMEGA0));
	//cep_item.omega0 = big2lt_4(cep_item.omega0);
//	printf("omega0:%x\n",cep_item.omega0);
	memcpy(buf+pos,&cep_item.omega0,sizeof(cep_item.omega0));
	return(pos+sizeof(cep_item.omega0));
}

static  int sdbp_pack_cis(char *s,unsigned char *buf,int pos)
{
	cep_item.cis = hex2dec(get_cep_item(s,NUM_CIS));
	//cep_item.cis = big2lt_2(cep_item.cis);
//	printf("cis:%x\n",cep_item.cis);
	memcpy(buf+pos,&cep_item.cis,sizeof(cep_item.cis));
	return(pos+sizeof(cep_item.cis));
}


static  int sdbp_pack_i0(char *s,unsigned char *buf,int pos)
{
	cep_item.i0 = hex2dec(get_cep_item(s,NUM_I0));
	//cep_item.i0 = big2lt_4(cep_item.i0);
//	printf("i0:%x\n",cep_item.i0);
	memcpy(buf+pos,&cep_item.i0,sizeof(cep_item.i0));
	return(pos+sizeof(cep_item.i0));
}

static  int sdbp_pack_crc(char *s,unsigned char *buf,int pos)
{	
	cep_item.crc= hex2dec(get_cep_item(s,NUM_CRC));
	//cep_item.crc = big2lt_2(cep_item.crc);
//	printf("crc:%x\n",cep_item.crc);
	memcpy(buf+pos,&cep_item.crc,sizeof(cep_item.crc));
	return(pos+sizeof(cep_item.crc));
}

static  int sdbp_pack_omega(char *s,unsigned char *buf,int pos)
{	
	cep_item.omega = hex2dec(get_cep_item(s,NUM_OMEGA));
	//cep_item.omega = big2lt_4(cep_item.omega);
//	printf("omega:%x\n",cep_item.omega);
	memcpy(buf+pos,&cep_item.omega,sizeof(cep_item.omega));
	return(pos+sizeof(cep_item.omega));
	//cep_item.n = hex2dec(get_cep_item(s,NUM_N));
}

static  int sdbp_pack_omegadot(char *s,unsigned char *buf,int pos)
{	
	cep_item.omegadot = hex2dec(get_cep_item(s,NUM_OMEGAD));
	//cep_item.omegadot = big2lt_4(cep_item.omegadot);
//	printf("omegadot:%x\n",cep_item.omegadot);
	memcpy(buf+pos,&cep_item.omegadot,sizeof(cep_item.omegadot));
	return(pos+sizeof(cep_item.omegadot));
}

static  int sdbp_pack_idot(char *s,unsigned char *buf,int pos)
{	
	cep_item.idot = hex2dec(get_cep_item(s,NUM_IDOT));
	//cep_item.idot = big2lt_2(cep_item.idot);
//	printf("idot:%x\n",cep_item.idot);
	memcpy(buf+pos,&cep_item.idot,sizeof(cep_item.idot));
	return(pos+sizeof(cep_item.idot));
}
/*
static int islittle(void)
{
	short int x;	
	char x1,x2;
	x = 0x1122;
	x1 = ((char *)&x)[0];	//low addr
	x2 = ((char *)&x)[1];	//hi addr
//	printf("x1=%x\n",x1);
//	printf("x2=%x\n",x2);
	return 0;
}*/
static int c2i(char ch)
{
	if(isdigit(ch))  
    	return ch - 48;
        if( ch < 'A' || (ch > 'F' && ch < 'a') || ch > 'z' )
        	return -1;
        if(isalpha(ch))  
        return isupper(ch) ? ch - 55 : ch - 87;
        return -1;  
} 
static unsigned int hex2dec(char *hex)  
{  
	int len;  
    int num = 0;  
    int temp;  
    int bits;  
    int i;        
    len = strlen(hex);  
    for (i=0, temp=0; i<len; i++, temp=0)  
    {  
    	temp = c2i( *(hex + i) );  
        bits = (len - i - 1) * 4;  
        temp = temp << bits;   
        num = num | temp;  
     }
     return num;  
}
#if 0
int main(int argc, char *argv[])  
{  
		memset(sdbpbuf,0,sizeof(sdbpbuf));
		
		char ch[10] = {0};
		jugelittlendian();
        strcpy(ch,"fef7559e");
		int a=hex2dec(ch);
		printf("hex:%x\n",a);
		a=big2lt_4(a);
        printf("hex:%x\n",a);
        return 0;  
}
#endif





int pack_sdbp_agps(char *s_cep,unsigned char *packbuf)
{
	char *s=s_cep;
	unsigned short int chek_sum;
	int pos;
	if(NULL == check_gps_cep(s,256)){
		return -1;
		//D("cep not rule_0!");
	}	
	if(check_item(s)==-1){
		//D("cep not rule!");
		return -1;
	}
	memset(sdbpbuf,0,73);
	pos = sdbp_pack_header(sdbpbuf);
	pos = sdbp_pack_prn(s,sdbpbuf,pos);
 	pos = sdbp_pack_svhealth(s,sdbpbuf,pos);
 	pos = sdbp_pack_uraindex(s,sdbpbuf,pos);
	pos = sdbp_pack_fitinterval(s,sdbpbuf,pos);
 	pos = sdbp_pack_tgd(s,sdbpbuf,pos);
 	pos = sdbp_pack_iodc(s,sdbpbuf,pos);
 	pos = sdbp_pack_toc(s,sdbpbuf,pos);
 	pos = sdbp_pack_af2(s,sdbpbuf,pos);
 	pos = sdbp_pack_af1(s,sdbpbuf,pos);
 	pos = sdbp_pack_af0(s,sdbpbuf,pos);
	pos = sdbp_pack_iode(s,sdbpbuf,pos);
	pos = sdbp_pack_crs(s,sdbpbuf,pos);
	pos = sdbp_pack_n(s,sdbpbuf,pos);
	pos = sdbp_pack_m0(s,sdbpbuf,pos);
 	pos = sdbp_pack_cuc(s,sdbpbuf,pos);
 	pos = sdbp_pack_e(s,sdbpbuf,pos);
 	pos = sdbp_pack_cus(s,sdbpbuf,pos);
	pos = sdbp_pack_a(s,sdbpbuf,pos);
 	pos = sdbp_pack_toe(s,sdbpbuf,pos);
  	pos = sdbp_pack_cic(s,sdbpbuf,pos);
 	pos = sdbp_pack_omega0(s,sdbpbuf,pos);
  	pos = sdbp_pack_cis(s,sdbpbuf,pos);
	pos = sdbp_pack_i0(s,sdbpbuf,pos);
 	pos = sdbp_pack_crc(s,sdbpbuf,pos);
 	pos = sdbp_pack_omega(s,sdbpbuf,pos);
 	pos = sdbp_pack_omegadot(s,sdbpbuf,pos);
 	pos = sdbp_pack_idot(s,sdbpbuf,pos);
	chek_sum = FletCher16(&sdbpbuf[2],67);
	memcpy(&sdbpbuf[69],&chek_sum,sizeof(chek_sum));
	sdbpbuf[71]=0x0d;
	sdbpbuf[72]=0x0a;
	memcpy(packbuf,sdbpbuf,73);
	return 0;
}

char  *check_gps_rmi(char *s,int size)
{
	char *p;
	p = strstr(s,"$CCRMI");
	if((!p)||(p>(s+size)))
		return 0;
	return p;
}





static int check_rmi_item(char *src)
{
	
	char delim[] = ",";
	char *ptoken = NULL,*temp;
	int cnt=0;
	memset(t_buf,0,sizeof(t_buf));
	memcpy(t_buf,src,strlen(src));
	ptoken = strtok(t_buf,delim);
	while(NULL!=ptoken)
	{
		cnt++;
	 	//printf("\n[%d]:%s",cnt,ptoken);
		temp = ptoken;
	  	ptoken = strtok(NULL,delim);
	}
	//printf("\n %d end:%s\n",cnt,temp);
	if((cnt == NUM_RMI_TALL)&&(strtok(temp,"*")))
	{	
		//printf("\nok\n");
		return 0;
	}else{
		//printf("\nfail!\n");
		return -1;
	}
}
static int get_time(char *s)
{
	char *p_tp;
	//p_t = get_cep_item(s,NUM_TIME);
	char temp[3]={0};
	p_tp = get_cep_item(s,NUM_TIMEPRI);
	//printf("p_tp:%d\n",strlen(p_tp));
	if(strlen(p_tp)!=10){
		//printf("rmi not rule\n");
		return -1;
	}	
	memcpy(temp,p_tp,2);
	cep_time.h = atoi(temp);
	//printf("src_h:%s c:%d\n",temp,cep_time.h);
	memset(temp,0,sizeof(temp));
	memcpy(temp,&p_tp[2],2);
	cep_time.min= atoi(temp);
	//printf("src_min:%s c:%d\n",temp,cep_time.min);
	memset(temp,0,sizeof(temp));
	memcpy(temp,&p_tp[4],2);
	cep_time.sec = atoi(temp);
	//printf("src_sec:%s c:%d\n",temp,cep_time.sec);

	p_tp = get_cep_item(s,NUM_TIME);
	if(strlen(p_tp)!=6){
		printf("rmi not rule\n");
		return -1;
	}
	memset(temp,0,sizeof(temp));
	memcpy(temp,p_tp,2);
	cep_time.day = atoi(temp);
	//printf("src_day:%s c:%d\n",temp,cep_time.day);
	memset(temp,0,sizeof(temp));
	memcpy(temp,&p_tp[2],2);
	cep_time.mon = atoi(temp);
	//printf("src_mon:%s c:%d\n",temp,cep_time.mon);
	memset(temp,0,sizeof(temp));
	memcpy(temp,&p_tp[4],2);
	cep_time.year = atoi(temp)+2000;
	//printf("src_year:%s c:%d\n",temp,cep_time.year);
	cep_time.s = 3;
	//cep_time.timepric = 
	return 0;
}


//unsigned char pack_time[24];
int pack_sdbp_time(char *s_rmi,unsigned char *packbuf)
{
	char *s=s_rmi;
	unsigned short int chek_sum;

	if(NULL == check_gps_rmi(s,256)){
		return -1;
		//printf("cep not rule_0!");
	}	
	if(check_rmi_item(s)==-1){
		//printf("cep not rule!");
		return -1;
	}
	if(get_time(s)==-1)
		return -1;
	//memset(packbuf,0,sizeof(packbuf));
	packbuf[0]=0x23;packbuf[1]=0x3E;packbuf[2]=0x04;packbuf[3]=0x02;
	packbuf[4]=0x10;packbuf[5]=0x0;
	packbuf[6]=cep_time.s;
	memcpy(&packbuf[7],&cep_time.year,2);
	packbuf[9]=cep_time.mon;
	packbuf[10]=cep_time.day;
	packbuf[11]=cep_time.h;
	packbuf[12]=cep_time.min;
	packbuf[13]=cep_time.sec;
	packbuf[14]=0;
	packbuf[15]=0;
	packbuf[16]=0;
	packbuf[17]=0;
	packbuf[18]=0xFF;
	packbuf[19]=0x93;
	packbuf[20]=0x35;
	packbuf[21]=0x77;
	
	chek_sum = FletCher16(&packbuf[2],20);
	memcpy(&packbuf[22],&chek_sum,2);
	//printf("pack\n");
	//for(i=0;i<24;i++)
	//	printf("%02X ",packbuf[i]);
	//printf("\n");
	return 0;
}

void  pack_sdbp_time_local(unsigned char *packbuf)
{
	
	time_t timep;
	struct tm *p;
	struct timeval tv;
	struct timezone tz;

	unsigned short int chek_sum;
	gettimeofday (&tv , &tz);
	time(&timep); 
	p=gmtime(&timep);
	cep_time.s =4;
	cep_time.year=1900+p->tm_year;
	cep_time.mon = 1+p->tm_mon;
	cep_time.day = p->tm_mday;
	cep_time.h = p->tm_hour;
	cep_time.min = p->tm_min;
	cep_time.sec =  p->tm_sec;
	cep_time.msec = tv.tv_usec*1000;
	cep_time.timepric = (uint32_t)3*1000000000;

	packbuf[0]=0x23;packbuf[1]=0x3E;packbuf[2]=0x04;packbuf[3]=0x02;
	packbuf[4]=0x10;packbuf[5]=0x0;
	packbuf[6]=cep_time.s;
	memcpy(&packbuf[7],&cep_time.year,2);
	packbuf[9]=cep_time.mon;
	packbuf[10]=cep_time.day;
	packbuf[11]=cep_time.h;
	packbuf[12]=cep_time.min;
	packbuf[13]=cep_time.sec;
	memcpy(&packbuf[14],&cep_time.msec,4);
	//packbuf[14]=0;
	//packbuf[15]=0;
	//packbuf[16]=0;
	//packbuf[17]=0;
	memcpy(&packbuf[18],&cep_time.timepric,4);
	//packbuf[18]=0xFF;
	//packbuf[19]=0x93;
	//packbuf[20]=0x35;
	//packbuf[21]=0x77;
	D("time:%d %d %d %d %d %d %u 0X%X\n",cep_time.year,cep_time.mon,cep_time.day,cep_time.h,cep_time.min,cep_time.sec,cep_time.msec,cep_time.timepric);
	chek_sum = FletCher16(&packbuf[2],20);
	memcpy(&packbuf[22],&chek_sum,2);
	return;
}

void  pack_sdbp_location(double lat, double lon, float acc, unsigned char *packbuf)
{
	unsigned short int chek_sum;

	packbuf[0]=0x23;packbuf[1]=0x3E;packbuf[2]=0x04;packbuf[3]=0x01;
	packbuf[4]=0x10;packbuf[5]=0x00;
	packbuf[6]=(int)(lon*10000000)>>0;
	packbuf[7]=(int)(lon*10000000)>>8;
	packbuf[8]=(int)(lon*10000000)>>16;
	packbuf[9]=(int)(lon*10000000)>>24;
	packbuf[10]=(int)(lat*10000000)>>0;
	packbuf[11]=(int)(lat*10000000)>>8;
	packbuf[12]=(int)(lat*10000000)>>16;
	packbuf[13]=(int)(lat*10000000)>>24;
	packbuf[14]=0;
	packbuf[15]=0;
	packbuf[16]=0;
	packbuf[17]=0;
	packbuf[18]=(unsigned int)(acc*100)>>0;
	packbuf[19]=(unsigned int)(acc*100)>>8;
	packbuf[20]=(unsigned int)(acc*100)>>16;
	packbuf[21]=(unsigned int)(acc*100)>>24;
	chek_sum = FletCher16(&packbuf[2],20);
	memcpy(&packbuf[22],&chek_sum,2);
	return;
}


#if 0
void main(void)
{
	char time_buf[256];
	pack_sdbp_time(rmi_str,time_buf);
	//pack_sdbp_time(rmi_str,pack_time)
}
#endif

