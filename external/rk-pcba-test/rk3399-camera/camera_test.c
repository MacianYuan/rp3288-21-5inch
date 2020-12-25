#include "camera_test.h"
#include "../minui_pcba/minui.h"
#include "../test_case.h"
#include "sys_core_ion.h"
#include <hardware/hardware.h>

#include "OV13850_MIPI_priv.h"
#include "OV5648_MIPI_priv.h"
#include "OV8858_MIPI_priv.h"
#include "GC2155_CIF_priv.h"
#include "GC2145_CIF_priv.h"
#include "GC0329_CIF_priv.h"

#define VIDEO_DEV_NAME		"/dev/video0"
#define PMEM_DEV_NAME		"/dev/pmem_cam"
#define DISP_DEV_NAME		"/dev/graphics/fb1"
#define ION_DEVICE		"/dev/ion"

#define CAMSYS_DEPATH_MARVIN	"/dev/camsys_marvin"

#define ION_LEN			0x1600000
#define SENSOR_MCLK		24000000

#define MRV_REG_ADDR(a)		(a/4)

/*
 * for switch ion and drm
 */
#define ION_USED 0

//hkw add;
static int camsys_fd;

static int camera_x=0,camera_y=0,camera_w=0,camera_h=0,camera_num=1;
static struct testcase_info *tc_info = NULL;

pthread_t camera_tid;

rk_camera_info_t camera_test_info;


/**add by cx start***/
int marvin_config(MrvAllRegister_t *pMrvReg, unsigned int buf)
{
    camsys_sysctrl_t sysctl;
    int err;

    sysctl.dev_mask = CAMSYS_DEVID_MARVIN;
    sysctl.ops = CamSys_ClkIn;
    sysctl.on = 1;

    err = ioctl(camsys_fd, CAMSYS_SYSCTRL, &sysctl);
    if (err<0) {
        printf("CamSys_ClkIn on failed\n");
    }

    pMrvReg->vi_ccl = 0x00;
    pMrvReg->vi_iccl = 0xFFFFFFFF;
	printf("%s %d  hcc\n",__FUNCTION__,__LINE__);

    usleep(10);
    pMrvReg->vi_ircl = (1<<7);
    usleep(10);
    pMrvReg->vi_ircl = (0<<7);
    usleep(10);
    usleep(15000);
    pMrvReg->mipi[0].mipi_ctrl = 0x00;

    // set ISP to RGB mode, enable input, output formatter and update shadow registers
    // '0: ISP_enable
    // 321:  000: raw picture
    //       001: ccir656
    //       010: ccir601
    //       011: bayer rgb
    //       100: data mode
    // 4: inform enable
    // 5: bls enable
    // 6: gamma_in enable
    // 7: awb enable
    // 8: ae enable
    // 9: cfg_upd
    // 10: gen_cfg_upd
    // 11: gamma_out enable
    //pMrvReg->isp_ctrl = 0x8c6;
    pMrvReg->isp_ctrl = 0x8c0 | (camera_test_info.Mode<<1);
    printf("isp_ctrl:0x%04x Mode: 0x%04x \n",pMrvReg->isp_ctrl,camera_test_info.Mode);
    //[5:0]: 2a-RAW8  2B-RAW10  2c-RAW12
    pMrvReg->mipi[0].mipi_img_data_sel = camera_test_info.mipi_img_data_sel;
	printf("mipi_img_data_sel: 0x%04x \n",pMrvReg->mipi[0].mipi_img_data_sel);
    //[5:0] data type selector for additional data output
    //regbase[MRV_REG_ADDR(MRV_MIPI_ADD_DATASEL1)]= 0x12;
    //regbase[MRV_REG_ADDR(MRV_MIPI_ADD_DATASEL2)]= 0x2b;
    //regbase[MRV_REG_ADDR(MRV_MIPI_ADD_DATASEL3)]= 0x2b;
    //regbase[MRV_REG_ADDR(MRV_MIPI_ADD_DATASEL4)]= 0x2b;

    //24: frame end send to output interface
    //23: checksum error
    //22: 1bit ecc error
    //21: 2bit ecc error
    //pMrvReg->mipi[0].mipi_imsc = (1<<24) | (0x07<<21);

    //[2]:enable IMSC_DATA_LOSS interrupt
    //[3]:enable IMSC_PIC_SIZE_ERR interrupt
    //regbase[MRV_REG_ADDR(MRV_ISP_IMSC)] = 0xC;
    pMrvReg->isp_imsc = (1<<1)|(1<<5);

    //[0] sample edge   0- negative  1-positive
    //[1] hsync pol     0-high       1-low
    //[2] vsync pol     0-high       1-low
    //[4:3] color components from sensor, 01-first line:GRGR,sec line BGBG   11-first line:BGBG,sec line GRGR
    //[6:5] 01-interleaved color subsampling Y0CB0-Y1Cr1
    //[8:7] 10-CbYCrY
    pMrvReg->isp_acq_prop = ((camera_test_info.Edge<<0) | (camera_test_info.HPol<<1) | (camera_test_info.VPol<<2)
    | (camera_test_info.BPat<<3) | (camera_test_info.Conv422<<5)|(camera_test_info.YCSequence<<7)); //0x13d
    printf("isp_acq_prop: 0x%08x \n",pMrvReg->isp_acq_prop);
    // set ISP input aquisition window
    //regbase[MRV_REG_ADDR(MRV_ISP_ACQ_H_OFFS)]      = 0;  //
    //regbase[MRV_REG_ADDR(MRV_ISP_ACQ_V_OFFS)]      = 0;  //
    //regbase[MRV_REG_ADDR(MRV_ISP_ACQ_H_SIZE)]      = camera_test_info.width;  //
    //regbase[MRV_REG_ADDR(MRV_ISP_ACQ_V_SIZE)]      = camera_test_info.height;  //
    pMrvReg->isp_acq_h_offs = 0 & MRV_ISP_ACQ_H_OFFS_MASK;
    pMrvReg->isp_acq_v_offs = 0 & MRV_ISP_ACQ_V_OFFS_MASK;
	if(camera_test_info.phy_type == CamSys_Phy_Cif)
		pMrvReg->isp_acq_h_size = (camera_test_info.width*2) & MRV_ISP_ACQ_H_SIZE_MASK;
	else
		pMrvReg->isp_acq_h_size = (camera_test_info.width) & MRV_ISP_ACQ_H_SIZE_MASK;

    pMrvReg->isp_acq_v_size = camera_test_info.height & MRV_ISP_ACQ_V_SIZE_MASK;
    printf("isp_acq_h_size:%d isp_acq_v_size:%d \n",pMrvReg->isp_acq_h_size,pMrvReg->isp_acq_v_size);

    // set ISP output window
    //regbase[MRV_REG_ADDR(MRV_ISP_OUT_H_OFFS)]      = 0;  //
    //regbase[MRV_REG_ADDR(MRV_ISP_OUT_V_OFFS)]      = 0;  //
    //regbase[MRV_REG_ADDR(MRV_ISP_OUT_H_SIZE)]      = camera_test_info.width;  //
    //regbase[MRV_REG_ADDR(MRV_ISP_OUT_V_SIZE)]      = camera_test_info.height;  //
    //regbase[MRV_REG_ADDR(MRV_ISP_IS_H_OFFS)]      = 0;  //
    //regbase[MRV_REG_ADDR(MRV_ISP_IS_V_OFFS)]      = 0;  //
    //regbase[MRV_REG_ADDR(MRV_ISP_IS_H_SIZE )]      = camera_test_info.width;  //
    //regbase[MRV_REG_ADDR(MRV_ISP_IS_V_SIZE )]      = camera_test_info.height;  //

    pMrvReg->isp_out_h_offs = 0 & MRV_ISP_ISP_OUT_H_OFFS_MASK;
	usleep(2);
    pMrvReg->isp_out_v_offs = 0 & MRV_ISP_ISP_OUT_V_OFFS_MASK;
    pMrvReg->isp_out_h_size = camera_test_info.width & MRV_ISP_ISP_OUT_H_SIZE_MASK;
    pMrvReg->isp_out_v_size = camera_test_info.height & MRV_ISP_ISP_OUT_V_SIZE_MASK;


    pMrvReg->isp_is_h_offs = 0 & MRV_IS_IS_H_OFFS_MASK;
	usleep(2);
    pMrvReg->isp_is_v_offs = 0 & MRV_IS_IS_V_OFFS_MASK;
	pMrvReg->isp_is_h_size = camera_test_info.width & MRV_IS_IS_H_SIZE_MASK;
	pMrvReg->isp_is_v_size = camera_test_info.height & MRV_IS_IS_V_SIZE_MASK;

    //[1:0] data path select: 01-from main resize to MI, uncompressed
    //[3:2] yc splitter channel mode: 01-main path and raw data mode
	//[9:8] select input interface : 00-parallel-interface 10-MIPI1-interface
    //pMrvReg->vi_dpcl = 0x005;
	if(camera_test_info.phy_type == CamSys_Phy_Cif)
		pMrvReg->vi_dpcl = 0x005;
	else
		pMrvReg->vi_dpcl = 0x205;

    printf("vi_dpcl:0x%04x \n",pMrvReg->vi_dpcl);

    // set ISP input number of frames to 0 (continuous)
    //regbase[MRV_REG_ADDR(MRV_ISP_ACQ_NR_FRAMES)]   = 0x0;
    pMrvReg->isp_acq_nr_frames = 0x00;
    /*
    //regbase[MRV_REG_ADDR(MRV_ISP_DEMOSAIC)]        = 0x008; //number of input frames to be sampled
    pMrvReg->isp_demosaic = 0x08;
    // enable filter
    //[0]:filt_enable
    //[1]:filt_mode:1-dynamic noise reduction/sharpen Default
	//[5:4]:chroma filter vertical mode:01-vertical chroma filter 1 static[8 16 8]
	//[7:6]:chroma filter horizontal mode:10-horizontal chroma filter2(dynamic blur1)
    //regbase[MRV_REG_ADDR(MRV_ISP_FILT_MODE)]       = (0x1|(0x1<<1)|(0x1<<4)|(0x2<<6));
    pMrvReg->isp_filt_mode = (0x1|(0x1<<1)|(0x1<<4)|(0x2<<6));
     */
    // set MI main Y buffer position and size
    pMrvReg->mi_mp_y_base_ad_init = buf;
    pMrvReg->mi_mp_y_size_init = camera_test_info.width*camera_test_info.height*2;
    // set MI main Cb buffer position and size
    pMrvReg->mi_mp_cb_base_ad_init = 0;
    pMrvReg->mi_mp_cb_size_init = 0;
    // set MI main Cr buffer position and size
    pMrvReg->mi_mp_cr_base_ad_init = 0;
    pMrvReg->mi_mp_cr_size_init = 0;

    // enable all MI interrupts
    //main picture end of frame interrupt
    pMrvReg->mi_imsc = 0x01;

    // set MI in main picture only mode and update shadow registers immediately
    //sp_output_format = YUV422;
    //sp_input_format  = YUV422;
    //sp_write_format  = PLANAR;
    //mp_write_format  = PLANAR;
    //init_offs_en     = 1
    //init_base_en     = 1;
    //burst_len_chrom  = BURST_4;
    //burst_len_lum    = BURST_4;
    //mp_enable        = 1;
    //regbase[MRV_REG_ADDR(MRV_MI_CTRL)]             = 0x28300001;
    //pMrvReg->mi_ctrl = 0x28300001;
    //pMrvReg->mi_ctrl = 0x752001;//mi_mp_y_size_init = camera_test_info.width*camera_test_info.height;
    pMrvReg->mi_ctrl = 0xba2001;//mi_mp_y_size_init = camera_test_info.width*camera_test_info.height*2;
    //pMrvReg->mi_ctrl = 0xb00001;//mi_mp_y_size_init = camera_test_info.width*camera_test_info.height*2;

    // soft update
    //regbase[MRV_REG_ADDR(MRV_MI_INIT)]             = 0x10;
    pMrvReg->mi_init = 0x10;
    //[0]: 1-output to add data fifo and to output interfaceis enable
    //[12:13] :num data lanes:  01 lane 1 and 2 used,
    //[18] : enable_clk
    if (camera_test_info.lane_num ==2)
       pMrvReg->mipi[0].mipi_ctrl = ((0x01<<12)|0x1<<18|(0x0f<<8));
    else if (camera_test_info.lane_num == 1)
       pMrvReg->mipi[0].mipi_ctrl = ((0x00<<12)|0x1<<18|(0x0f<<8));
    else if (camera_test_info.lane_num == 4)
       pMrvReg->mipi[0].mipi_ctrl = ((0x03<<12)|0x1<<18|(0x0f<<8));

    printf("marvin_config:  numLane: %d  mipi_ctrl: 0x%x\n",
        camera_test_info.lane_num,pMrvReg->mipi[0].mipi_ctrl);
    return 0;
}

