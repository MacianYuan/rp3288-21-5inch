
/*
    当　RIL_RELEASE　被定义时，所有处于调试中的modem将不会编译进来
    在编译RIL库并发布时，应当定义　RIL_RELEASE
 */
#define RIL_RELEASE

/* 
    当　ENABLE_DNS_CHECK　被定义时，在拨号完成后会检查所分配到的DNS
    如果DNS有异常(例如10.11.12.13)时，将会重新进行拨号
    
    使用专网的SIM卡，它本身可能不提供访问外部网络的能力，因此拨号
   上网时不会分配DNS,　对于这种情况，应设置:
        #define ENABLE_DNS_CHECK  0
   其它正常的SIM卡都应设置:
        #define ENABLE_DNS_CHECK  1
*/
#define ENABLE_DNS_CHECK   1

#define SUPPORT_SMS         0

#define GET_USI_IMEI     0

#define SUPPORT_MU509       0

#define SUPPORT_MT6229      0

#define SUPPORT_MW100       0

#define SUPPORT_STRONGRISING 0

/*
    当 ENABLE_STAY_AWAKE 定义时，对于CDMA/EVDO网络，拨号上网后将不会进入二级睡眠
 */
#define ENABLE_STAY_AWAKE



