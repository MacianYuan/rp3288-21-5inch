#ifndef VPU_API_PRIVATE_CMD_H_
#define VPU_API_PRIVATE_CMD_H_


/* note: do not conlict with VPU_API_CMD */
typedef enum VPU_API_PRIVATE_CMD {
    VPU_API_PRIVATE_CMD_NONE        = 0x0,
    VPU_API_PRIVATE_HEVC_NEED_PARSE = 0x1000,

} VPU_API_PRIVATE_CMD;

#endif