int marvin_check_vsync(MrvAllRegister_t *pMrvReg)
{
    uint32_t isp_flags_shd = 0;
	int result = 0;

   //31  S_HSYNC	state of ISP input port s_hsync, for test purposes
   //30  S_VSYNC	state of ISP input port s_vsync, for test purposes
   //29:28  ---  unused
   //27:16  S_DATA  state of ISP input port s_data, for test purposes
   //15:3  ---  unused
   // 2  INFORM_FIELD	current field information (0=odd, 1=even)
   // 1  ISP_INFORM_ENABLE_SHD  Input formatter enable shadow register
   // 0  ISP_ENABLE_SHD  ISP enable shadow register shows, if ISP currently outputs data (1) or not (0)
   isp_flags_shd = pMrvReg->isp_flags_shd;
	isp_flags_shd &= 0xffffffff;

    if((isp_flags_shd & 0x0fff0000) != 0x0) {
		result = 1;
		printf("check vsync is high! isp_flags_shd:%0x%08x 00\n",isp_flags_shd);
	} else {
		result = 0;
		printf("check vsync is low! isp_flags_shd:%0x%08x \n",isp_flags_shd);
	}

    printf("isp_flags_shd: 0x%08x result:%d\n",isp_flags_shd,result);
    return result;
}


int marvin_start(MrvAllRegister_t *pMrvReg)
{
    pMrvReg->mipi[0].mipi_ctrl |= 0x01;

    pMrvReg->isp_ctrl = ((pMrvReg->isp_ctrl = 0x8c0 | (camera_test_info.Mode<<1))|(1<<9)|(1<<4)|(1<<0));
    printf("isp turn on!\n");
    return 0;
}

int marvin_stop(MrvAllRegister_t *pMrvReg)
{
    pMrvReg->isp_ctrl = (1<<8);
    pMrvReg->mipi[0].mipi_ctrl = 0x00;
    sleep(1);
    pMrvReg->vi_iccl = 0x00;
    pMrvReg->vi_ccl = (1<<2);

    return 0;
}

