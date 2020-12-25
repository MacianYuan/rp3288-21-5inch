#ifndef ANDROID_HARDWARE_COMMONTYPE_H
#define ANDROID_HARDWARE_COMMONTYPE_H
/*****************************************************************************/
/**
 *          Picmergestatus_t
 *
 * @brief   picture merge status
 *
 *****************************************************************************/
typedef enum Picmergestatus_e
{
    PIC_ALLOC_BUFF  = 0,                 
    PIC_ALLOC_RETURN = 1,
    PIC_EVEN_SAVE = 2,
    PIC_ODD_SAVE = 3   
} Picmergestatus_t;


//目前只有CameraAdapter为frame provider，display及event类消费完frame后，可通过该类
//将buffer返回给CameraAdapter,CameraAdapter实现该接口。

//描述帧信息，如width，height，bufaddr，fmt，便于帧消费类收到帧后做后续处理。
//包括zoom的信息
typedef struct FramInfo
{
    ulong_t phy_addr;
    ulong_t vir_addr;
    ulong_t original_vir_addr;
    int frame_width;
    int frame_height;
    ulong_t frame_index;
    int frame_fmt;
    int original_frame_fmt;
    int zoom_value;
    ulong_t used_flag;
    int frame_size;
    void* res;
    bool vir_addr_valid;
    bool is_even_field;
    Picmergestatus_t merge_status;
}FramInfo_s;

typedef int (*func_displayCBForIsp)(void* frameinfo,void* cookie);

#endif
