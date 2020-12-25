typedef struct TOperatorItem{
    char numeric[6];        // NMSI(MCC+MNC)
    char salphanumeric[32]; // max 31 bytes
    char lalphanumeric[64]; // max 63 bytes
}OperatorInfo;

typedef struct TSpnInfo{
    char numeric[6];
	char name[64];

}SpnInfo;

OperatorInfo* getOperator(char* numeric);
int loadOperatorTable(const char* path);

SpnInfo* getSpn(char * numeric);
int loadSpn(const char* path);