int extdev_register(void)
{
    int err;
    camsys_devio_name_t extdev;

	memset(&extdev,0x00, sizeof(camsys_devio_name_t));
    extdev.dev_id = camera_test_info.dev_id;
	strlcpy((char*)extdev.dev_name, camera_test_info.sensorname,sizeof(camera_test_info.sensorname));
    strlcpy((char*)extdev.avdd.name, "NC",sizeof(extdev.avdd.name));
	extdev.avdd.min_uv = 0x0;
    extdev.avdd.max_uv = 0x0;
    strlcpy((char*)extdev.dovdd.name,"NC",sizeof(extdev.dovdd.name));
    extdev.dovdd.min_uv = 18000000;
    extdev.dovdd.max_uv = 18000000;
    strlcpy((char*)extdev.dvdd.name, "NC",sizeof(extdev.dvdd.name));
    extdev.dvdd.min_uv = 0x0;
    extdev.dvdd.max_uv = 0x0;
    strlcpy((char*)extdev.afvdd.name, "NC",sizeof(extdev.afvdd.name));
    extdev.afvdd.min_uv = 0x0;
    extdev.afvdd.max_uv = 0x0;
    strlcpy((char*)extdev.pwrdn.name, "RK30_PIN1_PA4",sizeof(extdev.pwrdn.name));
    extdev.pwrdn.active = 0x00;
    strlcpy((char*)extdev.pwren.name, "NC",sizeof(extdev.pwren.name));
    extdev.pwren.active = 0x01;
    strlcpy((char*)extdev.rst.name,"RK30_PIN2_PB2",sizeof(extdev.rst.name));
	extdev.rst.active = 0x0;
    strlcpy((char*)extdev.afpwrdn.name,"NC",sizeof(extdev.afpwrdn.name));
	extdev.afpwrdn.active = 0x0;
    strlcpy((char*)extdev.afpwr.name,"NC",sizeof(extdev.afpwr.name));
	extdev.afpwr.active = 0x0;

	extdev.dev_cfg |= CAMSYS_DEVCFG_FLASHLIGHT;
	extdev.fl.fl.active = 0;
	extdev.fl.fl_en.active = 0;
	strlcpy((char*)extdev.fl.fl.name,"NC",sizeof(extdev.fl.fl.name));
	strlcpy((char*)extdev.fl.fl_en.name,"NC",sizeof(extdev.fl.fl_en.name));
	strlcpy((char*)extdev.fl.fl_drv_name,"NC",sizeof(extdev.fl.fl_drv_name));
    extdev.phy.type = camera_test_info.phy_type;
	printf("%s %d phy_type:%d \n",__FUNCTION__,__LINE__,camera_test_info.phy_type);

    if(extdev.phy.type == CamSys_Phy_Cif){
        //extdev.phy.type = CamSys_Phy_Cif;
        extdev.phy.info.cif.fmt = camera_test_info.fmt;
        extdev.phy.info.cif.cif_num = camera_test_info.cif_num;
        extdev.phy.info.cif.cifio = camera_test_info.cifio;
    }else if(extdev.phy.type == CamSys_Phy_Mipi){
		extdev.phy.info.mipi.phy_index = camera_test_info.phy_index;
		//extdev.phy.info.mipi.data_en_bit = camera_test_info.data_en_bit;
		extdev.phy.info.mipi.bit_rate = camera_test_info.bit_rate;
		if (camera_test_info.lane_num == 1) {
			extdev.phy.info.mipi.data_en_bit = 0x01;
		} else if (camera_test_info.lane_num == 2) {
			extdev.phy.info.mipi.data_en_bit = 0x03;
		} else if (camera_test_info.lane_num == 4) {
			extdev.phy.info.mipi.data_en_bit = 0x0f;
		}
    }else{
        printf("%s %d: unknow phy type(%d)\n",__func__,__LINE__, extdev.phy.type);
    }

    extdev.clk.in_rate = SENSOR_MCLK;
	printf("%s %d  camsys_fd:%d hcc\n",__FUNCTION__,__LINE__,camsys_fd);

    err = ioctl(camsys_fd, CAMSYS_REGISTER_DEVIO, &extdev);
    printf("register_vide return %d\n", err);
    usleep(1000*1000);
    if (err<0) {
        printf("CAMSYS_REGISTER_DEVIO failed\n");
    }

    return err;
}

int extdev_register_front(void)
{
    int err;
    camsys_devio_name_t extdev;

	memset(&extdev,0x00, sizeof(camsys_devio_name_t));
    extdev.dev_id = camera_test_info.dev_id;
	strlcpy((char*)extdev.dev_name, camera_test_info.sensorname,sizeof(camera_test_info.sensorname));
    strlcpy((char*)extdev.avdd.name, "NC",sizeof(extdev.avdd.name));
	extdev.avdd.min_uv = 0x0;
    extdev.avdd.max_uv = 0x0;
    strlcpy((char*)extdev.dovdd.name,"NC",sizeof(extdev.dovdd.name));
    extdev.dovdd.min_uv = 18000000;
    extdev.dovdd.max_uv = 18000000;
    strlcpy((char*)extdev.dvdd.name, "NC",sizeof(extdev.dvdd.name));
    extdev.dvdd.min_uv = 0x0;
    extdev.dvdd.max_uv = 0x0;
    strlcpy((char*)extdev.afvdd.name, "NC",sizeof(extdev.afvdd.name));
    extdev.afvdd.min_uv = 0x0;
    extdev.afvdd.max_uv = 0x0;
    strlcpy((char*)extdev.pwrdn.name, "RK30_PIN1_PB4",sizeof(extdev.pwrdn.name));
    extdev.pwrdn.active = 0x01;
    strlcpy((char*)extdev.pwren.name, "RK30_PIN0_PD0",sizeof(extdev.pwren.name));
    extdev.pwren.active = 0x01;
    strlcpy((char*)extdev.rst.name,"NC",sizeof(extdev.rst.name));
	extdev.rst.active = 0x0;
    strlcpy((char*)extdev.afpwrdn.name,"NC",sizeof(extdev.afpwrdn.name));
	extdev.afpwrdn.active = 0x0;
    strlcpy((char*)extdev.afpwr.name,"NC",sizeof(extdev.afpwr.name));
	extdev.afpwr.active = 0x0;

	extdev.dev_cfg |= CAMSYS_DEVCFG_FLASHLIGHT;
	extdev.fl.fl.active = 0;
	extdev.fl.fl_en.active = 0;
	strlcpy((char*)extdev.fl.fl.name,"NC",sizeof(extdev.fl.fl.name));
	strlcpy((char*)extdev.fl.fl_en.name,"NC",sizeof(extdev.fl.fl_en.name));
	strlcpy((char*)extdev.fl.fl_drv_name,"NC",sizeof(extdev.fl.fl_drv_name));
	printf("%s %d  \n",__FUNCTION__,__LINE__);
	printf("%s %d phy_type:%d \n",__FUNCTION__,__LINE__,camera_test_info.phy_type);
    extdev.phy.type = camera_test_info.phy_type;
	printf("%s %d phy_type:%d \n",__FUNCTION__,__LINE__,camera_test_info.phy_type);

    if(extdev.phy.type == CamSys_Phy_Cif){
        //extdev.phy.type = CamSys_Phy_Cif;
        extdev.phy.info.cif.fmt = camera_test_info.fmt;
        extdev.phy.info.cif.cif_num = camera_test_info.cif_num;
        extdev.phy.info.cif.cifio = camera_test_info.cifio;
    }else if(extdev.phy.type == CamSys_Phy_Mipi){
		extdev.phy.info.mipi.phy_index = camera_test_info.phy_index;
		//extdev.phy.info.mipi.data_en_bit = camera_test_info.data_en_bit;
		extdev.phy.info.mipi.bit_rate = camera_test_info.bit_rate;
		if (camera_test_info.lane_num == 1) {
			extdev.phy.info.mipi.data_en_bit = 0x01;
		} else if (camera_test_info.lane_num == 2) {
			extdev.phy.info.mipi.data_en_bit = 0x03;
		} else if (camera_test_info.lane_num == 4) {
			extdev.phy.info.mipi.data_en_bit = 0x0f;
		}
    }else{
        printf("%s %d: unknow phy type(%d)\n",__func__,__LINE__, extdev.phy.type);
    }

    extdev.clk.in_rate = SENSOR_MCLK;
	printf("%s %d  hcc\n",__FUNCTION__,__LINE__);

    err = ioctl(camsys_fd, CAMSYS_REGISTER_DEVIO, &extdev);
    if (err<0) {
        printf("CAMSYS_REGISTER_DEVIO failed\n");
    }

    return err;

}

