#include <stdio.h>
#include "operator_table.h"
#include <string.h>
#define LOG_TAG "RIL"
#include <utils/Log.h>
#include <stdlib.h>
OperatorInfo** gOperatorTable = NULL;//[200];
SpnInfo** spnTable=NULL;
int gOperatorCounts = 0;
int gSpnCounts = 0;

/* 有效行: 去除了行前空格，去除了行末的回车换行，不以'#'开关，不为空行
   成功则返回 用户需要的数据。
   失败返回 null
 */
char* getValidLine(char* buf, int len, FILE* fp)
{
    char* ret = NULL;

    char line[1000]={0};
    char* pt = line;
    while(1)
    {
        if( (ret=fgets(line, 1000, fp)) == NULL ) break;

        pt = line;

        // 去除行前空格
        while(pt[0] == ' ') ++pt;

        // 去除尾部的 \r\n
        int line_len = strlen(pt);
        while(pt[0] != 0)
        {
            if( (pt[line_len-1] != '\n') && (pt[line_len-1] != '\r') )
                break;
            pt[line_len-1] = '\0';
            --line_len;
        }

//        LOGD("line: %s", pt);
        if( pt[0] == '#' ) continue; // 注释行
        if( pt[0] == '\0' ) continue; // 空行

        // 取得一个有效行
        int copylen = len>strlen(pt)?strlen(pt):len;
        memcpy( buf, pt, copylen );
        buf[copylen] = '\0';

        ret = buf;

        break;
    }

    return ret;
}

int calcValidLines(FILE* fp)
{
    char line[64];
    int count = 0;
    fseek(fp, 0, SEEK_SET);
    while(1){
        if( getValidLine(line, 63, fp) == NULL ) break;
        ++count;
//        LOGD("count=%d", count);
    }
    fseek(fp, 0, SEEK_SET);
    return count;
}

void clearOperatorTable()
{
    while(gOperatorCounts--)
    {
        free(gOperatorTable[gOperatorCounts]);
        gOperatorTable[gOperatorCounts] = NULL;
    }
    free(gOperatorTable);
    gOperatorTable = NULL;
    gOperatorCounts = 0;
}

OperatorInfo* getOperator(char* numeric)
{
//    LOGD("== enter getOperator");
    int i=0;
//    LOGD("gOperatorTable=%p", gOperatorTable);
    if( gOperatorTable == NULL ) return NULL;
    for(i=0; i<gOperatorCounts; i++)
    {
//        LOGD("gOperatorTable[i]=%p", gOperatorTable[i]);
        if( gOperatorTable[i]==NULL ) continue;
        
//        LOGD("%d: [%s] [%s] [%s]\n", i, gOperatorTable[i]->numeric, gOperatorTable[i]->salphanumeric, gOperatorTable[i]->lalphanumeric);
        if( !strcmp(gOperatorTable[i]->numeric,numeric) )
            return gOperatorTable[i];
    }
    return NULL;
}

/*    成功:  operator table 大小
      失败:  0
 */
int loadOperatorTable(const char* path)
{
    FILE* fp = NULL;
    int i=0;

    clearOperatorTable();
    
    if( (fp = fopen(path, "r")) == NULL )
        return 0;

    gOperatorCounts = calcValidLines(fp)/3;

    gOperatorTable = (OperatorInfo**)malloc(gOperatorCounts * sizeof(OperatorInfo*));

    for(i=0; i<gOperatorCounts; i++)
    {
        OperatorInfo* operator = (OperatorInfo*)malloc(sizeof(OperatorInfo));
        if( getValidLine(operator->numeric, 5, fp) == NULL ) break;
        if( getValidLine(operator->salphanumeric, 31, fp) == NULL ) break;
        if( getValidLine(operator->lalphanumeric, 63, fp) == NULL ) break;
        
        //LOGD("%d: %s   %s   %s", i, operator->numeric, operator->salphanumeric, operator->lalphanumeric);
        gOperatorTable[i] = operator;
    }

    return gOperatorCounts;
}
SpnInfo* getSpn(char * numeric)
{
	  ALOGD("== enter getOperator");
		int i=0;
	//	  LOGD("gOperatorTable=%p", gOperatorTable);
		if( spnTable == NULL ) return NULL;
		for(i=0; i<gSpnCounts; i++)
		{
	//		  LOGD("gOperatorTable[i]=%p", gOperatorTable[i]);
			if( spnTable[i]==NULL ) continue;
			
		 // LOGD("%d: [%s] [%s]\n", i, spnTable[i]->numeric, spnTable[i]->name);
			if( !strcmp(spnTable[i]->numeric,numeric) )
				return spnTable[i];
		}
		return NULL;


}

void clearSpnTable()
{
    while(gSpnCounts--)
    {
        free(spnTable[gSpnCounts]);
        spnTable[gSpnCounts] = NULL;
    }
    free(spnTable);
    spnTable = NULL;
    gSpnCounts = 0;
}
int loadSpnTable(const char* path)
{
    FILE* fp = NULL;

   int i=0;

    clearSpnTable();
    
    if( (fp = fopen(path, "r")) == NULL )
        return 0;

    gSpnCounts = calcValidLines(fp)/2;

    spnTable = (SpnInfo**)malloc(gSpnCounts * sizeof(SpnInfo*));

    for(i=0; i<gSpnCounts; i++)
    {
        SpnInfo* spn = (SpnInfo*)malloc(sizeof(OperatorInfo));
        if( getValidLine(spn->numeric, 5, fp) == NULL ) break;
        if( getValidLine(spn->name, 31, fp) == NULL ) break;
        
       // LOGD("%d: %s   %s   %s", i, spn->numeric, spn->name);
        spnTable[i] = spn;
    }
      return gSpnCounts;
}
