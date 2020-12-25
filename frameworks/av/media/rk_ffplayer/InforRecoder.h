#ifndef _FFMPEG_INFORMATION_RECODER_
#define _FFMPEG_INFORMATION_RECODER_

extern "C"
{
#include <stdio.h>
}

#include <utils/threads.h>

/*
* this file is defined by hh@rock-chips.com for record playing status for shanghai CMCC
*/


// ���ٿ�ʼ
#define LAG_START  701

// ���ٽ���
#define LAG_STOP 702

// ��ȡ������Ƭ
#define GET_DATA_FAIL 703

// ����������
#define PLAYER_ERROR 100



#define MAX_URL_LENGTH 4096


namespace android
{
class InforRecoder
{
public:
    InforRecoder();
    ~InforRecoder();

    int getOpenFileName();

   /*
    * �������LOG�ļ���Ŀ¼
    * path:�������ļ���Ŀ¼���Ϻ��ƶ���Ƶ����Ҫ��Ŀ¼�̶�Ϊ /tmp/playLog/
    */
    int createDirectory(char* path);

    /*
    * ���ݵ�ǰϵͳʱ��(������ʱ��)����Ϊ�ļ����֣�����:20140626195600.txt
    * time: ���ڱ����ļ���
    * length: �洢������󳤶�
    */
    void getSystemTime(char* time,int length,int needSecond);

    /*
    * �����ļ�������getFileName��ȡ�����ļ��������ļ�
    * �Ϻ��ƶ���Ƶ����Ҫ��ÿ��15���Ӵ���һ�����ļ���������15�����г��ֵĲ��ſ�����Ϣд�����ļ���
    */
    void openFile();

    /*
    * �ر��ļ�
    */
    void closeFile();

    /*
    *���������쳣״̬����Ϣд���ļ�
    * url: ��ǰ���ŵ�ƬԴurl
    * playtime: ��ǰ�Ĳ���ʱ��
    * status: �����룬�쳣��
    * des: ���������������
    */
    void writeFile(char* url,int playtime,int status,char* des);

    void realWrite(char* url,int playTime,int status,char* des);

    /*
    * ɾ���ض�Ŀ¼(/tmp/playLog/)�µ��ļ���ֻ�������һ��15���ӵ��ļ�
    */
    void deleteDirectory();

   /*
     *  delete all files in this directory
     */
   void deleteAllFileInDirectory();

    /*
    * �̻߳ص�����
    */
    static void* threadCallBack(void * recoder);
private:
    // �򿪵��ļ����
    FILE*      mLogFile;

    // �򿪵��ļ�·��������ɾ��û��д��LOG���ļ�
    char*      mFilePath;

    // ɾ���ļ���ʱ��
    int64_t    mDeleteFileTime;

    // �����ļ���ʱ��
    int64_t    mCreateFileTime;

    // �߳̿��Ʊ��������ڿ����̵߳Ľ����˳�
    int        mThreadExit;

    // ���ڱ�ʾ�߳��Ƿ��д���
    int        mThreadStatus;

    // �߳̾��
    pthread_t  mThread;

    // �߳�ͬ����. д�ļ����ļ��Ĵ�����ɾ������ͬһ���߳��С�
    // ʹ��Mutex����ֹͬһʱ�䣬����߳�ͬʱ�����ļ���
    Mutex      mLock;

	  // ���ڱ���д���URL
	  char* mURL;
};
}

#endif