int extdev_init(int camsys_fd,unsigned int *i2cbase)
{
    int err,i;
    camsys_sysctrl_t sysctl;
    camsys_mipiphy_t mipiphy;
	int id = 0;

    sysctl.dev_mask = (camera_test_info.dev_id|CAMSYS_DEVID_MARVIN);
    sysctl.ops = CamSys_ClkIn;
    sysctl.on = 1;//2;
    printf("%s %d: dev_id:0x%08x\n",__FUNCTION__,__LINE__,camera_test_info.dev_id);

    usleep(1000*1000);
    err = ioctl(camsys_fd, CAMSYS_SYSCTRL, &sysctl);
	if (err<0) {
		printf("CamSys_ClkIn on failed\n");
		goto end;
	}
    usleep(15000);

    sysctl.dev_mask = camera_test_info.dev_id;
    sysctl.ops = CamSys_Phy;
    sysctl.on = 1;

    mipiphy.phy_index = camera_test_info.phy_index;
	if (camera_test_info.lane_num == 1) {
		mipiphy.data_en_bit = 0x01;
	} else if (camera_test_info.lane_num == 2) {
		mipiphy.data_en_bit = 0x03;
	} else if (camera_test_info.lane_num == 4) {
		mipiphy.data_en_bit = 0x0f;
	}

    mipiphy.bit_rate = camera_test_info.bit_rate;//656;
	usleep(1000*1000);


    memcpy(sysctl.rev,&mipiphy, sizeof(camsys_mipiphy_t));
    err = ioctl(camsys_fd, CAMSYS_SYSCTRL, &sysctl);
    if (err<0) {
        printf("CamSys_Phy on failed");
		goto end;
    }
	usleep(5000);

	sysctl.dev_mask = camera_test_info.dev_id;
	sysctl.ops = CamSys_PwrEn;
	sysctl.on = 1;
	err = ioctl(camsys_fd, CAMSYS_SYSCTRL, &sysctl);
	if (err<0) {
		printf("CamSys_PwrEn on failed");
		goto end;
	}
	usleep(10000);

	sysctl.dev_mask = camera_test_info.dev_id;
	sysctl.ops = CamSys_PwrDn;
	sysctl.on = 0;
	err = ioctl(camsys_fd, CAMSYS_SYSCTRL, &sysctl);
	if (err<0) {
		printf("CamSys_PwrDn on failed");
		goto end;
	}
	usleep(20000);

	sysctl.dev_mask = camera_test_info.dev_id;
	sysctl.ops = CamSys_Rst;
	sysctl.on = 0;
	err = ioctl(camsys_fd, CAMSYS_SYSCTRL, &sysctl);
	if (err<0) {
		printf("CmSys_PwrDn on failed\n");
		goto end;
	}
	usleep(100000);

	if (!strcmp(BACK_SENSOR_NAME, "ov5648"))
		err = Ov5648_sensor_reg_init(camsys_fd, i2cbase);
	else if (!strcmp(BACK_SENSOR_NAME, "ov13850"))
		err = Ov13850_sensor_reg_init(camsys_fd, i2cbase);
	else
		printf(" %s sensor_reg_init fail\n", BACK_SENSOR_NAME);
	usleep(1000*2000);

end:
    return err;
}

int extdev_init_front(int camsys_fd,unsigned int *i2cbase)
{
    int err,i;
    camsys_sysctrl_t sysctl;
    camsys_mipiphy_t mipiphy;
	int id = 1;

    sysctl.dev_mask = (camera_test_info.dev_id|CAMSYS_DEVID_MARVIN);
    sysctl.ops = CamSys_ClkIn;
    sysctl.on = 1;
	printf("%s %d  hcc\n",__FUNCTION__,__LINE__);
    err = ioctl(camsys_fd, CAMSYS_SYSCTRL, &sysctl);
    if (err<0) {
        printf("CamSys_ClkIn on failed\n");
		goto end;
    }
	printf("%s %d  hcc\n",__FUNCTION__,__LINE__);
    usleep(15000);

    sysctl.dev_mask = camera_test_info.dev_id;
    sysctl.ops = CamSys_Phy;
    sysctl.on = 1;
	printf("%s %d  hcc\n",__FUNCTION__,__LINE__);

    mipiphy.phy_index = camera_test_info.phy_index;
	if (camera_test_info.lane_num == 1) {
		mipiphy.data_en_bit = 0x01;
	} else if (camera_test_info.lane_num == 2) {
		mipiphy.data_en_bit = 0x03;
	} else if (camera_test_info.lane_num == 4) {
		mipiphy.data_en_bit = 0x0f;
	}

    mipiphy.bit_rate = camera_test_info.bit_rate;//656;
	printf("%s %d  \n",__FUNCTION__,__LINE__);


    memcpy(sysctl.rev,&mipiphy, sizeof(camsys_mipiphy_t));
	printf("%s %d  hcc\n",__FUNCTION__,__LINE__);
    err = ioctl(camsys_fd, CAMSYS_SYSCTRL, &sysctl);
    if (err<0) {
        printf("CamSys_Phy on failed");
		goto end;
    }
	printf("%s %d  hcc\n",__FUNCTION__,__LINE__);
    usleep(5000);

    sysctl.dev_mask = camera_test_info.dev_id;
    sysctl.ops = CamSys_PwrEn;
    sysctl.on = 1;
    err = ioctl(camsys_fd, CAMSYS_SYSCTRL, &sysctl);
    if (err<0) {
        printf("CamSys_PwrEn on failed");
		goto end;
    }
	printf("%s %d  hcc\n",__FUNCTION__,__LINE__);
	usleep(1000);
    sysctl.dev_mask = camera_test_info.dev_id;
    sysctl.ops = CamSys_Rst;
    sysctl.on = 0;
    err = ioctl(camsys_fd, CAMSYS_SYSCTRL, &sysctl);
    if (err<0) {
        printf("CamSys_PwrDn on failed\n");
		goto end;
    }
	printf("%s %d  hcc\n",__FUNCTION__,__LINE__);

    usleep(1000);
    sysctl.dev_mask = camera_test_info.dev_id;
    sysctl.ops = CamSys_PwrDn;
    sysctl.on = 0;
    err = ioctl(camsys_fd, CAMSYS_SYSCTRL, &sysctl);
    if (err<0) {
        printf("CamSys_PwrDn on failed");
		goto end;
    }
    usleep(2000);
	printf("%s %d  hcc\n",__FUNCTION__,__LINE__);

    if (!strcmp(FRONT_SENSOR_NAME, "gc2155"))
	err = Gc2155_sensor_reg_init(camsys_fd,i2cbase);
    else if (!strcmp(FRONT_SENSOR_NAME, "gc2145"))
	err = Gc2145_sensor_reg_init(camsys_fd,i2cbase);
    else if (!strcmp(FRONT_SENSOR_NAME, "gc0329"))
	err = Gc0329_sensor_reg_init(camsys_fd,i2cbase); 
    else
		printf(" %s sensor_reg_init fail", FRONT_SENSOR_NAME);

end:
    return err;
}

typedef struct ionbuf {
    int ion_fd;
    int buf_fd;
    unsigned int len;
    ion_user_handle_t handle;
    unsigned int phy;
    unsigned int *vir;
} ionbuf_t;

int allocbuf_ion(ionbuf_t *ionbuf)
{
    int err;
    ionbuf->ion_fd = ion_open();
    if (ionbuf->ion_fd < 0) {
        printf("Open ion device failed\n");
    } else {
        //err = ion_alloc(ionbuf->ion_fd, ION_LEN, 4, ION_HEAP(ION_CMA_HEAP_ID),0,&ionbuf->handle);
        err = ion_alloc(ionbuf->ion_fd, ION_LEN, 4, ION_HEAP(4),0,&ionbuf->handle);
        if (err < 0) {
            printf("ion_alloc failed\n");
        } else {
            err = ion_share(ionbuf->ion_fd,ionbuf->handle,&ionbuf->buf_fd);
            if (err < 0) {
                printf("ion share failed\n");
            } else {
                err = ion_get_phys(ionbuf->ion_fd, ionbuf->handle, &ionbuf->phy);
                if (err<0) {
                    printf("ion get phys failed\n");
                } else{
                    printf("ion get phys: 0x%x\n", ionbuf->phy);
                }

                ionbuf->vir = (unsigned int*)mmap(0,ION_LEN,
                        PROT_READ|PROT_WRITE, MAP_SHARED,
                        ionbuf->buf_fd,0);

                if (ionbuf->vir == MAP_FAILED) {
                    printf("ion buffer mmap failed\n");
                } else {
                    printf("ion buffer: %p\n", ionbuf->vir);

                    memset((char*)ionbuf->vir,0x7a, ION_LEN);

                    return 0;
                }
            }
        }
    }

    return -1;
}

int freebuf_ion(ionbuf_t *ionbuf)
{
    if (ionbuf->vir) {
        munmap((void*)ionbuf->vir,ionbuf->len);
        ionbuf->vir = NULL;
    }

    if (ionbuf->buf_fd>0) {
        close(ionbuf->buf_fd);
        ionbuf->buf_fd = -1;
    }

    if (ionbuf->handle != -1) {
        ion_free(ionbuf->ion_fd, ionbuf->handle);
        ionbuf->handle = -1;
    }

    if (ionbuf->ion_fd>0) {
        close(ionbuf->ion_fd);
        ionbuf->ion_fd = -1;
    }

    return 0;
}

struct drm_rockchip_gem_phys {
	uint32_t handle;
	uint32_t phy_addr;
};
#define DRM_ROCKCHIP_GEM_GET_PHYS	0x04
#define DRM_IOCTL_ROCKCHIP_GEM_GET_PHYS		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_ROCKCHIP_GEM_GET_PHYS, struct drm_rockchip_gem_phys)

typedef struct drmbuf {
	int fd;
	const gralloc_module_t *module;
	struct alloc_device_t *mGrallocAllocDev;
	struct gralloc_drm_handle_t *buffHandle;
	unsigned int flag;

	unsigned long vir_addr;
	unsigned long phy_addr;
	size_t size;
} drmbuf_t;

int allocbuf_drm(drmbuf_t *drmbuf)
{
	int err;
	hw_device_t *device;
	unsigned int pix_format;
	int stride;
	void *vir_addr;
	uint32_t phy_addr;

	err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, (const hw_module_t **)&drmbuf->module);
	if (!err) {
		err = gralloc_open(drmbuf->module, &drmbuf->mGrallocAllocDev);
		if (err) {
			printf("Unable to open gralloc alloc device(error %d)\n", err);
		}
	} else {
	     printf("hw_get_module GRALLOC_HARDWARE_MODULE_ID(error %d)\n", err);
	     return 0;
	}

	drmbuf->flag = 0;
	drmbuf->flag |= GRALLOC_USAGE_HW_CAMERA_WRITE |
			GRALLOC_USAGE_HW_CAMERA_READ |
			GRALLOC_USAGE_SW_WRITE_OFTEN |
			GRALLOC_USAGE_SW_READ_OFTEN |
			GRALLOC_USAGE_TO_USE_PHY_CONT;
	pix_format = HAL_PIXEL_FORMAT_YCrCb_NV12;

	err = drmbuf->mGrallocAllocDev->alloc(drmbuf->mGrallocAllocDev,
		camera_test_info.width,
		camera_test_info.height,
		pix_format,
		drmbuf->flag,
		(buffer_handle_t *)&drmbuf->buffHandle,
		&stride);
	if (err) {
		printf("%s(%d): grlalloc buffer allocation failed (error: %d)\n",
			__func__, __LINE__, err);
	}

	err = drmbuf->module->lock(drmbuf->module,
		(buffer_handle_t)drmbuf->buffHandle,
		drmbuf->flag, 0, 0,
		camera_test_info.width,
		camera_test_info.height,
		&vir_addr);
	drmbuf->vir_addr = (unsigned long)vir_addr;

	err = drmbuf->module->perform(drmbuf->module,
		GRALLOC_MODULE_PERFORM_GET_HADNLE_PHY_ADDR,
		(buffer_handle_t)drmbuf->buffHandle,
		&phy_addr);
	drmbuf->phy_addr = phy_addr;

	err = drmbuf->module->perform(drmbuf->module,
		GRALLOC_MODULE_PERFORM_GET_HADNLE_SIZE,
		(buffer_handle_t)drmbuf->buffHandle,
		&drmbuf->size);

	printf("vir_addr 0x%x, fd 0x%x, size %d\n", drmbuf->vir_addr, drmbuf->phy_addr, drmbuf->size);
	return 0;
}

int freebuf_drm(drmbuf_t *drmbuf)
{
	int ret = 0;

	if (drmbuf->mGrallocAllocDev) {
		ret = gralloc_close(drmbuf->mGrallocAllocDev);
		if (ret) {
			printf("gralloc close failed (error %d)\n", ret);
		}
		drmbuf->mGrallocAllocDev = NULL;
	}

	drmbuf->module = NULL;
	return 0;
}

void Raw12_to_Raw10(unsigned short int *src, unsigned char *dst)
{
    unsigned int w,h,i;
    unsigned int tmp;

    for (h=0; h<camera_test_info.height; h++) {
        for (w=0; w<(camera_test_info.width/4); w++) {
            for (i=0; i<4; i++) {
                *dst++ = (*(src+i)&0xff);
            }
            *dst++ = ((*(src+0)&0xc000)>>14) | ((*(src+1)&0xc000)>>12) |
                     ((*(src+2)&0xc000)>>10) | ((*(src+3)&0xc000)>>8);
            src += 4;
        }
    }
}

void arm_yuyv_to_nv12(int src_w, int src_h,char *srcbuf, char *dstbuf){

    char *srcbuf_begin;
    int *dstint_y, *dstint_uv, *srcint;
    int i = 0,j = 0;
    int y_size = 0;

	y_size = src_w*src_h;
	dstint_y = (int*)dstbuf;
	srcint = (int*)srcbuf;
	dstint_uv =  (int*)(dstbuf + y_size);

    for(i=0;i<src_h; i++) {
		for (j=0; j<(src_w>>2); j++) {
			if(i%2 == 0){
			    *dstint_uv++ = (*(srcint+1)&0xff000000)|((*(srcint+1)&0x0000ff00)<<8)
				        |((*srcint&0xff000000)>>16)|((*srcint&0x0000ff00)>>8);
			 }
			 *dstint_y++ = ((*(srcint+1)&0x00ff0000)<<8)|((*(srcint+1)&0x000000ff)<<16)
			            |((*srcint&0x00ff0000)>>8)|(*srcint&0x000000ff);
		     srcint += 2;
		 }
	 }

}

void arm_uyvy_to_nv12(int src_w, int src_h,char *srcbuf, char *dstbuf){

    char *srcbuf_begin;
    int *dstint_y, *dstint_uv, *srcint;
    int i = 0,j = 0;
    int y_size = 0;

	y_size = src_w*src_h;
	dstint_y = (int*)dstbuf;
	srcint = (int*)srcbuf;
	dstint_uv =  (int*)(dstbuf + y_size);

    for(i=0;i<src_h; i++) {
		for (j=0; j<(src_w>>2); j++) {
			if(i%2 == 1){
			    *dstint_uv++ = ((*(srcint+1)&0x00ff0000)<<8)|((*(srcint+1)&0x000000ff)<<16)
			            |((*srcint&0x00ff0000)>>8)|(*srcint&0x000000ff);
			 }
			 *dstint_y++ = (*(srcint+1)&0xff000000)|((*(srcint+1)&0x0000ff00)<<8)
				        |((*srcint&0xff000000)>>16)|((*srcint&0x0000ff00)>>8);
		     srcint += 2;
		 }
	 }

}




/**add by cx end***/


int Camera_Click_Event(int x,int y)
{
	struct list_head *pos;
	int x_start,x_end;
	int y_start,y_end;
	int err;

	if(tc_info == NULL)
		return -1;

	if(camera_num < 2)
		return -1;

	get_camera_size();

	x_start = camera_x;
	x_end = x_start + camera_w;
	y_start = camera_y;
	y_end = y_start + camera_h;

	if( (x >= x_start) && (x <= x_end) && (y >= y_start) && (y <= y_end))
	{
		printf("Camera_Click_Event : change \r\n");
		stopCameraTest();
		usleep(100000);
		pthread_create(&camera_tid, NULL, startCameraTest, NULL);
	}

	return 0;
}


int CameraCreate(void)
{
	return 0;
}

int CameraStart(int phy_addr, int buffer_count, int w, int h)
{
    return 0;
}

int TaskStop(void)
{
	return 0;
}

int TaskRuning(int fps_total,int corx,int cory)
{
	return 0;
}

// the func is a while loop func , MUST  run in a single thread.
int startCameraTest(){
	int err = 0, err1 = 0,i = 0;
	camsys_sysctrl_t sysctl;
	camsys_querymem_t qmem;
	camsys_version_t camsys_ver;
	pthread_t thread[3];
    unsigned int m_Y = 0,m_Cr,m_Cb = 0,*regbase = 0,*i2cbase = 0,*csihostbase = 0, *grfbase = 0,*bufbase = 0,*cifbase = 0;
	FILE *image;
	MrvAllRegister_t *pMrvReg;
	ionbuf_t ionbuf;
	drmbuf_t drmbuf;
	int sleep_time = 1000;
	unsigned int *phy_addr = 0;
	unsigned int vsync_flag = 0;
	char test_result[200] = "";
	char tmp[100];

	if(tc_info->y <= 0)
		tc_info->y  = get_cur_print_y();

	ui_print_xy_rgba(0, tc_info->y , 255, 255, 0, 255, "%s:[%s..]\n", PCBA_CAMERA,
			 PCBA_TESTING);

	printf("%s %d back camera start test\n",__FUNCTION__,__LINE__);

	if (!strcmp(BACK_SENSOR_NAME, "ov13850"))
		Ov13850_get_SensorInfo(&camera_test_info);
	else if (!strcmp(BACK_SENSOR_NAME, "ov5648"))
		Ov5648_get_SensorInfo(&camera_test_info);
	else
		printf("%s get_SensorInfo fail\n", BACK_SENSOR_NAME);
	printf("Camera %s phy_type:%d image_w_h:%dx%d\n", BACK_SENSOR_NAME,
		camera_test_info.phy_type, camera_test_info.width,camera_test_info.height);

#if ION_USED
	ionbuf.ion_fd = -1;
	ionbuf.buf_fd = -1;
	ionbuf.handle = -1;
	ionbuf.phy = 0;
	ionbuf.vir = NULL;
	ionbuf.len = ION_LEN;
	err = allocbuf_ion(&ionbuf);
	printf("%s %d  hcc\n",__FUNCTION__,__LINE__);
	if (err) {
		printf("allocbuf_ion  failed\n");
		goto Next;
	}
#else
	allocbuf_drm(&drmbuf);
	printf("allocbuf drm\n");
#endif
	camsys_fd = open(CAMSYS_DEPATH_MARVIN,O_RDWR);
	printf("camsys_fd:%d\n", camsys_fd);
	if (camsys_fd < 0) {
		err = -1;
		printf("Open %s  failed\n", CAMSYS_DEPATH_MARVIN);
		goto Next;
	}

	qmem.mem_type = CamSys_Mmap_RegisterMem;
	err = ioctl(camsys_fd, CAMSYS_QUREYMEM, &qmem);
	if (err<0) {
		printf("CamSys_Mmap_RegisterMem query failed,%d\n",err);
		goto Next;
	} else {
		printf("regmem size: 0x%x	offset: 0x%lx\n",qmem.mem_size,qmem.mem_offset);
		regbase = (unsigned int*)mmap(0,qmem.mem_size,
						PROT_READ|PROT_WRITE, MAP_SHARED,
						camsys_fd,qmem.mem_offset);
		if (regbase == MAP_FAILED) {
			printf("register memory mmap failed!\n");
		} else {
			printf("register memory mmap success!\n");
			pMrvReg = (MrvAllRegister_t*)regbase;
		}
	}

	qmem.mem_type = CamSys_Mmap_I2cMem;
	err = ioctl(camsys_fd, CAMSYS_QUREYMEM, &qmem);
	if (err<0) {
		printf("CamSys_Mmap_I2cMem query failed\n");
		goto Next;
	} else {
		printf("i2cmem size: 0x%x	offset: 0x%lx\n",qmem.mem_size,qmem.mem_offset);
		i2cbase = (unsigned int*)mmap(0,qmem.mem_size,
						PROT_READ|PROT_WRITE, MAP_SHARED,
						camsys_fd,qmem.mem_offset);
		if (i2cbase == MAP_FAILED) {
			printf("i2c memory mmap failed!\n");
		} else {
			printf("i2cbase: %p   i2cbase[0]: 0x%x \n", i2cbase,*i2cbase);
		}
	}

	err = extdev_register();
	if (err < 0) {
		printf("extdev_register failed.\n");
		goto Next;
	}
	printf("%s %d camsys_fd:%d\n",__FUNCTION__,__LINE__,camsys_fd);
	usleep(100*1000);


	err = extdev_init(camsys_fd,i2cbase);
	if (err < 0) {
		printf("extdev_init failed.\n");
		goto Next;
	}
#if 0
#if ION_USED
	phy_addr = (unsigned int *)ionbuf.phy;
#else
	phy_addr = (unsigned int *)drmbuf.phy_addr;
#endif
	marvin_config((MrvAllRegister_t*)regbase, (unsigned int )phy_addr);
	marvin_start((MrvAllRegister_t*)regbase);
	usleep(100000);

	ioctl(camsys_fd, CAMSYS_VERCHK, &camsys_ver);
	if (camsys_ver.head_ver == CAMSYS_HEAD_VERSION) {
		printf("CamSys head version: v%d.%d.%d\n",(camsys_ver.head_ver&0xff0000)>>16,
			(camsys_ver.head_ver&0xff00)>>8,(camsys_ver.head_ver&0xff));
	} else {
		printf("CamSys head version check failed!User: v%d.%d.%d Kernel: v%d.%d.%d\n",
			(CAMSYS_HEAD_VERSION&0xff0000)>>16,
			(CAMSYS_HEAD_VERSION&0xff00)>>8,(CAMSYS_HEAD_VERSION&0xff),
			(camsys_ver.head_ver&0xff0000)>>16,
			(camsys_ver.head_ver&0xff00)>>8,(camsys_ver.head_ver&0xff));

	}

	if (!strcmp(BACK_SENSOR_NAME, "ov13850"))
		err1 = Ov13850_sensor_streamon(camsys_fd, 1);
	else if (!strcmp(BACK_SENSOR_NAME, "ov5648"))
		err1 = Ov5648_sensor_streamon(camsys_fd, 1);
	else
		printf("%s sensor streamon not ok\n", BACK_SENSOR_NAME);
	usleep(10000);

	for(i=0;i<200;i++){
		if(marvin_check_vsync((MrvAllRegister_t *) pMrvReg)){
			printf("%s %d  ISP input data is successed!\n",__FUNCTION__,__LINE__);
			break;
		} else {
			printf("%s %d  ISP input data is failed!\n",__FUNCTION__,__LINE__);
		}
		usleep(10000);//1fps(200*10ms/2)<framrate<100fps(10ms)
	}
	printf("%s %d  hcc, i = %d\n",__FUNCTION__,__LINE__, i);
	usleep(10000);

	if (i == 200) {
		err = -1;
		goto Next;
	}
	usleep(10000);

	FILE *fd;
	char filename[64];
	sprintf(filename,"/data/back_camera_%d_%d_yuv.bin",camera_test_info.width,camera_test_info.height);
	usleep(10000);
	fd = fopen(filename,"wb");
	printf("%s %d  hcc fb = 0x%x\n",__FUNCTION__,__LINE__, fd);
	usleep(10000);
	if(fd){
#if ION_USED
		fwrite(ionbuf.vir,0x01,camera_test_info.width*camera_test_info.height*2,fd);
#else
		fwrite((unsigned int *)drmbuf.vir_addr,0x01,camera_test_info.width*camera_test_info.height*2,fd);
#endif
		fclose(fd);
		printf("yuyv image write to file\n");
	} else {
		printf("/data/back_camera_%d_%d_yuv.bin open failed!\n",camera_test_info.width,camera_test_info.height);
		goto Next;
	}

	usleep(10000);
	//thread_disconnect(thread);
	marvin_stop((MrvAllRegister_t*)regbase);
#endif
	if (regbase) {
		munmap((void*)regbase, qmem.mem_size);
		regbase = NULL;
	}

	if (i2cbase) {
		munmap((void*)i2cbase, qmem.mem_size);
		i2cbase = NULL;
	}

	if (grfbase) {
        munmap((void*)grfbase, 0x1000);
        grfbase = NULL;
    }

    if (csihostbase) {
        munmap((void*)csihostbase, 0x4000);
        csihostbase = NULL;
    }

    if (cifbase) {
        munmap((void*)cifbase,0x10000);
        cifbase = NULL;
    }

	sysctl.dev_mask = camera_test_info.dev_id;
	sysctl.ops = CamSys_PwrDn;
	sysctl.on = 1;
	err = ioctl(camsys_fd, CAMSYS_SYSCTRL, &sysctl);
	if (err<0) {
		printf("CamSys_PwrDn off failed");
		//goto end;
	}

	sysctl.dev_mask = camera_test_info.dev_id;
	sysctl.ops = CamSys_Phy;
	sysctl.on = 0;
	err = ioctl(camsys_fd, CAMSYS_SYSCTRL, &sysctl);
	if (err<0) {
		printf("CamSys_Phy off failed");
		//goto end;
	}

	sysctl.dev_mask = (CAMSYS_DEVID_MARVIN|camera_test_info.dev_id);
	sysctl.ops = CamSys_ClkIn;
	sysctl.on = 0;

	err = ioctl(camsys_fd, CAMSYS_SYSCTRL, &sysctl);
	if (err<0) {
		printf("CamSys_ClkIn off failed\n");
		//goto end;
	}

Next:

	/*err = close(camsys_fd);
	printf("close camsys result:%d\n", err);
	if (err)
		printf("close camsys error: %d, errno:%d, %s\n", err,errno,strerror(errno));
	camsys_fd = 0;*/
	printf("%s %d  back camera test over.\n",__FUNCTION__,__LINE__);

	sprintf(tmp, "Back Camera:[%s] { ID:0x%x }", err < 0 ? PCBA_FAILED : PCBA_SECCESS,0);
	strcat(test_result, tmp);

	if (camera_num > 1) {
		usleep(1000000);
#if ION_USED
		freebuf_ion(&ionbuf);
#else
		freebuf_drm(&drmbuf);
#endif
		printf("%s %d front camera start test\n",__FUNCTION__,__LINE__);

		if (!strcmp(FRONT_SENSOR_NAME, "gc2155"))
			Gc2155_get_SensorInfo(&camera_test_info);
		else if (!strcmp(FRONT_SENSOR_NAME, "gc2145"))
			Gc2145_get_SensorInfo(&camera_test_info);
		else if (!strcmp(FRONT_SENSOR_NAME, "gc0329"))
		        Gc0329_get_SensorInfo(&camera_test_info);
		else
			printf("%s get_SensorInfo fail\n", FRONT_SENSOR_NAME);
		printf("Camera %s phy_type:%d image_w_h:%dx%d\n", FRONT_SENSOR_NAME,
			camera_test_info.phy_type, camera_test_info.width,camera_test_info.height);

#if ION_USED
		ionbuf.ion_fd = -1;
		ionbuf.buf_fd = -1;
		ionbuf.handle = -1;
		ionbuf.phy = 0;
		ionbuf.vir = NULL;
		ionbuf.len = ION_LEN;
		err1 = allocbuf_ion(&ionbuf);
		printf("%s %d  hcc\n",__FUNCTION__,__LINE__);
		if (err1) {
			printf("allocbuf_ion  failed\n");
			goto end;
		}
#else
		allocbuf_drm(&drmbuf);
#endif
		/*camsys_fd = open(CAMSYS_DEPATH_MARVIN,O_RDWR);
		if (camsys_fd < 0) {
			err1 = -1;
			printf("Open %s  failed\n", CAMSYS_DEPATH_MARVIN);
			goto end;
		}*/
		printf("%s %d  hcc\n",__FUNCTION__,__LINE__);

		qmem.mem_type = CamSys_Mmap_RegisterMem;
		err1 = ioctl(camsys_fd, CAMSYS_QUREYMEM, &qmem);
		printf("%s %d  hcc\n",__FUNCTION__,__LINE__);
		if (err1<0) {
			printf("CamSys_Mmap_RegisterMem query failed,%d\n",err1);
			goto end;
		} else {
			printf("regmem size: 0x%x	offset: 0x%lx\n",qmem.mem_size,qmem.mem_offset);
			regbase = (unsigned int*)mmap(0,qmem.mem_size,
							PROT_READ|PROT_WRITE, MAP_SHARED,
							camsys_fd,qmem.mem_offset);
			if (regbase == MAP_FAILED) {
				printf("register memory mmap failed!\n");
			} else {
				printf("register memory mmap success!\n");
				pMrvReg = (MrvAllRegister_t*)regbase;
			}
		}

		qmem.mem_type = CamSys_Mmap_I2cMem;
		err1 = ioctl(camsys_fd, CAMSYS_QUREYMEM, &qmem);
		if (err1<0) {
			printf("CamSys_Mmap_I2cMem query failed\n");
			goto end;
		} else {
			printf("i2cmem size: 0x%x	offset: 0x%lx\n",qmem.mem_size,qmem.mem_offset);
			i2cbase = (unsigned int*)mmap(0,qmem.mem_size,
							PROT_READ|PROT_WRITE, MAP_SHARED,
							camsys_fd,qmem.mem_offset);
			if (i2cbase == MAP_FAILED) {
				printf("i2c memory mmap failed!\n");
			} else {
				printf("i2cbase: %p   i2cbase[0]: 0x%x \n", i2cbase,*i2cbase);
			}
		}

		err1 = extdev_register_front();
		if (err1 < 0) {
			printf("extdev_register failed.\n");
			goto end;
		}
		printf("%s %d  hcc\n",__FUNCTION__,__LINE__);

		err1 = extdev_init_front(camsys_fd,i2cbase);
		if (err1 < 0) {
			printf("extdev_init failed.\n");
			goto end;
		}
#if 0
#if ION_USED
		phy_addr = (unsigned int *)ionbuf.phy;
#else
		phy_addr = (unsigned int *)drmbuf.phy_addr;
#endif
		marvin_config((MrvAllRegister_t*)regbase, (unsigned int)phy_addr);
		marvin_start((MrvAllRegister_t*)regbase);
		printf("%s %d  hcc\n",__FUNCTION__,__LINE__);

	    usleep(1000000);
		printf("%s %d  hcc\n",__FUNCTION__,__LINE__);

		ioctl(camsys_fd, CAMSYS_VERCHK, &camsys_ver);
		if (camsys_ver.head_ver == CAMSYS_HEAD_VERSION) {
			printf("CamSys head version: v%d.%d.%d\n",(camsys_ver.head_ver&0xff0000)>>16,
				(camsys_ver.head_ver&0xff00)>>8,(camsys_ver.head_ver&0xff));
		} else {
			printf("CamSys head version check failed!User: v%d.%d.%d Kernel: v%d.%d.%d\n",
				(CAMSYS_HEAD_VERSION&0xff0000)>>16,
				(CAMSYS_HEAD_VERSION&0xff00)>>8,(CAMSYS_HEAD_VERSION&0xff),
				(camsys_ver.head_ver&0xff0000)>>16,
				(camsys_ver.head_ver&0xff00)>>8,(camsys_ver.head_ver&0xff));

		}
		printf("%s %d  hcc\n",__FUNCTION__,__LINE__);


			printf("%s snesor streamon fail\n", FRONT_SENSOR_NAME);
		printf("%s %d  hcc\n",__FUNCTION__,__LINE__);

		usleep(1000000);

		for(i=0;i<200;i++){
			if(marvin_check_vsync((MrvAllRegister_t *) pMrvReg)){
				printf("%s %d  ISP input data is successed!\n",__FUNCTION__,__LINE__);
				break;
			} else {
				printf("%s %d  ISP input data is successed!\n",__FUNCTION__,__LINE__);
			}
			usleep(10000);//1fps(200*10ms/2)<framrate<100fps(10ms)
		}
		if (i == 200) {
			err1 = -1;
			goto end;
		}

		FILE *fd;
		char filename[64];
		sprintf(filename,"/data/front_camera_%d_%d_yuv.bin",camera_test_info.width,camera_test_info.height);
		fd = fopen(filename,"wb");
		if(fd){
#if ION_USED
			fwrite(ionbuf.vir,0x01,camera_test_info.width*camera_test_info.height*2,fd);
#else
			fwrite((unsigned int *)drmbuf.vir_addr,0x01,camera_test_info.width*camera_test_info.height*2,fd);
#endif
			fclose(fd);
			 printf("yuyv image write to file\n");
		} else {
			printf("/data/front_camera_%d_%d_yuv.bin open failed!\n",camera_test_info.width,camera_test_info.height);
			goto end;
		}


		printf("%s %d  hcc\n",__FUNCTION__,__LINE__);
		//thread_disconnect(thread);
		marvin_stop((MrvAllRegister_t*)regbase);
#endif
		if (regbase) {
			munmap((void*)regbase, qmem.mem_size);
			regbase = NULL;
		}

		if (i2cbase) {
			munmap((void*)i2cbase, qmem.mem_size);
			i2cbase = NULL;
		}

		if (grfbase) {
			munmap((void*)grfbase, 0x1000);
			grfbase = NULL;
		}

		if (csihostbase) {
			munmap((void*)csihostbase, 0x4000);
			csihostbase = NULL;
		}

		if (cifbase) {
			munmap((void*)cifbase,0x10000);
			cifbase = NULL;
		}
		printf("%s %d  hcc\n",__FUNCTION__,__LINE__);
		sysctl.dev_mask = camera_test_info.dev_id;
		sysctl.ops = CamSys_PwrDn;
		sysctl.on = 1;
		err1 = ioctl(camsys_fd, CAMSYS_SYSCTRL, &sysctl);
		if (err1<0) {
			printf("CamSys_PwrDn off failed");
			//goto end;
		}

		sysctl.dev_mask = camera_test_info.dev_id;
		sysctl.ops = CamSys_Phy;
		sysctl.on = 0;
		err1 = ioctl(camsys_fd, CAMSYS_SYSCTRL, &sysctl);
		if (err1<0) {
			printf("CamSys_Phy off failed");
			//goto end;
		}

		sysctl.dev_mask = (CAMSYS_DEVID_MARVIN|camera_test_info.dev_id);
		sysctl.ops = CamSys_ClkIn;
		sysctl.on = 0;

		err1 = ioctl(camsys_fd, CAMSYS_SYSCTRL, &sysctl);
		if (err1<0) {
			printf("CamSys_ClkIn off failed\n");
			//goto end;
		}
#if ION_USED
		freebuf_ion(&ionbuf);
#else
		freebuf_drm(&drmbuf);
#endif

		if (camsys_fd) {
			close(camsys_fd);
			camsys_fd = 0;
		}
		printf("%s %d  front camera test over\n",__FUNCTION__,__LINE__);
	}

end:
#if ION_USED
		freebuf_ion(&ionbuf);
#else
		freebuf_drm(&drmbuf);
#endif

	if (camsys_fd) {
		close(camsys_fd);
		camsys_fd = 0;
	}
	if (camera_num > 1) {
		sprintf(tmp, ",Front Camera:[%s] { ID:0x%x }", err1 < 0 ? PCBA_FAILED : PCBA_SECCESS,1);
		strcat(test_result, tmp);
	}
	printf("%s %d  hcc test_result:%s\n",__FUNCTION__,__LINE__,test_result);
	if (camera_num > 1) {
                if (err < 0 || err1 < 0)
                   ui_print_xy_rgba(0,tc_info->y,255,0,0,255, "Back Camera:[%s] { ID:0x%x } Front Camera:[%s] { ID:0x%x }\n",err < 0 ? PCBA_FAILED : PCBA_SECCESS,0
                        ,err1 < 0 ? PCBA_FAILED : PCBA_SECCESS,1);
                else
		    ui_print_xy_rgba(0,tc_info->y,0,255,0,255, "Back Camera:[%s] { ID:0x%x } Front Camera:[%s] { ID:0x%x }\n",err < 0 ? PCBA_FAILED : PCBA_SECCESS,0
			,err1 < 0 ? PCBA_FAILED : PCBA_SECCESS,1);
	} else {
                if (err < 0 || err1 < 0)
                    ui_print_xy_rgba(0,tc_info->y,255,0,0,255, "Back Camera:[%s] { ID:0x%x }\n",err < 0 ? PCBA_FAILED : PCBA_SECCESS,0);
                else
		    ui_print_xy_rgba(0,tc_info->y,0,255,0,255, "Back Camera:[%s] { ID:0x%x }\n",err < 0 ? PCBA_FAILED : PCBA_SECCESS,0);
	}
	//exit(0);
	printf("%s %d camera test over\n",__FUNCTION__,__LINE__);
	return 0;
}

int stopCameraTest(){
	return 0;
}
void finishCameraTest(){

}

int get_camera_size()
{
	if(camera_x>0 && camera_y>0 && camera_w>0 && camera_h >0)
		return 0;

	if(gr_fb_width() > gr_fb_height()){
		camera_w = ((gr_fb_width() >> 1) & ~0x03);//camera_msg->w;
		camera_h = ((gr_fb_height()*2/3) & ~0x03);// camera_msg->h;
	}
	else{
		camera_h = ((gr_fb_width() >> 1) & ~0x03);//camera_msg->w;
		camera_w = ((gr_fb_height()*2/3) & ~0x03);// camera_msg->h;
	}

	if(camera_w > 640)
		camera_w = 640;
	if(camera_h > 480)
		camera_h=480;

	camera_x = gr_fb_width() >> 1;
	camera_y = 0;

	return 0;
}


void * camera_test(void *argc)
{
	int ret,num;
	printf("------------enter camera_test------- \n");
	tc_info = (struct testcase_info *)argc;

	if (script_fetch("camera", "number",&num, 1) == 0) {
		printf("camera_test num:%d\r\n",num);
		camera_num = num;
	}

	pthread_create(&camera_tid, NULL, startCameraTest, NULL);

	return argc;
}

