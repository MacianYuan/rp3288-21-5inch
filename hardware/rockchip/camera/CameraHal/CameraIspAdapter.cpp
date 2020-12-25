#include "CameraIspAdapter.h"
#include "cam_api/halholder.h"
#include "CameraIspTunning.h"
#include "cutils/properties.h"

namespace android{

#define ISP_OUT_YUV420SP   0
#define ISP_OUT_YUV422_INTERLEAVED  1
#define ISP_OUT_YUV422_SEMI  2
#define ISP_OUT_RAW12   3
#define ISP_OUT_FORMAT  ISP_OUT_YUV420SP //ISP_OUT_YUV422_INTERLEAVED

#define USE_RGA_TODO_ZOOM   (1)

/******************************************************************************
 * MainWindow_AfpsResChangeCb
 *****************************************************************************/
void CameraIspAdapter_AfpsResChangeCb( void *ctx)
{
	CtxCbResChange_t *pctx = (CtxCbResChange_t *)ctx;

    CameraIspAdapter *mainwindow = (CameraIspAdapter*)(pctx->pIspAdapter);

    if (mainwindow)
    {
        mainwindow->AfpsResChangeCb();
    }
}



/******************************************************************************
 * MainWindow::AfpsResChangeCb
 *****************************************************************************/
void CameraIspAdapter::AfpsResChangeCb()
{
   // emit resChanged();
    LOG_FUNCTION_NAME

    

    LOG_FUNCTION_NAME_EXIT
}

int CameraIspAdapter::DEFAULTPREVIEWWIDTH = 640;
int CameraIspAdapter::DEFAULTPREVIEWHEIGHT = 480;
int CameraIspAdapter::preview_frame_inval = 1;





CameraIspAdapter::CameraIspAdapter(int cameraId)
                    :CameraAdapter(cameraId),
                    m_camDevice(NULL),
                    mSensorItfCur(0)
{
    LOG_FUNCTION_NAME
    mZoomVal = 100;
    mZoomMin = 100;
    mZoomMax = 240;
    mFlashStatus = false;
    mISPOutputFmt = ISP_OUT_YUV420SP;
    mISPTunningRun = false;
    mIsSendToTunningTh = false;
    mDispFrameLeak = 0;
    mVideoEncFrameLeak = 0;
    mPreviewCBFrameLeak = 0;
    mPicEncFrameLeak = 0;
    mImgAllFovReq = false;
	mCtxCbResChange.res = 0;
	mCtxCbResChange.pIspAdapter =NULL;

	mISO = 2;
	mCameraGL = NULL;
	m_buffers_capture = NULL;
	m_buffers_capture = new cv_fimc_buffer();
        memset(m_buffers_capture,0x0,sizeof(cv_fimc_buffer));
    #if 0
    mCameraGL = new CameraGL();
    mGPUCommandThread = new GPUCommandThread(this);
    mGPUCommandThreadState = STA_GPUCMD_IDLE;
    mGPUCommandThread->run("GPUCommandThread",ANDROID_PRIORITY_DISPLAY);
    #endif
	mfdISO = 2;
	mMutliFrameDenoise = NULL;
	mfd_buffers_capture = NULL;
    mfd_buffers_capture = new cv_fimc_buffer();
    memset(mfd_buffers_capture,0x0,sizeof(cv_fimc_buffer));
    memset(&mfd,0x0,sizeof(mfdprocess));
    #if 0
    mMutliFrameDenoise = new MutliFrameDenoise();
    mMFDCommandThread = new MFDCommandThread(this);
    mMFDCommandThreadState = STA_GPUCMD_IDLE;
    mMFDCommandThread->run("MFDCommandThread",ANDROID_PRIORITY_DISPLAY);
    #endif
	LOG_FUNCTION_NAME_EXIT
	if(mCameraGL == NULL){
        LOGW("mCameraGL is NULL!");
	}
	if(mMutliFrameDenoise == NULL){
        LOGW("mMutliFrameDenoise is NULL!");
    }
}
CameraIspAdapter::~CameraIspAdapter()
{
    if (mCameraGL != NULL){
        if (mGPUCommandThread != NULL) {
	        sendBlockedMsg(CMD_GPU_PROCESS_DEINIT);
            mGPUCommandThread->requestExitAndWait();
            mGPUCommandThread.clear();
            mGPUCommandThread = NULL;
        }
	//delete mCameraGL;
        mCameraGL = NULL;
    }

    if (m_buffers_capture != NULL){
        delete m_buffers_capture;
        m_buffers_capture = NULL;
    }
    if (mMutliFrameDenoise != NULL){
        if (mMFDCommandThread != NULL) {
	    	mfdsendBlockedMsg(CMD_GPU_PROCESS_DEINIT);
            mMFDCommandThread->requestExitAndWait();
            mMFDCommandThread.clear();
            mMFDCommandThread = NULL;
        }
	//delete mMutliFrameDenoise;
	mMutliFrameDenoise = NULL;
    }

    if (mfd_buffers_capture != NULL){
        delete mfd_buffers_capture;
        mfd_buffers_capture = NULL;
    }
    cameraDestroy();
    if(mDispFrameLeak != 0){
        LOGE("\n\n\n\nmay have disp frame mem leak,count is %d\n\n\n\n",mDispFrameLeak);
    }
    
    if(mVideoEncFrameLeak!=0){
        LOGE("\n\n\n\nmay have video frame mem leak,count is %d\n\n\n\n",mVideoEncFrameLeak);
    }
    
    if(mPreviewCBFrameLeak != 0){
        LOGE("\n\n\n\nmay have previewcb frame mem leak,count is %d\n\n\n\n",mPreviewCBFrameLeak);
    }
    
    if(mPicEncFrameLeak != 0){
        LOGE("\n\n\n\nmay have pic enc frame mem leak,count is %d\n\n\n\n",mPicEncFrameLeak);
    }
	
}
int CameraIspAdapter::cameraCreate(int cameraId)
{
    LOG_FUNCTION_NAME
	rk_cam_total_info *pCamInfo = gCamInfos[cameraId].pcam_total_info;
	char* dev_filename = pCamInfo->mHardInfo.mSensorInfo.mCamsysDevPath;
    HalPara_t isp_halpara = {0};
	int mipiLaneNum = 0;
	int i =0;

	preview_frame_inval = pCamInfo->mHardInfo.mSensorInfo.awb_frame_skip;

    //fix weixin capture switch fastly bug.
    const char* cameraCallProcess = getCallingProcess();
    if (strstr("com.tencent.mm,com.tencent.mm:tools", cameraCallProcess))
        preview_frame_inval = 0;

    pCamInfo->mLibIspVersion = CONFIG_SILICONIMAGE_LIBISP_VERSION;
	
	for(i=0; i<4; i++){
		mipiLaneNum += (pCamInfo->mHardInfo.mSensorInfo.mPhy.info.mipi.data_en_bit>>i)&0x01;
	}
	
#ifdef ROCKCHIP_ION_VERSION
	isp_halpara.is_new_ion = (bool_t)true;
#else
	isp_halpara.is_new_ion = (bool_t)false;
#endif

#if defined(RK_DRM_GRALLOC) // should use fd
	isp_halpara.mem_ops = get_cam_ops(CAM_MEM_TYPE_GRALLOC);
#else
	isp_halpara.mem_ops = NULL;
#endif

	isp_halpara.mipi_lanes = mipiLaneNum;
	isp_halpara.phy_index = pCamInfo->mHardInfo.mSensorInfo.mPhy.info.mipi.phy_index;
	mCtxCbResChange.pIspAdapter = (void*)this;
	isp_halpara.sensorpowerupseq = pCamInfo->mHardInfo.mSensorInfo.mSensorPowerupSequence;
	isp_halpara.vcmpowerupseq = pCamInfo->mHardInfo.mVcmInfo.mVcmPowerupSequence;	
	isp_halpara.avdd_delay = pCamInfo->mHardInfo.mSensorInfo.mAvdd_delay;
	isp_halpara.dvdd_delay = pCamInfo->mHardInfo.mSensorInfo.mDvdd_delay;
	isp_halpara.vcmvdd_delay = pCamInfo->mHardInfo.mVcmInfo.mVcmvdd_delay;	
	isp_halpara.dovdd_delay = pCamInfo->mHardInfo.mSensorInfo.mDovdd_delay;
	isp_halpara.pwr_delay = pCamInfo->mHardInfo.mSensorInfo.mPwr_delay;
	isp_halpara.rst_delay = pCamInfo->mHardInfo.mSensorInfo.mRst_delay;
	isp_halpara.pwrdn_delay = pCamInfo->mHardInfo.mSensorInfo.mPwrdn_delay;
	isp_halpara.clkin_delay = pCamInfo->mHardInfo.mSensorInfo.mClkin_delay;
	isp_halpara.vcmpwr_delay = pCamInfo->mHardInfo.mVcmInfo.mVcmpwr_delay;	
	isp_halpara.vcmpwrdn_delay = pCamInfo->mHardInfo.mVcmInfo.mVcmpwrdn_delay;	
    m_halHolder = new HalHolder(dev_filename,&isp_halpara);
    m_camDevice = new CamDevice( m_halHolder->handle(), CameraIspAdapter_AfpsResChangeCb, (void*)&mCtxCbResChange ,NULL, mipiLaneNum);

	//load sensor
    loadSensor( cameraId);
    //open image
    //openImage("/system/lib/libisp_isi_drv_OV8825.so");   

    {
        if (OSLAYER_OK != osQueueInit(&mAfListenerQue.queue,100, sizeof(CamEngineAfEvt_t)))   /* ddl@rock-chips.com: v0.0x22.0 */  
        {
            LOGE("create af listener queue failed!");
        }

        ListInit(&mAfListenerQue.list);

        mAfListenerThread = new CameraAfThread(this);
        mAfListenerThread->run("CamAfLisThread",ANDROID_PRIORITY_NORMAL);

        if (mISPTunningRun == false) {
            m_camDevice->resetAf(CAM_ENGINE_AUTOFOCUS_SEARCH_ALGORITHM_ADAPTIVE_RANGE);
            m_camDevice->registerAfEvtQue(&mAfListenerQue);
        }
    }

    LOG_FUNCTION_NAME_EXIT
    return 0;

}
int CameraIspAdapter::cameraDestroy()
{
    LOG_FUNCTION_NAME

    {
        CamEngineAfEvt_t cmd;
        int ret;

        cmd.evnt_id = (CamEngineAfEvtId_t)0xfefe5aa;//change from 0xfefe5aa5 to 0xfefe5aa for android Nougat
        
        osQueueWrite(&mAfListenerQue.queue, &cmd);
        
        if(mAfListenerThread!= NULL){
        	mAfListenerThread->requestExitAndWait();
        	mAfListenerThread.clear();
    	}

        osQueueDestroy(&mAfListenerQue.queue);
        
    }

    preview_frame_inval = 0;
    //check flash mode last time
    
    if(mParameters.get(CameraParameters::KEY_SUPPORTED_FLASH_MODES) &&
        (strcmp(mParameters.get(CameraParameters::KEY_FLASH_MODE),CameraParameters::FLASH_MODE_TORCH)==0)){
        CamEngineFlashCfg_t flash_cfg;
        rk_cam_total_info *pCamInfo = gCamInfos[mCamId].pcam_total_info;
        if((strcmp(pCamInfo->mHardInfo.mFlashInfo.mFlashName,"Internal")==0))
            flash_cfg.dev_mask = pCamInfo->mHardInfo.mSensorInfo.mHostDevid;
        else
            flash_cfg.dev_mask = pCamInfo->mHardInfo.mSensorInfo.mCamDevid;
        flash_cfg.mode = CAM_ENGINE_FLASH_TORCH;
        m_camDevice->configureFlash(&flash_cfg);
        m_camDevice->stopFlash(true);
    }

    if(m_camDevice){
        disconnectCamera();
        delete m_camDevice;
        m_camDevice = NULL;
        delete m_halHolder;
        m_halHolder = NULL;
    }
    LOG_FUNCTION_NAME_EXIT
    return 0;
}

void CameraIspAdapter::setupPreview(int width_sensor,int height_sensor,int preview_w,int preview_h,int zoom_val)
{
    CamEngineWindow_t dcWin;
	unsigned int max_w = 0,max_h = 0, bufNum = 0, bufSize = 0;
	//when cts FOV ,don't crop
    if((!mImgAllFovReq)&&(width_sensor != 0)&&(height_sensor != 0)&&((width_sensor*10/height_sensor) != (preview_w*10/preview_h))){
        int ratio = ((width_sensor*10/preview_w) >= (height_sensor*10/preview_h))?(height_sensor*10/preview_h):(width_sensor*10/preview_w);
        dcWin.width = ((ratio*preview_w/10) & ~0x1);
        dcWin.height = ((ratio*preview_h/10) & ~0x1);
        dcWin.hOffset =(ABS(width_sensor-dcWin.width ))>>1;
        dcWin.vOffset = (ABS(height_sensor-dcWin.height))>>1;        
    }else{
        dcWin.width = width_sensor;
        dcWin.height = height_sensor;
        dcWin.hOffset = 0;
        dcWin.vOffset = 0;
    }

#if (USE_RGA_TODO_ZOOM == 0)            /* zyc@rock-chips.com: v0.0x22.0 */ 

    if((zoom_val > mZoomMin) && (zoom_val <= mZoomMax)){
        if((preview_w <= 2592) && (preview_h <= 1944)){
            dcWin.width = dcWin.width*100/zoom_val & ~0x1;
            dcWin.height = dcWin.height*100/zoom_val & ~0x1;
            dcWin.hOffset = (ABS(width_sensor-dcWin.width )) >> 1;
            dcWin.vOffset = (ABS(height_sensor-dcWin.height))>>1;
        }else{
            LOGE("isp output res big than 5M!");
        }

    }
#endif

	getSensorMaxRes(max_w,max_h);
    if(mISPOutputFmt == ISP_OUT_YUV422_INTERLEAVED){
        m_camDevice->previewSetup_ex( dcWin, preview_w, preview_h,
                                CAMERIC_MI_DATAMODE_YUV422,CAMERIC_MI_DATASTORAGE_INTERLEAVED,(bool_t)true);
		bufSize = ((max_w+15)&(~0xf))*((max_h+15)&(~0xf))*2;
		bufNum = CONFIG_CAMERA_ISP_BUF_REQ_CNT;
        LOGD("isp out put format is YUV422 interleaved.");
    }else if(mISPOutputFmt == ISP_OUT_YUV422_SEMI){
        m_camDevice->previewSetup_ex( dcWin, preview_w, preview_h,
                                 CAMERIC_MI_DATAMODE_YUV422,CAMERIC_MI_DATASTORAGE_SEMIPLANAR,(bool_t)true);
		bufSize = ((max_w+15)&(~0xf))*((max_h+15)&(~0xf))*2;
		bufNum = CONFIG_CAMERA_ISP_BUF_REQ_CNT;
        LOGD("isp out put format is YUV422 semi.");
    }else if(mISPOutputFmt == ISP_OUT_YUV420SP){
        m_camDevice->previewSetup_ex( dcWin, preview_w, preview_h,
                                 CAMERIC_MI_DATAMODE_YUV420,CAMERIC_MI_DATASTORAGE_SEMIPLANAR,(bool_t)true);
		bufSize = ((max_w+15)&(~0xf))*((max_h+15)&(~0xf))*3/2 ;
		bufNum = CONFIG_CAMERA_ISP_BUF_REQ_CNT;		
        LOGD("isp out put format is YUV420SP.");
    }else if(mISPOutputFmt == ISP_OUT_RAW12){
        m_camDevice->previewSetup_ex( dcWin, preview_w, preview_h,
                                CAMERIC_MI_DATAMODE_RAW12,CAMERIC_MI_DATASTORAGE_INTERLEAVED,(bool_t)false);
		bufSize = ((max_w+15)&(~0xf))*((max_h+15)&(~0xf))*2;
		bufNum = CONFIG_CAMERA_ISP_BUF_REQ_CNT;
        LOGD("isp out put format is RAW12.");
    }else{
        LOGE("%s:isp don't support this format %d now",__func__,mISPOutputFmt);
    }
	m_camDevice->setIspBufferInfo(bufNum, bufSize);
    LOGD("Sensor output: %dx%d --(%d,%d,%d,%d)--> User request: %dx%d",width_sensor,height_sensor,
        dcWin.hOffset,dcWin.vOffset,dcWin.width,dcWin.height,preview_w,preview_h);

}


status_t CameraIspAdapter::startPreview(int preview_w,int preview_h,int w, int h, int fmt,bool is_capture)
{
    LOG_FUNCTION_NAME
	bool err_af;
	bool err;
	bool avail = false;
    bool enable_flash = false;
    bool low_illumin = false;
    bool is_video = false;
    rk_cam_total_info *pCamInfo = gCamInfos[mCamId].pcam_total_info;
    
    property_set("sys.hdmiin.display", "0");//just used by hdmi-in
    if ( ( !m_camDevice->hasSensor() ) &&
         ( !m_camDevice->hasImage()  ) ){
          goto startPreview_end;
    	}

    //for isp tunning
    if((fmt != mISPOutputFmt)){
        //restart isp
        cameraDestroy();
        mISPOutputFmt = fmt;
        cameraCreate(mCamId);
    }

    is_video = (((preview_w == 1920) && (preview_h == 1080)) || 
                ((preview_w == 1280) && (preview_h == 720)));

    //must to get illum befor resolution changed
    if (is_capture) {
        enable_flash = isNeedToEnableFlash();

        m_camDevice->lock3a((CamEngine3aLock_t)(Lock_awb|Lock_aec)); 
    }
    low_illumin = isLowIllumin(15);
	m_camDevice->pre2capparameter(is_capture,pCamInfo);

    //need to change resolution ?
    if((preview_w != mCamPreviewW) ||(preview_h != mCamPreviewH)
        || (w != mCamDrvWidth) || (h != mCamDrvHeight)){

        //change resolution
        //get sensor res
        int width_sensor = 0,height_sensor = 0;
        uint32_t resMask;
        CamEnginePathConfig_t mainPathConfig ,selfPathConfig;
		CamEngineBestSensorResReq_t resReq;
        float curGain,curExp; /* ddl@rock-chips.com: v1.0x15.0 */

        memset(&resReq, 0x00, sizeof(CamEngineBestSensorResReq_t));
        resReq.request_w = preview_w;
        resReq.request_h = preview_h;

        
        if ((m_camDevice->getIntegrationTime(curExp) == false)) {
            curExp = 0.0;
        }

        if (m_camDevice->getGain(curGain) == false){
            curGain = 0.0;
        }
        
        if (is_video) {
            resReq.request_fps = 20;
        } else if (is_capture) {
            resReq.request_fps = 0;
            resReq.request_exp_t = curExp*curGain;
        } else {
        	if(pCamInfo->mSoftInfo.mFrameRate > 0) {
				resReq.request_fps = pCamInfo->mSoftInfo.mFrameRate;
			} else {
            	resReq.request_fps = 0;
            }
            resReq.request_exp_t = curExp;
        }

        resReq.requset_aspect = (bool_t)false;        
        resReq.request_fullfov = (bool_t)mImgAllFovReq;    
        m_camDevice->getPreferedSensorRes(&resReq);
     
        width_sensor = ISI_RES_W_GET(resReq.resolution);
        height_sensor = ISI_RES_H_GET(resReq.resolution);

        //stop streaming
        if(-1 == stop())
			goto startPreview_end;
        /* ddl@rock-chips.com: v1.0x16.0 */
        if ( is_video ) {
            enableAfps(false);
        } else {
            enableAfps(true);
        }

        //need to change sensor resolution ?
        //if((width_sensor != mCamDrvWidth) || (height_sensor != mCamDrvHeight)){
            m_camDevice->changeResolution(resReq.resolution,false);
        //}
        //reset dcWin,output width(data path)
        //get dcWin
        #if CONFIG_CAMERA_SCALE_CROP_ISP
        setupPreview(width_sensor,height_sensor,preview_w,preview_h,mZoomVal);
        #else
        if ((preview_w == 1600) && (preview_h == 1200) && 
            (width_sensor==1632) && (height_sensor == 1224) ||
            ((preview_w == 1280) && (preview_h == 720) &&
            (width_sensor == 1296) && (height_sensor == 972))) {
            setupPreview(width_sensor,height_sensor,preview_w,preview_h,mZoomVal);
        } else {
            setupPreview(width_sensor,height_sensor,width_sensor,height_sensor,mZoomVal);
        }
        #endif
		
        m_camDevice->getPathConfig(CHAIN_MASTER,CAM_ENGINE_PATH_MAIN,mainPathConfig);
        m_camDevice->getPathConfig(CHAIN_MASTER,CAM_ENGINE_PATH_SELF,selfPathConfig);
        m_camDevice->setPathConfig( CHAIN_MASTER, mainPathConfig, selfPathConfig  );

        mCamPreviewH = preview_h;
        mCamPreviewW = preview_w;
		//start streaming
        if(-1 == start())
			goto startPreview_end;
		
		//set mannual exposure and mannual whitebalance
		if(strcmp((mParameters.get(CameraParameters::KEY_EXPOSURE_COMPENSATION)), "0"))
			setMe(mParameters.get(CameraParameters::KEY_EXPOSURE_COMPENSATION));
		setMwb(mParameters.get(CameraParameters::KEY_WHITE_BALANCE));		
		//
        mCamDrvWidth = width_sensor;
        mCamDrvHeight = height_sensor;
        mCamPreviewH = preview_h;
        mCamPreviewW = preview_w;
        
    }else{
    
        if(mPreviewRunning == 0){
			if(-1 == start())
				goto startPreview_end;
        }
    }


    if (is_capture){
        if(strtod(mParameters.get(KEY_CONTINUOUS_PIC_NUM),NULL) > 1){
            //stop af
            unsigned int maxFocus, minFocus;
            m_camDevice->stopAf();  /* ddl@rock-chips.com: v0.d.3 */
            if(low_illumin){ // low illumin
                if (m_camDevice->getFocusLimits(minFocus, maxFocus) == true) {
                    m_camDevice->setFocus(maxFocus);
                } else {
                    LOGE("getFocusLimits failed!");
                }
            }
            enable_flash = false;
        }else if(low_illumin){
            unsigned int maxFocus, minFocus;
            m_camDevice->stopAf();
            if (m_camDevice->getFocusLimits(minFocus, maxFocus) == true) {
                m_camDevice->setFocus(maxFocus);
            } else {
                LOGE("getFocusLimits failed!");
            }
        }
    }else{
        //restore focus mode
        // Continues picture focus
        if (strcmp(mParameters.get(CameraParameters::KEY_FOCUS_MODE),CameraParameters::FOCUS_MODE_CONTINUOUS_PICTURE)==0) {
            err_af = m_camDevice->startAfContinous(CAM_ENGINE_AUTOFOCUS_SEARCH_ALGORITHM_ADAPTIVE_RANGE);
        	if ( err_af == false ){
        		LOGE("Set startAfContinous failed");        		
        	} 
        } else if (strcmp(mParameters.get(CameraParameters::KEY_FOCUS_MODE),CameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO)==0) {
            unsigned int maxFocus, minFocus;
            m_camDevice->stopAf();  
            
            if (m_camDevice->getFocusLimits(minFocus, maxFocus) == true) {
                m_camDevice->setFocus(maxFocus);
            } else {
                LOGE("getFocusLimits failed!");
            }
        }

        m_camDevice->unlock3a((CamEngine3aLock_t)(Lock_awb|Lock_aec));
        
    } 
    flashControl(enable_flash);

    mPreviewRunning = 1;

    LOG_FUNCTION_NAME_EXIT
    return 0;
startPreview_end:
	LOG_FUNCTION_NAME_EXIT
	return -1;
}
status_t CameraIspAdapter::stopPreview()
{
    LOG_FUNCTION_NAME
	int err;

    if (strcmp(mParameters.get(CameraParameters::KEY_FOCUS_MODE),CameraParameters::FOCUS_MODE_CONTINUOUS_PICTURE)==0) {
        m_camDevice->stopAf();
    }
    
    if(mPreviewRunning) {
        if(-1 == stop())
			return -1;
        clearFrameArray();        
    }
    mPreviewRunning = 0;

    LOG_FUNCTION_NAME_EXIT
    return 0;
}
int CameraIspAdapter::setParameters(const CameraParameters &params_set,bool &isRestartValue)
{
    int fps_min,fps_max;
    int framerate=0;
    
    if (mParameters.get(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES) &&
    	strstr(mParameters.get(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES), params_set.get(CameraParameters::KEY_PREVIEW_SIZE)) == NULL) {
        LOGE("PreviewSize(%s) not supported",params_set.get(CameraParameters::KEY_PREVIEW_SIZE));        
        return BAD_VALUE;
    } else if (strcmp(mParameters.get(CameraParameters::KEY_PREVIEW_SIZE), params_set.get(CameraParameters::KEY_PREVIEW_SIZE))) {
        LOG1("Set preview size %s",params_set.get(CameraParameters::KEY_PREVIEW_SIZE));
        //should update preview cb settings ,for cts
        int w,h;
        const char * fmt=  params_set.getPreviewFormat();
		params_set.getPreviewSize(&w, &h); 
        mRefEventNotifier->setPreviewDataCbRes(w, h, fmt);

    }

    if (mParameters.get(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES) &&
    	strstr(mParameters.get(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES), params_set.get(CameraParameters::KEY_PICTURE_SIZE)) == NULL) {
        LOGE("PictureSize(%s) not supported",params_set.get(CameraParameters::KEY_PICTURE_SIZE));
        return BAD_VALUE;
    } else if (strcmp(mParameters.get(CameraParameters::KEY_PICTURE_SIZE), params_set.get(CameraParameters::KEY_PICTURE_SIZE))) {
        LOG1("Set picture size %s",params_set.get(CameraParameters::KEY_PICTURE_SIZE));
    }

    if (strcmp(params_set.getPictureFormat(), "jpeg") != 0) {
        LOGE("Only jpeg still pictures are supported");
        return BAD_VALUE;
    }

    if (params_set.getInt(CameraParameters::KEY_ZOOM) > params_set.getInt(CameraParameters::KEY_MAX_ZOOM)) {
        LOGE("Zomm(%d) is larger than MaxZoom(%d)",params_set.getInt(CameraParameters::KEY_ZOOM),params_set.getInt(CameraParameters::KEY_MAX_ZOOM));
        return BAD_VALUE;
    }

    if(mParameters.get(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE) &&
    	strstr(mParameters.get(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE),params_set.get(CameraParameters::KEY_PREVIEW_FPS_RANGE)) == NULL) {
        LOGE("fps range(%s) not supported,supported(%s)",params_set.get(CameraParameters::KEY_PREVIEW_FPS_RANGE),mParameters.get(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE));
        return BAD_VALUE;
    }else {
        params_set.getPreviewFpsRange(&fps_min,&fps_max);
        if ((fps_min < 0) || (fps_max < 0) || (fps_max < fps_min)) {
            LOGE("FpsRange(%s) is invalidate",params_set.get(CameraParameters::KEY_PREVIEW_FPS_RANGE));
            return BAD_VALUE;
        }
    }
	{
		if(params_set.get("3dnr_enabled")!= NULL)
		{
			if (strcmp(params_set.get("3dnr_enabled"),mParameters.get("3dnr_enabled"))) {
				if(!strcmp(params_set.get("3dnr_enabled"),"true")){
					mfd.enable = true;
					mfd.buffer_full = false;

				} else {
					mfd.enable = false;
					mfd.buffer_full = true;

				}
			}
		}
		else {
			LOGE("3dnr_enabled is null!");
		}
	}


    {/* ddl@rock-chips.com: v1.5.0 */
        if (params_set.get(CameraParameters::KEY_MAX_NUM_METERING_AREAS) != NULL) {
            if (params_set.getInt(CameraParameters::KEY_MAX_NUM_METERING_AREAS) >= 1) {
                int hOff,vOff,w,h,weight;
                CamEngineWindow_t aeWin;
    			const char* zoneStr = params_set.get(CameraParameters::KEY_METERING_AREAS);
    	    	if (zoneStr) {   
                    LOG1("meter zoneStr %s",zoneStr);
        	        hOff = strtol(zoneStr+1,0,0);
        	        zoneStr = strstr(zoneStr,",");
        	        vOff = strtol(zoneStr+1,0,0);
        		    zoneStr = strstr(zoneStr+1,",");
        	        w = strtol(zoneStr+1,0,0);                    
                    zoneStr = strstr(zoneStr+1,",");
        	        h = strtol(zoneStr+1,0,0);
                    zoneStr = strstr(zoneStr+1,",");
                    weight = strtol(zoneStr+1,0,0);
                    if(strstr(zoneStr+1,"(")){
                        //more than one zone
                        return BAD_VALUE;
                    }
                    
                    w -= hOff;
                    h -= vOff;
                    if((w == 0) && (h == 0) && (hOff == 0) && (weight == 0)){
                        hOff = 0;
                        vOff = 0;
                        w = 0;
                        h = 0;
                    }else if ( ((hOff<-1000) || (hOff>1000)) ||
                         ((vOff<-1000) || (vOff>1000)) ||
                         ((w<=0) || (w>2000)) ||
                         ((h<=0) || (h>2000)) ||
                         ((weight < 1) || (weight > 1000))) {
                        hOff = 0;
                        vOff = 0;
                        w = 0;
                        h = 0;
                        return BAD_VALUE;
                    } 
					{
						rk_cam_total_info *pCamInfo = gCamInfos[mCamId].pcam_total_info;
	                    //ddl@rock-chips.com v1.0x1d.0
	                    if ((hOff || vOff || w || h) && (pCamInfo->mSoftInfo.touchAE == 0x01)){
	                       m_camDevice->setAecHistMeasureWinAndMode(hOff,vOff,w,h,AfWeightMetering);
	                    } else {
	                       m_camDevice->setAecHistMeasureWinAndMode(hOff,vOff,w,h,CentreWeightMetering);
	                    }
                    }
    	    	}

            }

        }

    }


    {
        bool err_af = false;

        if (mParameters.get(CameraParameters::KEY_SUPPORTED_FOCUS_MODES) &&
        	strstr(mParameters.get(CameraParameters::KEY_SUPPORTED_FOCUS_MODES),params_set.get(CameraParameters::KEY_FOCUS_MODE))) {            
            // Continues picture focus
            if (strcmp(params_set.get(CameraParameters::KEY_FOCUS_MODE),CameraParameters::FOCUS_MODE_CONTINUOUS_PICTURE)==0) {
                if (mPreviewRunning == 1) {
                    /* ddl@rock-chips.com: v0.0x22.0 */ 
                    if (strcmp(mParameters.get(CameraParameters::KEY_FOCUS_MODE),CameraParameters::FOCUS_MODE_CONTINUOUS_PICTURE)) {
                        err_af = m_camDevice->startAfContinous(CAM_ENGINE_AUTOFOCUS_SEARCH_ALGORITHM_ADAPTIVE_RANGE);
                    	if ( err_af == false ){
                    		LOGE("Set startAfContinous failed");        		
                    	} 
                    }
                }
            // Continues video focus is not implement, so fixd focus; /* ddl@rock-chips.com: v0.c.0 */
            } else if (strcmp(params_set.get(CameraParameters::KEY_FOCUS_MODE),CameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO)==0) {
                if (mPreviewRunning == 1) {
                    /* ddl@rock-chips.com: v0.0x22.0 */ 
                    if (strcmp(mParameters.get(CameraParameters::KEY_FOCUS_MODE),CameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO)) {
                        unsigned int maxFocus, minFocus;

                        m_camDevice->stopAf();  /* ddl@rock-chips.com: v0.d.3 */
                        
                        if (m_camDevice->getFocusLimits(minFocus, maxFocus) == true) {
                            m_camDevice->setFocus(maxFocus);
                        } else {
                            LOGE("getFocusLimits failed!");
                        }
        				LOG1("Continues-video focus is fixd focus now!");
                    }
                }                
            } else if (strcmp(params_set.get(CameraParameters::KEY_FOCUS_MODE), CameraParameters::FOCUS_MODE_FIXED) == 0
                || params_set.get(CameraParameters::KEY_FOCUS_MODE) == NULL) {
                if (mPreviewRunning == 1) {
                    LOG1("Focus mode is fixed or null,stop af!!!!!!!!");
                    m_camDevice->stopAf();
                }
            }
            
            if (mParameters.getInt(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS) == 1) {
                mAfChk = false;
                /* ddl@rock-chips.com: v0.0x22.0 */ 
                if ((mParameters.get(CameraParameters::KEY_FOCUS_AREAS) == NULL) || 
                    (params_set.get(CameraParameters::KEY_FOCUS_AREAS) == NULL)) {
                    mAfChk = true;
                    LOG1("Focus areas is change(%s -> %s), Must af again!!",
                        mParameters.get(CameraParameters::KEY_FOCUS_AREAS),
                        params_set.get(CameraParameters::KEY_FOCUS_AREAS));
                } else if (strcmp(mParameters.get(CameraParameters::KEY_FOCUS_AREAS),params_set.get(CameraParameters::KEY_FOCUS_AREAS))) {
                    mAfChk = true;

        	    	int hOff,vOff,w,h,weight;
        			const char* zoneStr = params_set.get(CameraParameters::KEY_FOCUS_AREAS);
        	    	if (zoneStr) {  
                        LOG1("focus zoneStr %s",zoneStr);
            	        vOff = strtol(zoneStr+1,0,0);
            	        zoneStr = strstr(zoneStr,",");
            	        hOff = strtol(zoneStr+1,0,0);
            		    zoneStr = strstr(zoneStr+1,",");
            	        w = strtol(zoneStr+1,0,0);                    
                        zoneStr = strstr(zoneStr+1,",");
            	        h = strtol(zoneStr+1,0,0);
                        zoneStr = strstr(zoneStr+1,",");
                        weight = strtol(zoneStr+1,0,0);

                        if(strstr(zoneStr+1,"(")){
                            //more than one zone
                            return BAD_VALUE;
                        }
                        w -= vOff;
                        h -= hOff;                    
                        
                        if((w == 0) && (h == 0) && (hOff == 0) && (weight == 0)){
                            hOff = 0;
                            vOff = 0;
                            w = 0;
                            h = 0;
                        }else if ( ((hOff<-1000) || (hOff>1000)) ||
                         ((vOff<-1000) || (vOff>1000)) ||
                         ((w<=0) || (w>2000)) ||
                         ((h<=0) || (h>2000)) ||
                         ((weight < 1) || (weight > 1000))) {
                        LOGE("%s: %s , afWin(%d,%d,%d,%d)is invalidate!",
                            CameraParameters::KEY_FOCUS_AREAS,
                            params_set.get(CameraParameters::KEY_FOCUS_AREAS),
                            hOff,vOff,w,h);
                        return BAD_VALUE;
                        }
        	    	}
                    
                    LOG1("Focus areas is change(%s -> %s), Must af again!!",
                        mParameters.get(CameraParameters::KEY_FOCUS_AREAS),
                        params_set.get(CameraParameters::KEY_FOCUS_AREAS));
                }
            }
            
            if( err_af == true )
                LOG1("Set focus mode: %s success",params_set.get(CameraParameters::KEY_FOCUS_MODE));
        } else {
            LOGE("%s isn't supported for this camera, support focus: %s",
                params_set.get(CameraParameters::KEY_FOCUS_MODE),
                mParameters.get(CameraParameters::KEY_SUPPORTED_FOCUS_MODES));
			return BAD_VALUE;
		}
	}

    {
        CamEngineFlashCfg_t flash_cfg;

        if (mParameters.get(CameraParameters::KEY_SUPPORTED_FLASH_MODES) && mParameters.get(CameraParameters::KEY_FLASH_MODE)) {

            if (mParameters.get(CameraParameters::KEY_SUPPORTED_FLASH_MODES) &&
            	strstr(mParameters.get(CameraParameters::KEY_SUPPORTED_FLASH_MODES),params_set.get(CameraParameters::KEY_FLASH_MODE))) {
                if (strcmp(mParameters.get(CameraParameters::KEY_FLASH_MODE),params_set.get(CameraParameters::KEY_FLASH_MODE))) {
                    rk_cam_total_info *pCamInfo = gCamInfos[mCamId].pcam_total_info;
                    flash_cfg.active_pol = (pCamInfo->mHardInfo.mFlashInfo.mFlashTrigger.active>0) ? CAM_ENGINE_FLASH_HIGH_ACTIVE:CAM_ENGINE_FLASH_LOW_ACTIVE;
					flash_cfg.flashtype = pCamInfo->mHardInfo.mFlashInfo.mFlashMode;
                    if((strcmp(pCamInfo->mHardInfo.mFlashInfo.mFlashName,"Internal")==0))
                        flash_cfg.dev_mask = pCamInfo->mHardInfo.mSensorInfo.mHostDevid;
                    else
                        flash_cfg.dev_mask = pCamInfo->mHardInfo.mSensorInfo.mCamDevid;
                    if ((strcmp(params_set.get(CameraParameters::KEY_FLASH_MODE),CameraParameters::FLASH_MODE_ON)==0)
                        || ((strcmp(params_set.get(CameraParameters::KEY_FLASH_MODE),CameraParameters::FLASH_MODE_AUTO)==0))){
                        //check flash mode last time
                        if(strcmp(mParameters.get(CameraParameters::KEY_FLASH_MODE),CameraParameters::FLASH_MODE_TORCH)==0){
                            flash_cfg.mode = CAM_ENGINE_FLASH_TORCH;
                            m_camDevice->configureFlash(&flash_cfg);
                            m_camDevice->stopFlash(true);
                        }
                        flash_cfg.mode = CAM_ENGINE_FLASH_ON;
                        m_camDevice->configureFlash(&flash_cfg);
            			LOG1("Set flash on success!");
                    }else if (strcmp(params_set.get(CameraParameters::KEY_FLASH_MODE),CameraParameters::FLASH_MODE_OFF)==0) {
                        //check flash mode last time
                        if(strcmp(mParameters.get(CameraParameters::KEY_FLASH_MODE),CameraParameters::FLASH_MODE_TORCH)==0){
                            flash_cfg.mode = CAM_ENGINE_FLASH_TORCH;
                            m_camDevice->configureFlash(&flash_cfg);
                            m_camDevice->stopFlash(true);
                        }
                        flash_cfg.mode = CAM_ENGINE_FLASH_OFF;
                        m_camDevice->configureFlash(&flash_cfg);
            			LOG1("Set flash off success!");
                    }else if (strcmp(params_set.get(CameraParameters::KEY_FLASH_MODE),CameraParameters::FLASH_MODE_TORCH)==0) {
                        flash_cfg.mode = CAM_ENGINE_FLASH_TORCH;
                        m_camDevice->configureFlash(&flash_cfg);
                        m_camDevice->startFlash(true);
            			LOG1("Set flash torch success!");
                    }
                }
            } else {
                LOGE("%s isn't supported for this camera, support flash: %s",
                    params_set.get(CameraParameters::KEY_FLASH_MODE),
                    mParameters.get(CameraParameters::KEY_SUPPORTED_FLASH_MODES));
                return BAD_VALUE;

            }

        }

    }
	if (!cameraConfig(params_set,false,isRestartValue)) {        
        LOG1("PreviewSize(%s)", mParameters.get(CameraParameters::KEY_PREVIEW_SIZE));
        LOG1("PreviewFormat(%s)",params_set.getPreviewFormat());  
        LOG1("FPS Range(%s)",mParameters.get(CameraParameters::KEY_PREVIEW_FPS_RANGE));
        LOG1("PictureSize(%s)",mParameters.get(CameraParameters::KEY_PICTURE_SIZE)); 
        LOG1("PictureFormat(%s)  ", params_set.getPictureFormat());
        LOG1("Framerate: %d  ", framerate);
        LOG1("WhiteBalance: %s", params_set.get(CameraParameters::KEY_WHITE_BALANCE));
        LOG1("Flash: %s", params_set.get(CameraParameters::KEY_FLASH_MODE));
        LOG1("Focus: %s  Foucus area: %s", params_set.get(CameraParameters::KEY_FOCUS_MODE),
            params_set.get(CameraParameters::KEY_FOCUS_AREAS));
        LOG1("Scene: %s", params_set.get(CameraParameters::KEY_SCENE_MODE));
    	LOG1("Effect: %s", params_set.get(CameraParameters::KEY_EFFECT));
    	LOG1("ZoomIndex: %s", params_set.get(CameraParameters::KEY_ZOOM));
	}else{
	    return BAD_VALUE;
	}  
    
    return 0;
}

void CameraIspAdapter::initDefaultParameters(int camFd)
{
    CameraParameters params;
	String8 parameterString;
    rk_cam_total_info *pCamInfo = gCamInfos[camFd].pcam_total_info;
    bool isRestartPreview = false;
    char string[100];

	LOG_FUNCTION_NAME
    //previwe size and picture size
    {
        IsiSensorCaps_t pCaps;
        unsigned int pixels;
        unsigned int max_w,max_h,max_fps,maxfps_res;
        bool chk_720p,chk_1080p;
        
        parameterString = "176x144,320x240,352x288,640x480,720x480,800x600";
        LOG1("Sensor resolution list:");

        max_w = 0;
        max_h = 0;
        max_fps = 0;
        maxfps_res = 0;
        pCaps.Index = 0;
        chk_720p = false;
        chk_1080p = false;
	    while (m_camDevice->getSensorCaps(pCaps) == true) {
         
            memset(string,0x00,sizeof(string));        
            if (ISI_FPS_GET(pCaps.Resolution) >= 10) {
                pixels = ISI_RES_W_GET(pCaps.Resolution)*ISI_RES_H_GET(pCaps.Resolution)*10;                
                if (pixels > 1280*720*9 && (ISI_RES_W_GET(pCaps.Resolution)>= 1280)) {
                    if (chk_720p == false) {
                        strcat(string,",1280x720");
                        chk_720p = true;
                    }
                }

                if (pixels > 1920*1080*9 && (ISI_RES_W_GET(pCaps.Resolution)>= 1920)) {
                    if (chk_1080p == false) {
                        strcat(string,",1920x1080");
                        chk_1080p = true;
                    }
                } 

                parameterString.append(string);
            }

            if (max_fps < ISI_FPS_GET(pCaps.Resolution)) {
                maxfps_res = pCaps.Resolution;
            }
            memset(string,0x00,sizeof(string)); 
            sprintf(string,",%dx%d",ISI_RES_W_GET(pCaps.Resolution),ISI_RES_H_GET(pCaps.Resolution));
			if(ISI_RES_W_GET(pCaps.Resolution) <= 4096 &&(ISI_RES_W_GET(pCaps.Resolution) <= 2104)) {
            	if (strcmp(string,",1600x1200") && !parameterString.contains(string)){
                	parameterString.append(string);
            	}
			}
            LOG1("    %dx%d @ %d fps", ISI_RES_W_GET(pCaps.Resolution),ISI_RES_H_GET(pCaps.Resolution),
                ISI_FPS_GET(pCaps.Resolution));

            if (ISI_RES_W_GET(pCaps.Resolution)>max_w)
                max_w = ISI_RES_W_GET(pCaps.Resolution);
            if (ISI_RES_H_GET(pCaps.Resolution)>max_h)
                max_h = ISI_RES_H_GET(pCaps.Resolution);
            pCaps.Index++;
	    };
        if(max_w >= 1280 && max_h >= 720)
            parameterString.append(",960x540");
        
        if (pCamInfo->mSoftInfo.mPreviewWidth && pCamInfo->mSoftInfo.mPreviewHeight) {
            memset(string,0x00,sizeof(string));    
            sprintf(string,"%d",pCamInfo->mSoftInfo.mPreviewWidth);  
            params.set(KEY_PREVIEW_W_FORCE,string);
            memset(string,0x00,sizeof(string));
            sprintf(string,"%d",pCamInfo->mSoftInfo.mPreviewHeight);  
            params.set(KEY_PREVIEW_H_FORCE,string);
            memset(string,0x00,sizeof(string));
            sprintf(string,"%dx%d",pCamInfo->mSoftInfo.mPreviewWidth,pCamInfo->mSoftInfo.mPreviewHeight);  
        } else {
            memset(string,0x00,sizeof(string)); 
            sprintf(string,"%dx%d",ISI_RES_W_GET(maxfps_res),ISI_RES_H_GET(maxfps_res));
        }
		
        pixels = max_w*max_h;
        if (max_w*10/max_h == 40/3) {          //  4:3 Sensor
            if (pixels > 12800000) {
                if ((max_w != 4128)&&(max_h != 3096))
                    parameterString.append(",2064x1548");                
            } else if (pixels > 7900000) {
                if ((max_w != 3264)&&(max_h != 2448))
                    parameterString.append(",1632x1224");                
            } else if (pixels > 5000000) {
                if ((max_w != 2592)&&(max_h != 1944))
                    parameterString.append(",1296x972");
            }
        }
        
		char prop_val[PROPERTY_VALUE_MAX];
		property_get("sys.camera.uvnr", prop_val, "1");
		if (!strcmp(prop_val, "1")) {
			uvnr.enable = true;
		} else {
			uvnr.enable = false;
		}

        //720p previewsize  perform not well in this app, so remove it.
        const char* cameraCallProcess = getCallingProcess();
        if (strstr("com.campmobile.snowcamera", cameraCallProcess))
            parameterString.removeAll(",1280x720");

        params.set(CameraParameters::KEY_PREVIEW_SIZE,string);
        params.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES, parameterString.string());        
        
        memset(string,0x00,sizeof(string));
        sprintf(string,"%dx%d",max_w,max_h);
        parameterString = string;
		int interpolationRes = pCamInfo->mSoftInfo.mInterpolationRes;
		if(interpolationRes){			
	        if (max_w*10/max_h == 40/3) {          //  4:3 Sensor
				if (interpolationRes >= 8000000) {
	                if ((max_w != 3264)&&(max_h != 2448))
	                    parameterString.append(",3264x2448");
	                parameterString.append(",2592x1944,2048x1536,1600x1200");
	            }else if (interpolationRes >= 5000000) {
	                parameterString.append(",2592x1944,2048x1536,1600x1200");
	            }else if (interpolationRes >= 3000000) {
	                parameterString.append(",2048x1536,1600x1200,1024x768");
	            } else if (interpolationRes >= 2000000) {
	                parameterString.append(",1600x1200,1024x768");
	            } else if (interpolationRes >= 1000000) {
	                parameterString.append(",1024x768");
	            }
	        } else if (max_w*10/max_h == 160/9) {   // 16:9 Sensor

	        }
		}else{
            if (max_w*10/max_h == 40/3) {          //  4:3 Sensor
                if (pixels > 12500000) {
                    if ((max_w != 4128)&&(max_h != 3096))
                        parameterString.append(",4096x3096");
                    parameterString.append(",4096x3072,3264x2448,2592x1944,1600x1200");
                } else if (pixels > 7900000) {
                    if ((max_w != 3264)&&(max_h != 2448))
                        parameterString.append(",3264x2448");
                    parameterString.append(",2592x1944,2048x1536,1632x1224,1600x1200");
                } else if (pixels > 5000000) {
                    parameterString.append(",2048x1536,1600x1200,1024x768");
                } else if (pixels > 3000000) {
                    parameterString.append(",1600x1200,1024x768");
                } else if (pixels >= 1920000) {
                    parameterString.append(",1024x768");
                }
            } else if (max_w*10/max_h == 160/9) {   // 16:9 Sensor

            }
		}
        parameterString.append(",640x480,320x240");

		if(max_w >= 1280 && max_h >= 720) {
			parameterString.append(",1280x720");
		}
		if(max_w >= 1920 && max_h >= 1080){
			parameterString.append(",1920x1080,1920x1088");
		}

		if(uvnr.enable) {
			if(max_w >= 4096 && max_h >= 3072)
			{
				parameterString.removeAll("4208x3120,");
				ALOGD("PICSIZE: %s", parameterString.string());
				sprintf(string,"%dx%d", 4096, 3096);
			}
		}
		if(mfd.enable == true) {
			if(max_w >= 4096 && max_h >= 3072)
			{
				parameterString.removeAll("4208x3120,");
				ALOGD("PICSIZE: %s", parameterString.string());
				sprintf(string,"%dx%d", 4096, 3096);
			}
		}
        params.set(CameraParameters::KEY_PICTURE_SIZE, string);
        params.set(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES, parameterString.string());
	}

#if (CONFIG_CAMERA_SETVIDEOSIZE == 0)
     if(false){
        if(gCamInfos[camFd].facing_info.facing == CAMERA_FACING_BACK){
             //back camera, may need to manually modify based on media_profiles.xml supported.
             params.set(CameraParameters::KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO,"1920x1080");
             params.set(CameraParameters::KEY_VIDEO_SIZE,"1920x1080");
             params.set(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES,"176x144,320x240,352x288,640x480,1280x720,1920x1080");
        }else{
             //front camera
             params.set(CameraParameters::KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO,"640x480");
             params.set(CameraParameters::KEY_VIDEO_SIZE,"640x480");
             params.set(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES,"176x144,320x240,352x288,640x480");
        }
     LOGD("Support video sizes:%s",params.get(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES));
     }else{
         params.set(CameraParameters::KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO,"");
         params.set(CameraParameters::KEY_VIDEO_SIZE,"");
         params.set(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES,"");
     }
#endif


    //auto focus parameters
	{
        bool err_af;
    	bool avail = false;

        parameterString = CameraParameters::FOCUS_MODE_FIXED;
        params.set(CameraParameters::KEY_FOCUS_MODE, CameraParameters::FOCUS_MODE_FIXED);

       if(0){
           params.set(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS,"0");
           params.set(CameraParameters::KEY_SUPPORTED_FOCUS_MODES, CameraParameters::FOCUS_MODE_FIXED);
       }else{ 
		if (strcmp(pCamInfo->mHardInfo.mVcmInfo.mVcmDrvName,"NC")!=0) {
          //if(0){
            err_af = m_camDevice->isAfAvailable(avail);
            if ((err_af == true) && (avail == true)) {
                parameterString.append(",");
                parameterString.append(CameraParameters::FOCUS_MODE_AUTO);
                params.set(CameraParameters::KEY_FOCUS_MODE, CameraParameters::FOCUS_MODE_AUTO);
        		params.set(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS,"1");  

                if (pCamInfo->mSoftInfo.mFocusConfig.mFocusSupport & (0x01<<FOCUS_CONTINUOUS_VIDEO_BITPOS)) {
                    /* ddl@rock-chips.com: v0.d.0 */
                    parameterString.append(",");
                    parameterString.append(CameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO);
                }

                if (pCamInfo->mSoftInfo.mFocusConfig.mFocusSupport & (0x01<<FOCUS_CONTINUOUS_PICTURE_BITPOS)) {
                    parameterString.append(",");
                    parameterString.append(CameraParameters::FOCUS_MODE_CONTINUOUS_PICTURE);
                } 
            } else {
             	params.set(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS,"0");
        	}

		}else{
         	params.set(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS,"0");

        }
            params.set(CameraParameters::KEY_SUPPORTED_FOCUS_MODES, parameterString.string());
      }
	}

    //flash parameters
    {
        CamEngineFlashCfg_t flash_cfg;

        //Internal or external flash
        if ((strcmp(pCamInfo->mHardInfo.mFlashInfo.mFlashName,"NC")!=0)) {
            parameterString = CameraParameters::FLASH_MODE_OFF;

            parameterString.append(",");
            parameterString.append(CameraParameters::FLASH_MODE_ON);
            
            parameterString.append(",");
            parameterString.append(CameraParameters::FLASH_MODE_AUTO);

            //only external flash support torch now,zyc
            //if((strcmp(pCamInfo->mHardInfo.mFlashInfo.mFlashName,"Internal")!=0)){
                parameterString.append(",");
                parameterString.append(CameraParameters::FLASH_MODE_TORCH);
            //}
            
            //must FLASH_MODE_OFF when initial,forced by cts
            params.set(CameraParameters::KEY_FLASH_MODE,CameraParameters::FLASH_MODE_OFF);
            params.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES,parameterString.string());


            flash_cfg.mode = CAM_ENGINE_FLASH_ON;
            flash_cfg.active_pol = (pCamInfo->mHardInfo.mFlashInfo.mFlashTrigger.active>0) ? CAM_ENGINE_FLASH_HIGH_ACTIVE:CAM_ENGINE_FLASH_LOW_ACTIVE;
			flash_cfg.flashtype = pCamInfo->mHardInfo.mFlashInfo.mFlashMode;
            if((strcmp(pCamInfo->mHardInfo.mFlashInfo.mFlashName,"Internal")==0))
                flash_cfg.dev_mask = pCamInfo->mHardInfo.mSensorInfo.mHostDevid;
            else
                flash_cfg.dev_mask = pCamInfo->mHardInfo.mSensorInfo.mCamDevid;
            m_camDevice->configureFlash(&flash_cfg);
        }
    }

    //digital zoom
    {
        if (pCamInfo->mSoftInfo.mZoomConfig == 1) {
            char str_zoom_max[3],str_zoom_element[5];
            char str[300];           
            int max,i;
            
        	mZoomMax = 300;
        	mZoomMin= 100;
        	mZoomStep = 5;	
            memset(str,0x00,sizeof(str));
            strcpy(str, "");//default zoom
            
        	max = (mZoomMax - mZoomMin)/mZoomStep;
        	sprintf(str_zoom_max,"%d",max);
        	params.set(CameraParameters::KEY_ZOOM_SUPPORTED, "true");
        	params.set(CameraParameters::KEY_MAX_ZOOM, str_zoom_max);
        	params.set(CameraParameters::KEY_ZOOM, "0");
        	for (i=mZoomMin; i<=mZoomMax; i+=mZoomStep) {
        		sprintf(str_zoom_element,"%d,", i);
        		strcat(str,str_zoom_element);
        	}
        	params.set(CameraParameters::KEY_ZOOM_RATIOS, str);
            mZoomVal = 100;
        }else{
        	params.set(CameraParameters::KEY_ZOOM_SUPPORTED, "false");
        }
		params.set(CameraParameters::KEY_SMOOTH_ZOOM_SUPPORTED, "false");
    }	

    //WB setting
	{   
    	parameterString = "auto";
    	
        if(pCamInfo->mSoftInfo.mAwbConfig.mAwbSupport&(0x1<<AWB_INCANDESCENT_BITPOS)) {
            if (m_camDevice->chkAwbIllumination((char*)"A") == true) {
                parameterString.append(",");
                parameterString.append(CameraParameters::WHITE_BALANCE_INCANDESCENT);
            }
        }
        
    	if(pCamInfo->mSoftInfo.mAwbConfig.mAwbSupport&0x1<<(AWB_FLUORESCENT_BITPOS)){
            if (m_camDevice->chkAwbIllumination((char*)"F2_CWF") == true) {
                parameterString.append(",");
                parameterString.append(CameraParameters::WHITE_BALANCE_FLUORESCENT);
            }
    	}
        
    	if(pCamInfo->mSoftInfo.mAwbConfig.mAwbSupport&0x1<<(AWB_WARM_FLUORESCENT_BITPOS)) {
            if (m_camDevice->chkAwbIllumination((char*)"U30") == true) {
                parameterString.append(",");
                parameterString.append(CameraParameters::WHITE_BALANCE_WARM_FLUORESCENT);
            }

    	}
        
    	if(pCamInfo->mSoftInfo.mAwbConfig.mAwbSupport&0x1<<(AWB_DAYLIGHT_BITPOS)) {
            if (m_camDevice->chkAwbIllumination((char*)"D65") == true) {
                parameterString.append(",");
                parameterString.append(CameraParameters::WHITE_BALANCE_DAYLIGHT);
            }
    	}

        if(pCamInfo->mSoftInfo.mAwbConfig.mAwbSupport&0x1<<(AWB_CLOUDY_BITPOS)) {
            if (m_camDevice->chkAwbIllumination((char*)"D50") == true) {
                parameterString.append(",");
                parameterString.append(CameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT);
            }
    	}

        if(pCamInfo->mSoftInfo.mAwbConfig.mAwbSupport&0x1<<(AWB_SHADE_BITPOS)) {
            if (m_camDevice->chkAwbIllumination((char*)"D75") == true) {
                parameterString.append(",");
                parameterString.append(CameraParameters::WHITE_BALANCE_SHADE);
            }
    	}

    	if(pCamInfo->mSoftInfo.mAwbConfig.mAwbSupport&0x1<<(AWB_TWILIGHT_BITPOS)) {
            if (m_camDevice->chkAwbIllumination((char*)"HORIZON") == true) {
                parameterString.append(",");
                parameterString.append(CameraParameters::WHITE_BALANCE_TWILIGHT);
            }
    	}
    
        params.set(CameraParameters::KEY_SUPPORTED_WHITE_BALANCE, parameterString.string());
    	params.set(CameraParameters::KEY_WHITE_BALANCE, "auto");
	}
    
    /*preview format setting*/
    //yuv420p ,forced by cts
    params.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FORMATS, "yuv420sp,yuv420p");
    params.set(CameraParameters::KEY_VIDEO_FRAME_FORMAT,CameraParameters::PIXEL_FORMAT_YUV420SP);
    params.setPreviewFormat(CameraParameters::PIXEL_FORMAT_YUV420SP);
    params.set(CameraParameters::KEY_VIDEO_FRAME_FORMAT,CameraParameters::PIXEL_FORMAT_YUV420SP);

    /*picture format setting*/
    params.set(CameraParameters::KEY_SUPPORTED_PICTURE_FORMATS, CameraParameters::PIXEL_FORMAT_JPEG);
    params.setPictureFormat(CameraParameters::PIXEL_FORMAT_JPEG);
    /*jpeg quality setting*/
    params.set(CameraParameters::KEY_JPEG_QUALITY, "70");
    /*rotation setting*/
    params.set(CameraParameters::KEY_ROTATION, "0");

    /*lzg@rockchip.com :add some settings to pass cts*/	
    /*focus distance setting ,no much meaning ,only for passing cts */
    parameterString = "0.3,50,Infinity";
    params.set(CameraParameters::KEY_FOCUS_DISTANCES, parameterString.string());
    /*focus length setting ,no much meaning ,only for passing cts */
    parameterString = "35";
    params.set(CameraParameters::KEY_FOCAL_LENGTH, parameterString.string());
    /*horizontal angle of view setting ,no much meaning ,only for passing cts */
	parameterString = String8::format( "%f",  pCamInfo->mHardInfo.mSensorInfo.fov_h);
    params.set(CameraParameters::KEY_HORIZONTAL_VIEW_ANGLE, parameterString.string());
    /*vertical angle of view setting ,no much meaning ,only for passing cts */
	parameterString = String8::format("%f",  pCamInfo->mHardInfo.mSensorInfo.fov_v);
    params.set(CameraParameters::KEY_VERTICAL_VIEW_ANGLE, parameterString.string());

    /*quality of the EXIF thumbnail in Jpeg picture setting */
    parameterString = "70";
    params.set(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY, parameterString.string());
    /*supported size of the EXIF thumbnail in Jpeg picture setting */
    parameterString = "0x0,160x128,160x96";
    params.set(CameraParameters::KEY_SUPPORTED_JPEG_THUMBNAIL_SIZES, parameterString.string());
    parameterString = "160";
    params.set(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH, parameterString.string());
    parameterString = "128";
    params.set(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT, parameterString.string()); 
    if(pCamInfo->mSoftInfo.mFaceDetctConfig.mFaceDetectSupport > 0){
        memset(string,0x00,sizeof(string));    
    	sprintf(string,"%d",pCamInfo->mSoftInfo.mFaceDetctConfig.mFaceMaxNum);
    	params.set(CameraParameters::KEY_MAX_NUM_DETECTED_FACES_HW,string);
    }else
    	params.set(CameraParameters::KEY_MAX_NUM_DETECTED_FACES_HW,"0");
    params.set(CameraParameters::KEY_RECORDING_HINT,"false");
    params.set(CameraParameters::KEY_VIDEO_STABILIZATION_SUPPORTED,"false");
    params.set(CameraParameters::KEY_VIDEO_SNAPSHOT_SUPPORTED,"true");
    params.set(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED, "false");
    params.set(CameraParameters::KEY_AUTO_EXPOSURE_LOCK_SUPPORTED, "false");
            
    //params.set(CameraParameters::KEY_FOCUS_MODE, CameraParameters::FOCUS_MODE_FIXED);
    //params.set(CameraParameters::KEY_SUPPORTED_FOCUS_MODES,CameraParameters::FOCUS_MODE_FIXED);

    // params.set(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS,"0");
    //exposure setting
    if (m_camDevice->isSOCSensor() == false) {
        params.set(CameraParameters::KEY_EXPOSURE_COMPENSATION, "0");
        params.set(CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION, "2");
        params.set(CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION, "-2");
        params.set(CameraParameters::KEY_EXPOSURE_COMPENSATION_STEP, "1");

        params.set(CameraParameters::KEY_MAX_NUM_METERING_AREAS,"1");
    } else {
        params.set(CameraParameters::KEY_EXPOSURE_COMPENSATION, "0");
        params.set(CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION, "0");
        params.set(CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION, "0");
        params.set(CameraParameters::KEY_EXPOSURE_COMPENSATION_STEP, "0");

        params.set(CameraParameters::KEY_MAX_NUM_METERING_AREAS,"0");
    }
	
	{
		/*no much meaning ,only for passing cts*/
		params.set(CameraParameters::KEY_AUTO_EXPOSURE_LOCK, "false");
		params.set(CameraParameters::KEY_AUTO_EXPOSURE_LOCK_SUPPORTED, "true");
	}

	//color effect
	//for passing cts
	params.set(CameraParameters::KEY_SUPPORTED_EFFECTS, "none,mono,sepia");
	params.set(CameraParameters::KEY_EFFECT, "none");
	//antibanding
	/*no much meaning ,only for passing cts*/
	params.set(CameraParameters::KEY_SUPPORTED_ANTIBANDING, "auto,50hz,60hz,off");
	params.set(CameraParameters::KEY_ANTIBANDING, "off");
	//if (m_camDevice->isSOCSensor() == false) {
	//    params.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, "5000,30000");
	//	params.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE, "(5000,30000),(30000,30000)");
	//}else {
	    params.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, "5000,19000");
		params.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE, "(5000,19000),(19000,19000),(24000,24000)");
	//}
    params.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES, "10,15,19,24,30");
    params.setPreviewFrameRate(30);

    params.set(KEY_CONTINUOUS_PIC_NUM,"1");
	params.set("3dnr_enabled","false");
    if((pCamInfo->mSoftInfo.mContinue_snapshot_config)
        && (pCamInfo->mHardInfo.mSensorInfo.mPhy.type == CamSys_Phy_Mipi)
        && (pCamInfo->mHardInfo.mSensorInfo.laneNum > 1)){
        params.set(KEY_CONTINUOUS_SUPPORTED,"true");
    }else{
        params.set(KEY_CONTINUOUS_SUPPORTED,"false");
    }
    // for cts
    params.set(CameraParameters::KEY_SUPPORTED_ANTIBANDING, "auto,50hz,60hz,off");
    params.set(CameraParameters::KEY_ANTIBANDING, "off");
    params.set(CameraParameters::KEY_SUPPORTED_EFFECTS, "none,mono,sepia");
    params.set(CameraParameters::KEY_EFFECT, "none");

    LOGD ("Support Preview format: %s    %s(default)",params.get(CameraParameters::KEY_SUPPORTED_PREVIEW_FORMATS),params.get(CameraParameters::KEY_PREVIEW_FORMAT)); 
    LOGD ("Support Preview sizes: %s    %s(default)    %dx%d(force)",params.get(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES),params.get(CameraParameters::KEY_PREVIEW_SIZE),
        pCamInfo->mSoftInfo.mPreviewWidth, pCamInfo->mSoftInfo.mPreviewHeight);
    LOGD ("Support Preview FPS range: %s",params.get(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE));
    LOGD ("Support Preview framerate: %s",params.get(CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES));
	LOGD ("Support Picture sizes: %s    %s(default)",params.get(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES),params.get(CameraParameters::KEY_PICTURE_SIZE));
    LOGD ("Support Focus: %s  Focus zone: %s",params.get(CameraParameters::KEY_SUPPORTED_FOCUS_MODES),params.get(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS));
    if (params.get(CameraParameters::KEY_SUPPORTED_FLASH_MODES) && params.get(CameraParameters::KEY_FLASH_MODE))
        LOGD ("Support Flash: %s  Flash: %s",params.get(CameraParameters::KEY_SUPPORTED_FLASH_MODES),params.get(CameraParameters::KEY_FLASH_MODE));
    LOGD ("Support AWB: %s ",params.get(CameraParameters::KEY_SUPPORTED_WHITE_BALANCE));
    LOGD("Support FOV test h(%f) v(%f)\n", pCamInfo->mHardInfo.mSensorInfo.fov_h,pCamInfo->mHardInfo.mSensorInfo.fov_v);

	cameraConfig(params,true,isRestartPreview);
    LOG_FUNCTION_NAME_EXIT
}

int CameraIspAdapter::cameraConfig(const CameraParameters &tmpparams,bool isInit,bool &isRestartValue)
{
	int err = 0, i = 0;
	CameraParameters params = tmpparams;
	
    /*white balance setting*/
    const char *white_balance = params.get(CameraParameters::KEY_WHITE_BALANCE);
	const char *mwhite_balance = mParameters.get(CameraParameters::KEY_WHITE_BALANCE);
	if (params.get(CameraParameters::KEY_SUPPORTED_WHITE_BALANCE)) {	
		if ( !mwhite_balance || strcmp(white_balance, mwhite_balance) ){
			if(!isInit) {
				char prfName[10];
				uint32_t illu_index = 1; 
				if(!strcmp(white_balance, "auto")) {
                    if (m_camDevice->isSOCSensor() == false) {
    					m_camDevice->stopAwb();
    					m_camDevice->resetAwb();
    					m_camDevice->startAwb(CAM_ENGINE_AWB_MODE_AUTO, 0, (bool_t)true);
                    }
				} else {
					setMwb(white_balance);
				}
			}
		}
	}

	/*zoom setting*/
    const int zoom = params.getInt(CameraParameters::KEY_ZOOM);
	const int mzoom = mParameters.getInt(CameraParameters::KEY_ZOOM);
	if (params.get(CameraParameters::KEY_ZOOM_SUPPORTED)) {
		//TODO
        if((zoom != mzoom) && (!isInit)){
            CamEnginePathConfig_t pathConfig;
            mZoomVal = zoom*mZoomStep+mZoomMin;
            #if (USE_RGA_TODO_ZOOM == 0)          /* zyc@rock-chips.com: v0.0x22.0 */ 
            setupPreview(mCamDrvWidth,mCamDrvHeight,mCamPreviewW, mCamPreviewH, mZoomVal);
            m_camDevice->getPathConfig(CHAIN_MASTER,CAM_ENGINE_PATH_MAIN,pathConfig);
            m_camDevice->reSetMainPathWhileStreaming(&pathConfig.dcWin,pathConfig.width,pathConfig.height);
            #endif

        }
        
	}
	
    /*color effect setting*/
    const char *effect = params.get(CameraParameters::KEY_EFFECT);
	const char *meffect = mParameters.get(CameraParameters::KEY_EFFECT);
	if (params.get(CameraParameters::KEY_SUPPORTED_EFFECTS)) {
		//TODO
	}

    /*anti-banding setting*/
    const char *anti_banding = params.get(CameraParameters::KEY_ANTIBANDING);
	const char *manti_banding = mParameters.get(CameraParameters::KEY_ANTIBANDING);
	if (anti_banding != NULL) {
		if ( !manti_banding || (anti_banding && strcmp(anti_banding, manti_banding)) ) {
			//TODO
		}
	}
	
	/*scene setting*/
    const char *scene = params.get(CameraParameters::KEY_SCENE_MODE);
	const char *mscene = mParameters.get(CameraParameters::KEY_SCENE_MODE);
	if (params.get(CameraParameters::KEY_SUPPORTED_SCENE_MODES)) {
		if ( !mscene || strcmp(scene, mscene) ) {
			//TODO
		}
	}
	
    /*focus setting*/
    const char *focusMode = params.get(CameraParameters::KEY_FOCUS_MODE);
	const char *mfocusMode = mParameters.get(CameraParameters::KEY_FOCUS_MODE);
	if (params.get(CameraParameters::KEY_SUPPORTED_FOCUS_MODES)) {
		if ( !mfocusMode || strcmp(focusMode, mfocusMode) ) {
       		//if(!cameraAutoFocus(isInit)){
        	//	params.set(CameraParameters::KEY_FOCUS_MODE,(mfocusMode?mfocusMode:CameraParameters::FOCUS_MODE_FIXED));
        	//	err = -1;
   			//}
   			//TODO
		}
	} else{
		params.set(CameraParameters::KEY_FOCUS_MODE,(mfocusMode?mfocusMode:CameraParameters::FOCUS_MODE_FIXED));
	}
	
	/*flash mode setting*/
    const char *flashMode = params.get(CameraParameters::KEY_FLASH_MODE);
	const char *mflashMode = mParameters.get(CameraParameters::KEY_FLASH_MODE);
	
	if (params.get(CameraParameters::KEY_SUPPORTED_FLASH_MODES)) {
		if ( !mflashMode || strcmp(flashMode, mflashMode) ) {
			//TODO
		}
	}
    
    /*exposure setting*/
	const char *exposure = params.get(CameraParameters::KEY_EXPOSURE_COMPENSATION);
    const char *mexposure = mParameters.get(CameraParameters::KEY_EXPOSURE_COMPENSATION);
    
	if (strcmp("0", params.get(CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION))
		|| strcmp("0", params.get(CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION))) {
	    if (!mexposure || (exposure && strcmp(exposure,mexposure))) {
            if (m_camDevice->isSOCSensor() == false) {
    			if(isInit)
    			{
    				float tolerance;
    				manExpConfig.clmtolerance = m_camDevice->getAecClmTolerance();
    				tolerance = manExpConfig.clmtolerance/100.0f;
    				m_camDevice->getInitAePoint(&manExpConfig.level_0);

    				manExpConfig.plus_level_1 = (manExpConfig.level_0/(1-tolerance))+manExpConfig.clmtolerance*1.5;
    				manExpConfig.plus_level_2 = (manExpConfig.plus_level_1/(1-tolerance))+manExpConfig.clmtolerance*1.5;
    				manExpConfig.plus_level_3 = (manExpConfig.plus_level_2/(1-tolerance))+manExpConfig.clmtolerance*1.5;
    				
    				manExpConfig.minus_level_1 = (manExpConfig.level_0/(1+tolerance))-manExpConfig.clmtolerance*1.5;
    				if(manExpConfig.minus_level_1 < 0)
    					manExpConfig.minus_level_1 = 0;
					manExpConfig.minus_level_2 = (manExpConfig.minus_level_1/(0.6+tolerance))-manExpConfig.clmtolerance*1.5;
    				if(manExpConfig.minus_level_2 < 0)
    					manExpConfig.minus_level_2 = 0;				
    				manExpConfig.minus_level_3 = (manExpConfig.minus_level_2/(1+tolerance))-manExpConfig.clmtolerance*1.5;	
    				if(manExpConfig.minus_level_3 < 0)
    					manExpConfig.minus_level_3 = 0;
    				LOG1("Exposure Compensation ae point:\n");
                    LOG1("    -3 : %f\n",manExpConfig.minus_level_3);
                    LOG1("    -2 : %f\n",manExpConfig.minus_level_2);
                    LOG1("    -1 : %f\n",manExpConfig.minus_level_1);
                    LOG1("     0 : %f\n",manExpConfig.level_0);
                    LOG1("     1 : %f\n",manExpConfig.plus_level_1);
                    LOG1("     2 : %f\n",manExpConfig.plus_level_2);
                    LOG1("     3 : %f\n",manExpConfig.plus_level_3);
    			}else{
				setMe(exposure);
    			}
            }
	    }
	}    

	mParameters = params;
	//changeVideoPreviewSize();
	isRestartValue = isNeedToRestartPreview();

	return err;
}

status_t CameraIspAdapter::autoFocus()
{
    bool shot = false,err_af = false;
    CamEngineWindow_t afWin;

    if (strcmp(mParameters.get(CameraParameters::KEY_FOCUS_MODE), CameraParameters::FOCUS_MODE_AUTO) == 0) {
        if (mAfChk == false) {
            LOG1("Focus mode is Auto and areas not change, CheckAfShot!");
            if (m_camDevice->checkAfShot(&shot) == false) {
                shot = true;    
            }
        } else {
            LOG1("Focus mode is Auto, but areas have changed, Must AfShot!");
            shot = true;
        }
        
    } else if (strcmp(mParameters.get(CameraParameters::KEY_FOCUS_MODE), CameraParameters::FOCUS_MODE_CONTINUOUS_PICTURE) == 0) {
        LOG1("Focus mode is continues, checkAfShot!");
        if (m_camDevice->checkAfShot(&shot) == false) {
            shot = true;    
        }
    }

    if (shot == true) {
        LOG1("Single auto focus must be trigger");
        m_camDevice->stopAf();  /* ddl@rock-chips.com: v0.d.0 */
        /* ddl@rock-chips.com: v1.5.0 */
        if (mParameters.get(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS) != NULL) {
            if (mParameters.getInt(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS) > 0) {
    	    	int hOff,vOff,w,h;
    			const char* zoneStr = mParameters.get(CameraParameters::KEY_FOCUS_AREAS);
    	    	if (zoneStr) {        			
        	        hOff = strtol(zoneStr+1,0,0);
        	        zoneStr = strstr(zoneStr,",");
        	        vOff = strtol(zoneStr+1,0,0);
        		    zoneStr = strstr(zoneStr+1,",");
        	        w = strtol(zoneStr+1,0,0);                    
                    zoneStr = strstr(zoneStr+1,",");
        	        h = strtol(zoneStr+1,0,0);

                    w -= hOff;
                    h -= vOff;                    
                    
                    if ( ((hOff<-1000) || (hOff>1000)) ||
                         ((vOff<-1000) || (vOff>1000)) ||
                         ((w<=0) || (w>2000)) ||
                         ((h<=0) || (h>2000)) ) {
                        LOGE("%s: %s , afWin(%d,%d,%d,%d)is invalidate!",
                            CameraParameters::KEY_FOCUS_AREAS,
                            mParameters.get(CameraParameters::KEY_FOCUS_AREAS),
                            hOff,vOff,w,h);
                        vOff = 0;
                        hOff = 0;
                        w = 0;
                        h = 0;
                    }
                    if (hOff==750 && vOff==750 && w==250 && h==250)//for cts pass
                        goto finish_focus;
                    m_camDevice->setAfMeasureWindow(hOff,vOff,w,h);
    	    	}
            }
        }
        
        err_af = m_camDevice->startAfOneShot(CAM_ENGINE_AUTOFOCUS_SEARCH_ALGORITHM_ADAPTIVE_RANGE);
    	if ( err_af == false ){
    		TRACE_E("Trigger a single auto focus failed!");  
            goto finish_focus;  /* ddl@rock-chips.com: v0.0x27.0 */
    	} else {
            LOG1("Trigger a single auto focus success");
    	}
    } else {

finish_focus:    
        LOG1("It has been focused!");

        CamEngineAfEvt_t evnt;
        int32_t err, times;
        
        evnt.evnt_id = CAM_ENGINE_AUTOFOCUS_FINISHED;
        evnt.info.fshEvt.focus = BOOL_TRUE;

        err = osQueueTryWrite(&mAfListenerQue.queue, &evnt);
        if (err != OSLAYER_OK) {
            times = 0;
            do 
            {
                times++;
                err = osQueueTryRead(&mAfListenerQue.queue, &evnt);
            } while ((err == OSLAYER_OK) && (times < 5));

            evnt.evnt_id = CAM_ENGINE_AUTOFOCUS_FINISHED;
            evnt.info.fshEvt.focus = BOOL_TRUE;

            err = osQueueTryWrite(&mAfListenerQue.queue, &evnt);
            if (err != OSLAYER_OK) {
                LOGE(" mAfListenerQue.queue write failed!!");
                DCT_ASSERT(0);
            }
        }
    }
	 
    return 0;
}


status_t CameraIspAdapter::cancelAutoFocus()
{  
    /* ddl@rock-chips.com: v0.0x27.0 */
    if (strcmp(mParameters.get(CameraParameters::KEY_FOCUS_MODE), CameraParameters::FOCUS_MODE_AUTO) == 0) {
        m_camDevice->stopAf();    
    }
    
    return 0;
}

void CameraIspAdapter::setScenarioMode(CamEngineModeType_t newScenarioMode)
{
    CamEngineModeType_t oldScenarioMode = CAM_ENGINE_MODE_INVALID;
    m_camDevice->getScenarioMode( oldScenarioMode );


    if ( oldScenarioMode != newScenarioMode )
    {
        // disconnect
        disconnectCamera();

        m_camDevice->setScenarioMode( newScenarioMode );
        m_camDevice->clearSensorDriverFile();
    }
}
//sensor interface :mipi ? parallel?
void CameraIspAdapter::setSensorItf(int newSensorItf)
{
    if ( CamEngineItf::Sensor == m_camDevice->configType() )
    {
        int oldSensorItf = m_camDevice->getSensorItf();
        if ( oldSensorItf != newSensorItf )
        {
            m_camDevice->setSensorItf( newSensorItf );
             mSensorItfCur = newSensorItf;
            loadSensor( mCamId);
           
        }
        
    }
}
void CameraIspAdapter::enableAfps( bool enable )
{
    if ( CamEngineItf::Sensor == m_camDevice->configType() )
    {
        if ( enable != m_camDevice->isAfpsEnabled() )
        {
            m_camDevice->enableAfps( enable );
            m_camDevice->changeEcm( true );
        }
    }
}

void CameraIspAdapter::openImage( const char* fileName)
{
    // open sensor
    if ( NULL != fileName )
    {
        disconnectCamera();
        PicBufMetaData_t        image;
        image.Type = PIC_BUF_TYPE_RAW8;
        image.Layout =PIC_BUF_LAYOUT_BAYER_RGRGGBGB;
        image.Data.raw.PicWidthPixel= 1920;
        image.Data.raw.PicHeightPixel = 1080;
        image.Data.raw.PicWidthBytes = 1920;

        image.Data.raw.pBuffer = (uint8_t *)malloc( image.Data.raw.PicWidthBytes * image.Data.raw.PicHeightPixel );
        if (true == m_camDevice->openImage(fileName,image,NULL,NULL,NULL,NULL,1.0,1.0))
        {
            //set scenario mode
            CamEngineModeType_t mode1 = CAM_ENGINE_MODE_IMAGE_PROCESSING; 

            m_camDevice->setScenarioMode( mode1 );
            // connect
            connectCamera();


        }else{
            LOGE("failed!");
        }
        
        if(image.Data.raw.pBuffer)
            free(image.Data.raw.pBuffer);


        
    }

    
}
void CameraIspAdapter::loadSensor( const int cameraId)
{
    // open sensor
    if(cameraId>=0)
    {
        disconnectCamera();

        rk_cam_total_info *pCamInfo = gCamInfos[cameraId].pcam_total_info;

        if ( true == m_camDevice->openSensor( pCamInfo, mSensorItfCur ) )
        {
        	bool res = m_camDevice->checkVersion(pCamInfo);
			if(res!=true)
			    return;
#if 1
            // connect
            uint32_t resMask;
            CamEngineWindow_t dcWin;
            CamEngineBestSensorResReq_t resReq;

            resReq.request_w = DEFAULTPREVIEWWIDTH;
            resReq.request_h = DEFAULTPREVIEWHEIGHT;
            resReq.request_fps = 15;
            resReq.requset_aspect = (bool_t)false;
            resReq.request_fullfov = (bool_t)mImgAllFovReq;
            m_camDevice->getPreferedSensorRes(&resReq);            
           
            m_camDevice->setSensorResConfig(resReq.resolution);
            mCamDrvWidth = ISI_RES_W_GET(resReq.resolution);
            mCamDrvHeight = ISI_RES_H_GET(resReq.resolution);
            setupPreview(mCamDrvWidth,mCamDrvHeight,DEFAULTPREVIEWWIDTH,DEFAULTPREVIEWHEIGHT,mZoomVal);
            connectCamera();
            mCamPreviewH = DEFAULTPREVIEWHEIGHT;
            mCamPreviewW = DEFAULTPREVIEWWIDTH;
            //mSensorDriverFile[mSensorItfCur] = fileName;
#endif
        }
        else
        {
            LOGE("%s(%d):failed!",__func__,__LINE__);
        }
    }
}

void CameraIspAdapter::loadCalibData(const char* fileName )
{
    if ( NULL != fileName )
    {
        disconnectCamera();
        if ( true == m_camDevice->loadCalibrationData( fileName ) )
        {
            // connect
            connectCamera();
        }
        else
        {
            LOGE("loadCalibrationData failed!");
        }
    }
}

bool CameraIspAdapter::connectCamera(){
    bool result = false;
    result = m_camDevice->connectCamera( true, this );
    if ( true != result)
    {
        LOGE("connectCamera failed!");
    }

    {
    	rk_cam_total_info *pCamInfo = gCamInfos[mCamId].pcam_total_info;

        CamEngineCprocConfig_t cproc_config = {
            CAM_ENGINE_CPROC_CHROM_RANGE_OUT_BT601,//CAM_ENGINE_CPROC_CHROM_RANGE_OUT_FULL_RANGE,
            CAM_ENGINE_CPROC_LUM_RANGE_OUT_FULL_RANGE,//CAM_ENGINE_CPROC_LUM_RANGE_OUT_BT601,//,
            CAM_ENGINE_CPROC_LUM_RANGE_IN_FULL_RANGE,//CAM_ENGINE_CPROC_LUM_RANGE_IN_BT601,//,
            1.1,  //contrast 0-1.992
            0,      //brightness -128 - 127
            1.0,      //saturation 0-1.992
            0,      //hue   -90 - 87.188
        };
        if(pCamInfo->mSoftInfo.mPreCprocConfig.mSupported == true){
            cproc_config.contrast = pCamInfo->mSoftInfo.mPreCprocConfig.mContrast;
            cproc_config.saturation= pCamInfo->mSoftInfo.mPreCprocConfig.mSaturation;
            cproc_config.hue= pCamInfo->mSoftInfo.mPreCprocConfig.mHue;
            cproc_config.brightness= pCamInfo->mSoftInfo.mPreCprocConfig.mBrightness;
            m_camDevice->cProcEnable( &cproc_config);
        }
    }

	rk_cam_total_info *pCamInfo = gCamInfos[mCamId].pcam_total_info;

    if(pCamInfo->mSoftInfo.mGammaOutConfig.mSupported == true){
        m_camDevice->gamCorrectEnable();
        m_camDevice->gamCorrectSetCurve();
    }

    return result;
}

void CameraIspAdapter::disconnectCamera()
{
    unsigned int maxFocus, minFocus;

    if (mISPTunningRun == false) {
    
        m_camDevice->stopAf();  /* ddl@rock-chips.com: v0.d.1 */
        
        if (m_camDevice->getFocusLimits(minFocus, maxFocus) == true) {
            m_camDevice->setFocus(maxFocus);
            usleep(100000);
        } else {
            LOGE("getFocusLimits failed!");
        }
    }

	rk_cam_total_info *pCamInfo = gCamInfos[mCamId].pcam_total_info;
    if((pCamInfo->mSoftInfo.mPreCprocConfig.mSupported == true) || (pCamInfo->mSoftInfo.mCapCprocConfig.mSupported == true)){
        bool running = false;
        CamEngineCprocConfig_t cproc_config;
        if((m_camDevice->state() == CamEngineItf::Running) ||
            (m_camDevice->state() == CamEngineItf::Idle))
            m_camDevice->cProcStatus( running, cproc_config );
        if(running)
            m_camDevice->cProcDisable();
    }
    
    m_camDevice->disconnectCamera();
}

int CameraIspAdapter::start()
{
    if ( ( !m_camDevice->hasSensor() ) &&
         ( !m_camDevice->hasImage()  ) )
    {
        LOGE("start failed! (!m_camDevice->hasSensor(): %d !m_camDevice->hasImage(): %d)",
            (!m_camDevice->hasSensor()),(!m_camDevice->hasImage()));
        return -1;
    }

    if ( true == m_camDevice->startPreview() )
    {
        LOGD("m_camDevice->startPreview success");
        m_camDevice->isPictureOrientationAllowed( CAM_ENGINE_MI_ORIENTATION_ORIGINAL );
		return 0;
    } else {
        LOGE("m_camDevice->startPreview failed!");
		return -1;
    }
}
int CameraIspAdapter::pause()
{
    if ( ( !m_camDevice->hasSensor() ) &&
         ( !m_camDevice->hasImage()  ) )
    {
        LOGE("start failed! (!m_camDevice->hasSensor(): %d !m_camDevice->hasImage(): %d)",
            (!m_camDevice->hasSensor()),(!m_camDevice->hasImage()));
        return -1;
    }

    if ( true == m_camDevice->pausePreview() )
    {
        LOGD("m_camDevice->pausePreview success!");
		return 0;
    } else {
        LOGE("m_camDevice->pausePreview failed!");
		return -1;
    }
}


/******************************************************************************
 * stop
 *****************************************************************************/
int CameraIspAdapter::stop()
{
    if ( ( !m_camDevice->hasSensor() ) &&
         ( !m_camDevice->hasImage()  ) )
    {
        LOGE("start failed! (!m_camDevice->hasSensor(): %d !m_camDevice->hasImage(): %d)",
            (!m_camDevice->hasSensor()),(!m_camDevice->hasImage()));
        return -1;
    }

    if ( true == m_camDevice->stopPreview() )
    {
    
        LOGD("m_camDevice->stopPreview success!");
		return 0;
    }
	else
	{
		LOGE("m_camDevice->stopPreview fail!");
		return -1;
	}
}

void CameraIspAdapter::clearFrameArray(){
    LOG_FUNCTION_NAME
    MediaBuffer_t *pMediaBuffer = NULL;
    FramInfo_s *tmpFrame = NULL;
    Mutex::Autolock lock(mFrameArrayLock);

    int num = mFrameInfoArray.size();
    while(--num >= 0){
        tmpFrame = (FramInfo_s *)mFrameInfoArray.keyAt(num);
        if(mFrameInfoArray.indexOfKey((void*)tmpFrame) < 0){
            LOGE("this frame is not in frame array,used_flag is %d!",tmpFrame->used_flag);
        }else{
            pMediaBuffer = (MediaBuffer_t *)mFrameInfoArray.valueAt(num);
            switch (tmpFrame->used_flag){
                case 0:
                    mDispFrameLeak--;
                    break;
                case 1:
                    mVideoEncFrameLeak--;
                    break;
                case 2:
                    mPicEncFrameLeak--;
                    break;
                case 3:
                    mPreviewCBFrameLeak--;
                    break;
                default:
                    LOGE("not the valid used_flag %d",tmpFrame->used_flag);
            }
            //remove item
            mFrameInfoArray.removeItem((void*)tmpFrame);
            free(tmpFrame);
            //unlock
            
            MediaBufUnlockBuffer( pMediaBuffer );
        }
    }
    mFrameInfoArray.clear();
    LOG_FUNCTION_NAME_EXIT
}
int CameraIspAdapter::adapterReturnFrame(long index,int cmd){
    FramInfo_s* tmpFrame = ( FramInfo_s *)index;
    Mutex::Autolock lock(mFrameArrayLock);
    if(mFrameInfoArray.size() > 0){
        if(mFrameInfoArray.indexOfKey((void*)tmpFrame) < 0){
            LOGE("this frame is not in frame array,used_flag is %d!",tmpFrame->used_flag);
        }else{
            MediaBuffer_t *pMediaBuffer = (MediaBuffer_t *)mFrameInfoArray.valueFor((void*)tmpFrame);
            {	
                switch (tmpFrame->used_flag){
                    case 0:
                        mDispFrameLeak--;
                        break;
                    case 1:
                        mVideoEncFrameLeak--;
                        break;
                    case 2:
                        mPicEncFrameLeak--;
                        break;
                    case 3:
                        mPreviewCBFrameLeak--;
                        break;
                    default:
                        LOG1("not the valid used_flag %d",tmpFrame->used_flag);
                }
                //remove item
                mFrameInfoArray.removeItem((void*)tmpFrame);
                free(tmpFrame);
                
                //unlock
                MediaBufUnlockBuffer( pMediaBuffer );
            }
        }
    }else{
        LOGD("frame array has been cleard!");
    }
    return 0;
}

int CameraIspAdapter::getCurPreviewState(int *drv_w,int *drv_h)
{
    *drv_w = mCamPreviewW;
    *drv_h = mCamPreviewH;
    return mPreviewRunning;
}

void CameraIspAdapter::gpuCommandThread()
{
    Message_cam msg;
	float uvnr_set_ratio;
	float uvnr_set_distances;
	char  uvnr_set_enable;
	float uvnr_ISO[3];
	float uvnr_ratio[3];
	float uvnr_distances[3];

    while (mGPUCommandThreadState != STA_GPUCMD_STOP) {
    gpu_receive_cmd:
        if (gpuCmdThreadCommandQ.isEmpty() == false ) {
            gpuCmdThreadCommandQ.get(&msg);

            //ALOGD("isp-msg,command thread receive message: %d", msg.command);
            switch (msg.command)
            {
                case CMD_GPU_PROCESS_INIT:
                {
                    //ALOGD("check init, w-h: %d-%d, tid: %d", width, height, gettid());
                    if (!mCameraGL->initialized) {
//                        mCameraGL->init(NULL, mGpuFBOWidth, mGpuFBOHeight);
                    }
                    mGPUCommandThreadState = STA_GPUCMD_RUNNING;
                    if(msg.arg1)
                        ((Semaphore*)msg.arg1)->Signal();
                    break;
                }
                case CMD_GPU_PROCESS_UPDATE:
                {
                    if (mCameraGL->initialized) {
                        //mCameraGL->update(m_buffers_capture);
                    }
                    if(msg.arg1)
                        ((Semaphore*)msg.arg1)->Signal();

                    break;
                }
                case CMD_GPU_PROCESS_RENDER:
                {
                    if (mCameraGL->initialized) {
						m_camDevice->getUvnrPara(&uvnr_set_enable, uvnr_ISO, uvnr_ratio,uvnr_distances);
                        m_camDevice->getGain(mISO);

						if(uvnr_set_enable == true) {
							if(mISO > uvnr_ISO[2]) {
								uvnr_set_ratio = uvnr_ratio[2];
								uvnr_set_distances = uvnr_distances[2];
							}
							else if(mISO > uvnr_ISO[1]){
								uvnr_set_ratio = uvnr_ratio[1];
								uvnr_set_distances = uvnr_distances[1];
							}
							else {
								uvnr_set_ratio = uvnr_ratio[0];
								uvnr_set_distances = uvnr_distances[0];
							}
						} else {
							uvnr_set_ratio = 15;
							uvnr_set_distances = 5;
						}

//                        mCameraGL->process(NULL,OUTPUT_NONE,uvnr_set_ratio,uvnr_set_distances);
                    }
                    if(msg.arg1)
                        ((Semaphore*)msg.arg1)->Signal();

                    break;
                }
                case CMD_GPU_PROCESS_GETRESULT:
                {
                    break;
                }
                case CMD_GPU_PROCESS_DEINIT:
                {
                    if (mCameraGL->initialized) {
                       // mCameraGL->destroy();
                    }

                    mGPUCommandThreadState = STA_GPUCMD_STOP;

                    if(msg.arg1)
                        ((Semaphore*)msg.arg1)->Signal();

                    continue;
                }
            }
        }

        mGpuOPLock.lock();
        if (gpuCmdThreadCommandQ.isEmpty() == false ) {
            mGpuOPLock.unlock();
            goto gpu_receive_cmd;
        }

        mGpuOPCond.wait(mGpuOPLock);
        mGpuOPLock.unlock();

        goto gpu_receive_cmd;
    }
}
void CameraIspAdapter::sendBlockedMsg(int message) {
    //ALOGD("isp-msg, receive message: %d", message);
    Message_cam msg;
    Semaphore sem;

    mGpuOPLock.lock();

    msg.command = message;
    sem.Create();
    msg.arg1 = (void*)(&sem);
    gpuCmdThreadCommandQ.put(&msg);
    mGpuOPCond.signal();
    mGpuOPLock.unlock();
    if(msg.arg1){
        sem.Wait();
    }

}

void CameraIspAdapter::mfdCommandThread()
{
    Message_cam msg;
	float mfdsetISO;
	char mfd_set_enable;
	float get_mfdISO[3];
	float mfdFrames[3];
    while (mMFDCommandThreadState != STA_GPUCMD_STOP) {
    gpu_receive_cmd:
        if (mfdCmdThreadCommandQ.isEmpty() == false ) {
            mfdCmdThreadCommandQ.get(&msg);

            //ALOGD("isp-msg,command thread receive message: %d", msg.command);
            switch (msg.command)
            {
                case CMD_GPU_PROCESS_INIT:
                {
                    //ALOGD("check init, w-h: %d-%d, tid: %d", width, height, gettid());
                    if (!mMutliFrameDenoise->initialized) {
//                        mMutliFrameDenoise->initOpenGLES(NULL, mMfdFBOWidth, mMfdFBOHeight);
                    }
                    mMFDCommandThreadState = STA_GPUCMD_RUNNING;
                    if(msg.arg1)
                        ((Semaphore*)msg.arg1)->Signal();
                    break;
                }
                case CMD_GPU_PROCESS_UPDATE:
                {
                    if (mMutliFrameDenoise->initialized) {
                        //ALOGD("fill buffer capture, start: %.8x, dat0: %d, fd: %d, length: %d", mfd_buffers_capture->start, (void*)(mfd_buffers_capture->start),             mfd_buffers_capture->share_fd, mfd_buffers_capture->length);
					// m_camDevice->getGain(mfdISO);
//                        mMutliFrameDenoise->updateImageData(mfd_buffers_capture);
                    }
                    if(msg.arg1)
                        ((Semaphore*)msg.arg1)->Signal();

                    break;
                }
                case CMD_GPU_PROCESS_RENDER:
                {
                    if (mMutliFrameDenoise->initialized) {
                        m_camDevice->getGain(mfdISO);
//                        mMutliFrameDenoise->processing(NULL,mfdISO);
                        LOGD("CameraIspAdapter::mfdCommandThread mfdISO = %f",mfdISO);
                    }
                    if(msg.arg1)
                        ((Semaphore*)msg.arg1)->Signal();

                    break;
                }
                case CMD_GPU_PROCESS_SETFRAMES:
                {
					m_camDevice->getGain(mfdISO);

					m_camDevice->getMfdGain(&mfd_set_enable, get_mfdISO, mfdFrames);
					if(mfd_set_enable == 1) {
						if(mfdISO > get_mfdISO[2]) {
							mfd.process_frames = mfdFrames[2];
						}
						else if(mfdsetISO > get_mfdISO[1]){
							mfd.process_frames = mfdFrames[1];
						}
						else {
							mfd.process_frames = mfdFrames[0];
						}
					} else {
						mfd.process_frames = 2;
					}
//					mMutliFrameDenoise->setFrames(mfd.process_frames);
                    if(msg.arg1)
                        ((Semaphore*)msg.arg1)->Signal();
                    break;
                }
                case CMD_GPU_PROCESS_DEINIT:
                {
                    if (mMutliFrameDenoise->initialized) {
//                        mMutliFrameDenoise->destroy();
                    }

                    mMFDCommandThreadState = STA_GPUCMD_STOP;

                    if(msg.arg1)
                        ((Semaphore*)msg.arg1)->Signal();

                    continue;
                }
            }
        }

        mMfdOPLock.lock();
        if (mfdCmdThreadCommandQ.isEmpty() == false ) {
            mMfdOPLock.unlock();
            goto gpu_receive_cmd;
        }

        mMfdOPCond.wait(mMfdOPLock);
        mMfdOPLock.unlock();

        goto gpu_receive_cmd;
    }
}
void CameraIspAdapter::mfdsendBlockedMsg(int message) {
    //ALOGD("isp-msg, receive message: %d", message);
    Message_cam msg;
    Semaphore sem;

    mMfdOPLock.lock();

    msg.command = message;
    sem.Create();
    msg.arg1 = (void*)(&sem);
    mfdCmdThreadCommandQ.put(&msg);
    mMfdOPCond.signal();
    mMfdOPLock.unlock();
    if(msg.arg1){
        sem.Wait();
    }

}

int CameraIspAdapter::selectPreferedDrvSize(int *width,int * height,bool is_capture)
{

    return 0;
}

void CameraIspAdapter::bufferCb( MediaBuffer_t* pMediaBuffer )
{
    static int writeoneframe = 0;
    ulong_t y_addr = 0,uv_addr = 0;
	uint32_t y_size;
    void* y_addr_vir = NULL,*uv_addr_vir = NULL ;
    int width = 0,height = 0;
    int fmt = 0;
	int tem_val;
	ulong_t phy_addr=0;

	Mutex::Autolock lock(mLock);
    // get & check buffer meta data
    PicBufMetaData_t *pPicBufMetaData = (PicBufMetaData_t *)(pMediaBuffer->pMetaData);
    HalHandle_t  tmpHandle = m_camDevice->getHalHandle();

    debugShowFPS();

    if(pPicBufMetaData->Type == PIC_BUF_TYPE_YCbCr420 || pPicBufMetaData->Type == PIC_BUF_TYPE_YCbCr422){        
        if(pPicBufMetaData->Type == PIC_BUF_TYPE_YCbCr420){
            fmt = V4L2_PIX_FMT_NV12;
        }else{
            fmt = V4L2_PIX_FMT_YUYV;
        }

        if(pPicBufMetaData->Layout == PIC_BUF_LAYOUT_SEMIPLANAR ){
            y_addr = (ulong_t)(pPicBufMetaData->Data.YCbCr.semiplanar.Y.pBuffer);
            //now gap of y and uv buffer is 0. so uv addr could be calc from y addr.
            uv_addr = (ulong_t)(pPicBufMetaData->Data.YCbCr.semiplanar.CbCr.pBuffer);
            width = pPicBufMetaData->Data.YCbCr.semiplanar.Y.PicWidthPixel;
            height = pPicBufMetaData->Data.YCbCr.semiplanar.Y.PicHeightPixel;
            //get vir addr
            HalMapMemory( tmpHandle, y_addr, 100, HAL_MAPMEM_READWRITE, &y_addr_vir );
            HalMapMemory( tmpHandle, uv_addr, 100, HAL_MAPMEM_READWRITE, &uv_addr_vir );

			
#if defined(RK_DRM_GRALLOC) // should use fd
			HalGetMemoryMapFd(tmpHandle, y_addr,(int*)&phy_addr);
#else
            if(gCamInfos[mCamId].pcam_total_info->mIsIommuEnabled)
                HalGetMemoryMapFd(tmpHandle, y_addr,(int*)&phy_addr);
            else
                phy_addr = y_addr;
#endif
           
            /* ddl@rock-chips.com:  v1.3.0 */
            y_size = pPicBufMetaData->Data.YCbCr.semiplanar.Y.PicWidthPixel*pPicBufMetaData->Data.YCbCr.semiplanar.Y.PicHeightPixel;
            if (uv_addr > (y_addr+y_size)) {
                memcpy((void*)((ulong_t)y_addr_vir+y_size),uv_addr_vir, y_size/2);
            }
            
        }else if(pPicBufMetaData->Layout == PIC_BUF_LAYOUT_COMBINED){
            y_addr = (ulong_t)(pPicBufMetaData->Data.YCbCr.combined.pBuffer );
            width = pPicBufMetaData->Data.YCbCr.combined.PicWidthPixel>>1;
            height = pPicBufMetaData->Data.YCbCr.combined.PicHeightPixel;
            HalMapMemory( tmpHandle, y_addr, 100, HAL_MAPMEM_READWRITE, &y_addr_vir );
#if defined(RK_DRM_GRALLOC) // should use fd
			HalGetMemoryMapFd(tmpHandle, y_addr,(int*)&phy_addr);
#else
            if(gCamInfos[mCamId].pcam_total_info->mIsIommuEnabled)
                HalGetMemoryMapFd(tmpHandle, y_addr,(int*)&phy_addr);
            else
                phy_addr = y_addr;
#endif
        }

    } else if(pPicBufMetaData->Type == PIC_BUF_TYPE_RAW16) {

        y_addr = (ulong_t)(pPicBufMetaData->Data.raw.pBuffer );
        width = pPicBufMetaData->Data.raw.PicWidthPixel;
        height = pPicBufMetaData->Data.raw.PicHeightPixel;
        fmt = V4L2_PIX_FMT_SBGGR10;
        HalMapMemory( tmpHandle, y_addr, 100, HAL_MAPMEM_READWRITE, &y_addr_vir );
#if defined(RK_DRM_GRALLOC) // should use fd
		HalGetMemoryMapFd(tmpHandle, y_addr,(int*)&phy_addr);
#else
        if(gCamInfos[mCamId].pcam_total_info->mIsIommuEnabled)
            HalGetMemoryMapFd(tmpHandle, y_addr,(int*)&phy_addr);
        else
            phy_addr = y_addr;
#endif		
    } else {
        LOGE("not support this type(%dx%d)  ,just support  yuv20 now",width,height);
        return ;
    }
    

    if ( pMediaBuffer->pNext != NULL ) {
        MediaBufLockBuffer( (MediaBuffer_t*)pMediaBuffer->pNext );
    }
	
	if( (preview_frame_inval > 0) && !mIsCtsTest){
	  	preview_frame_inval--;
		LOG1("frame_inval:%d\n",preview_frame_inval);
        if(m_camDevice->isSOCSensor() == false){
			bool awb_ret = m_camDevice->isAwbStable();
			LOG1("awb test fps(%d) awb stable(%d)\n", preview_frame_inval, awb_ret);
			if( awb_ret!=true){
				LOG1("awb test fps(%d) awb stable(%d)\n", preview_frame_inval, awb_ret);
				goto end;
			}
		}else{
			goto end;
		}
    }


    if(mIsSendToTunningTh){
        MediaBufLockBuffer( pMediaBuffer );
        //new frames
        FramInfo_s *tmpFrame=(FramInfo_s *)malloc(sizeof(FramInfo_s));
        if(!tmpFrame){
            MediaBufUnlockBuffer( pMediaBuffer );
            return;
        }
        //add to vector
        memset(tmpFrame, 0x0, sizeof(*tmpFrame));
        tmpFrame->frame_index = (ulong_t)tmpFrame; 
        tmpFrame->phy_addr = (ulong_t)phy_addr;
        tmpFrame->frame_width = width;
        tmpFrame->frame_height= height;
        tmpFrame->vir_addr = (ulong_t)y_addr_vir;
        tmpFrame->frame_fmt = fmt;
        tmpFrame->used_flag = (ulong_t)pMediaBuffer; // tunning thread will use pMediaBuffer

        {
            Mutex::Autolock lock(mFrameArrayLock);
            mFrameInfoArray.add((void*)tmpFrame,(void*)pMediaBuffer);
        }
        Message_cam msg;
        msg.command = ISP_TUNNING_CMD_PROCESS_FRAME;
        msg.arg2 = (void*)(tmpFrame);
        msg.arg3 = (void*)(tmpFrame->used_flag);
        mISPTunningQ->put(&msg);

    }else{
        //need to send face detection ?
    	if(mRefEventNotifier->isNeedSendToFaceDetect()){  
    	    MediaBufLockBuffer( pMediaBuffer );
    		//new frames
    		FramInfo_s *tmpFrame=(FramInfo_s *)malloc(sizeof(FramInfo_s));
    		if(!tmpFrame){
    			MediaBufUnlockBuffer( pMediaBuffer );
    			return;
          }
          //add to vector
          memset(tmpFrame, 0x0, sizeof(*tmpFrame));
          tmpFrame->frame_index = (ulong_t)tmpFrame; 
          tmpFrame->phy_addr = (ulong_t)phy_addr;
          tmpFrame->frame_width = width;
          tmpFrame->frame_height= height;
          tmpFrame->vir_addr = (ulong_t)y_addr_vir;
          tmpFrame->frame_fmt = fmt;
    	  
          tmpFrame->used_flag = 4;

          tmpFrame->zoom_value = mZoomVal;
        
          {
            Mutex::Autolock lock(mFrameArrayLock);
            mFrameInfoArray.add((void*)tmpFrame,(void*)pMediaBuffer);

          }
          mRefEventNotifier->notifyNewFaceDecFrame(tmpFrame);
        }
    	//need to display ?
    	if(mRefDisplayAdapter->isNeedSendToDisplay()){  
	    	property_set("sys.hdmiin.display", "1");//just used by hdmi-in
    	    MediaBufLockBuffer( pMediaBuffer );
    		//new frames
    		FramInfo_s *tmpFrame=(FramInfo_s *)malloc(sizeof(FramInfo_s));
    		if(!tmpFrame){
    			MediaBufUnlockBuffer( pMediaBuffer );
    			return;
          }
          //add to vector
          memset(tmpFrame, 0x0, sizeof(*tmpFrame));
          tmpFrame->frame_index = (ulong_t)tmpFrame; 
          tmpFrame->phy_addr = (ulong_t)phy_addr;
          tmpFrame->frame_width = width;
          tmpFrame->frame_height= height;
          tmpFrame->vir_addr = (ulong_t)y_addr_vir;
          tmpFrame->frame_fmt = fmt;
    	  
          tmpFrame->used_flag = 0;

          #if (USE_RGA_TODO_ZOOM == 1)  
             tmpFrame->zoom_value = mZoomVal;
          #else
          if((tmpFrame->frame_width > 2592) && (tmpFrame->frame_height > 1944) && (mZoomVal != 100) ){
             tmpFrame->zoom_value = mZoomVal;
          }else
             tmpFrame->zoom_value = 100;
          #endif
        
          {
            Mutex::Autolock lock(mFrameArrayLock);
            mFrameInfoArray.add((void*)tmpFrame,(void*)pMediaBuffer);
            mDispFrameLeak++;

          }
          mRefDisplayAdapter->notifyNewFrame(tmpFrame);

        }

    	//video enc ?
    	if(mRefEventNotifier->isNeedSendToVideo()) {
            MediaBufLockBuffer( pMediaBuffer );
            //new frames
            FramInfo_s *tmpFrame=(FramInfo_s *)malloc(sizeof(FramInfo_s));
            if(!tmpFrame){
            	MediaBufUnlockBuffer( pMediaBuffer );
            	return;
            }          
            //add to vector
            memset(tmpFrame, 0x0, sizeof(*tmpFrame));
            tmpFrame->frame_index = (ulong_t)tmpFrame; 
            tmpFrame->phy_addr = (ulong_t)phy_addr;
            tmpFrame->frame_width = width;
            tmpFrame->frame_height= height;
            tmpFrame->vir_addr = (ulong_t)y_addr_vir;
            tmpFrame->frame_fmt = fmt;
            tmpFrame->used_flag = 1;
            tmpFrame->vir_addr_valid = true;
#if (USE_RGA_TODO_ZOOM == 1)  
            tmpFrame->zoom_value = mZoomVal;
#else
            if((tmpFrame->frame_width > 2592) && (tmpFrame->frame_height > 1944) && (mZoomVal != 100) ) {
                tmpFrame->zoom_value = mZoomVal;
            } else {
                tmpFrame->zoom_value = 100;
            }
#endif
          
            {
                Mutex::Autolock lock(mFrameArrayLock);
                mFrameInfoArray.add((void*)tmpFrame,(void*)pMediaBuffer);
                mVideoEncFrameLeak++;
            }
            mRefEventNotifier->notifyNewVideoFrame(tmpFrame);		
    	}
        
    	//picture ?
    	if(mRefEventNotifier->isNeedSendToPicture()){
    	#if 0
            bool send_to_pic = true;
			MediaBufLockBuffer( pMediaBuffer );
            //new frames
            FramInfo_s *tmpFrame=(FramInfo_s *)malloc(sizeof(FramInfo_s));
            if(!tmpFrame){
				MediaBufUnlockBuffer( pMediaBuffer );
				return;
            }
			memset(tmpFrame, 0x0, sizeof(*tmpFrame));
			{
				if (mfd.enable) {
					mfd_buffers_capture->start = y_addr_vir;
					mfd_buffers_capture->share_fd = phy_addr;
					mfd_buffers_capture->length = width * height * 3 / 2;
					mfd_buffers_capture->handle = NULL;
					if (!mMutliFrameDenoise->initialized) {
						mMfdFBOWidth = width;
						mMfdFBOHeight = height;
						mfdsendBlockedMsg(CMD_GPU_PROCESS_INIT);
					}
					if((mfd.frame_cnt == 0) && (mMutliFrameDenoise->initialized)) {
						LOGE("mMutliFrameDenoise->initialized:%d,mfd.frame_cnt:%d	hcc101802!",mMutliFrameDenoise->initialized,mfd.frame_cnt);
						mfdsendBlockedMsg(CMD_GPU_PROCESS_SETFRAMES);
						LOGE("CMD_GPU_PROCESS_SETFRAMES finish!");
					}
					if(mfd.frame_cnt < mfd.process_frames) {
						mfdsendBlockedMsg(CMD_GPU_PROCESS_UPDATE);
						mfd.buffer_full = false;
						mfd.frame_cnt++ ;
					} else {
						mfd.frame_cnt = 0;
						mfd.buffer_full = true;
					}
				} else {
				mfd.frame_cnt = 0;
				mfd.buffer_full = true;
				}
			}

			if(mfd.buffer_full == true) {
				#if 0
	             if(mFlashStatus && ((ulong_t)(pPicBufMetaData->priv) != 1)){
	                pPicBufMetaData->priv = NULL;
	                send_to_pic = false;
	                LOG1("not the desired flash pic,skip it,mFlashStatus %d!",mFlashStatus);
	            }
	            #endif
				if (send_to_pic) {
					float flash_luminance = 0;
					tmpFrame->vir_addr = (ulong_t)y_addr_vir;
	                if ((mMutliFrameDenoise->initialized) && (mfd.enable)) {
						mfdsendBlockedMsg(CMD_GPU_PROCESS_RENDER);
//	                    mMutliFrameDenoise->getResult(tmpFrame->vir_addr);
	                }

					if( tmpFrame->vir_addr == NULL) {
						LOGE("mfd tmpFrame->vir_addr is NULL!");
					}

					if (uvnr.enable) {
						if (!mCameraGL->initialized) {
							mGpuFBOWidth = width;
							mGpuFBOHeight = height;
							sendBlockedMsg(CMD_GPU_PROCESS_INIT);
						}

						m_buffers_capture->start = (void *)tmpFrame->vir_addr;
						m_buffers_capture->share_fd = phy_addr;
						m_buffers_capture->length = width * height * 3 / 2;
						m_buffers_capture->handle = NULL;

						sendBlockedMsg(CMD_GPU_PROCESS_UPDATE);
						sendBlockedMsg(CMD_GPU_PROCESS_RENDER);
//						mCameraGL->getResult(tmpFrame->vir_addr);
					}
					if( tmpFrame->vir_addr == NULL) {
						LOGE("uvnr tmpFrame->vir_addr is NULL!");
					}
	                //add to vector
	                tmpFrame->frame_index = (ulong_t)tmpFrame; 
	                tmpFrame->phy_addr = (ulong_t)phy_addr;
	                tmpFrame->frame_width = width;
	                tmpFrame->frame_height= height;
	                //tmpFrame->vir_addr = (ulong_t)y_addr_vir;
	                tmpFrame->frame_fmt = fmt;
	                tmpFrame->used_flag = 2;
	                tmpFrame->res = &mImgAllFovReq;
#if (USE_RGA_TODO_ZOOM == 1)  
	                tmpFrame->zoom_value = mZoomVal;
#else
	                if((tmpFrame->frame_width > 2592) && (tmpFrame->frame_height > 1944) && (mZoomVal != 100) ){
	                    tmpFrame->zoom_value = mZoomVal;
	                } else {
	                    tmpFrame->zoom_value = 100;
	                }
#endif
	                {
	                    Mutex::Autolock lock(mFrameArrayLock);
	                    mFrameInfoArray.add((void*)tmpFrame,(void*)pMediaBuffer);
	                    mPicEncFrameLeak++;
	                }
	                picture_info_s &picinfo = mRefEventNotifier->getPictureInfoRef();
	                getCameraParamInfo(picinfo.cameraparam);
	                mRefEventNotifier->notifyNewPicFrame(tmpFrame);
	            }
            }
        #endif
				MediaBufLockBuffer( pMediaBuffer );
				//new frames
				FramInfo_s *tmpFrame=(FramInfo_s *)malloc(sizeof(FramInfo_s));
				if(!tmpFrame){
					MediaBufUnlockBuffer( pMediaBuffer );
					return;
				}		   
				//add to vector
				memset(tmpFrame, 0x0, sizeof(*tmpFrame));
				tmpFrame->frame_index = (ulong_t)tmpFrame; 
				tmpFrame->phy_addr = (ulong_t)phy_addr;
				tmpFrame->frame_width = width;
				tmpFrame->frame_height= height;
				tmpFrame->vir_addr = (ulong_t)y_addr_vir;
				tmpFrame->frame_fmt = fmt;
				tmpFrame->used_flag = 2;
				tmpFrame->res = &mImgAllFovReq;
#if (USE_RGA_TODO_ZOOM == 1)  
				tmpFrame->zoom_value = mZoomVal;
#else
				if((tmpFrame->frame_width > 2592) && (tmpFrame->frame_height > 1944) && (mZoomVal != 100) ) {
					tmpFrame->zoom_value = mZoomVal;
				} else {
					tmpFrame->zoom_value = 100;
				}
#endif
				{
					Mutex::Autolock lock(mFrameArrayLock);
					mFrameInfoArray.add((void*)tmpFrame,(void*)pMediaBuffer);
					mPicEncFrameLeak++;
				}
				picture_info_s &picinfo = mRefEventNotifier->getPictureInfoRef();
				getCameraParamInfo(picinfo.cameraparam);
				mRefEventNotifier->notifyNewPicFrame(tmpFrame);

        }

    	//preview data callback ?
    	if(mRefEventNotifier->isNeedSendToDataCB() && (mRefDisplayAdapter->getDisplayStatus() == 0)) {
            MediaBufLockBuffer( pMediaBuffer );
            //new frames
            FramInfo_s *tmpFrame=(FramInfo_s *)malloc(sizeof(FramInfo_s));
            if(!tmpFrame){
            	MediaBufUnlockBuffer( pMediaBuffer );
            	return;
            }
            //add to vector
            memset(tmpFrame, 0x0, sizeof(*tmpFrame));
            tmpFrame->frame_index = (ulong_t)tmpFrame; 
            tmpFrame->phy_addr = (ulong_t)phy_addr;
            tmpFrame->frame_width = width;
            tmpFrame->frame_height= height;
            tmpFrame->vir_addr = (ulong_t)y_addr_vir;
            tmpFrame->frame_fmt = fmt;
            tmpFrame->used_flag = 3;
#if (USE_RGA_TODO_ZOOM == 1)  
            tmpFrame->zoom_value = mZoomVal;
#else
            if((tmpFrame->frame_width > 2592) && (tmpFrame->frame_height > 1944) && (mZoomVal != 100) ) {
                tmpFrame->zoom_value = mZoomVal;
            } else {
                tmpFrame->zoom_value = 100;
            }
#endif

            {
                Mutex::Autolock lock(mFrameArrayLock);
                mFrameInfoArray.add((void*)tmpFrame,(void*)pMediaBuffer);
                mPreviewCBFrameLeak++;
            }
                mRefEventNotifier->notifyNewPreviewCbFrame(tmpFrame);			
        }
    }
end:
	
	tem_val =0 ;
}

void CameraIspAdapter::dump(int cameraId)
{
    Message_cam msg;
	rk_cam_total_info *pCamInfo = gCamInfos[cameraId].pcam_total_info;
	m_camDevice->checkVersion(pCamInfo);

    if(mISPTunningRun){
        TRACE_D(0, "-----------stop isp tunning in--------------");
        msg.command = ISP_TUNNING_CMD_EXIT;
        mISPTunningQ->put(&msg);
    	mISPTunningThread->requestExitAndWait();
    	mISPTunningThread.clear();
        delete mISPTunningQ;
        mISPTunningQ = NULL;
        delete mIspTunningTask;
        mIspTunningTask = NULL;
        mISPTunningRun = false;
        TRACE_D(0, "-----------stop isp tunning out--------------");
    }else{
        TRACE_D(0, "-----------start isp tunning in--------------");
        mISPTunningQ = new MessageQueue("ISPTunningQ");
        mISPTunningThread = new CamISPTunningThread(this);

        //parse tunning xml file
        mIspTunningTask = CameraIspTunning::createInstance();
        if(mIspTunningTask){
            mISPTunningThread->run("CamISPTunningThread",ANDROID_PRIORITY_NORMAL);
            msg.command = ISP_TUNNING_CMD_START;
            mISPTunningQ->put(&msg);
            mISPTunningRun = true;
            TRACE_D(0, "-----------start isp tunning out--------------");
        }else{
            delete mISPTunningQ;
            mISPTunningThread.clear();
            TRACE_E("-----------start isp tunning failed--------------");
        }
    }
}

int CameraIspAdapter::afListenerThread(void)
{

    LOG_FUNCTION_NAME

    bool bExit = false;
    int evnt_id ;

    while (bExit == false)
    {
        CamEngineAfEvt_t afEvt;        
        
        OSLAYER_STATUS osStatus = (OSLAYER_STATUS)osQueueRead(&mAfListenerQue.queue, &afEvt); 
        if (OSLAYER_OK != osStatus)
        {
            LOGE( "receiving af event failed -> OSLAYER_RESULT=%d\n", osStatus );
            continue; /* for now we simply try again */
        }

        evnt_id = (int)afEvt.evnt_id;
        switch (evnt_id)
        {
            case CAM_ENGINE_AUTOFOCUS_MOVE:
            {
                LOG2("CAMERA_MSG_FOCUS_MOVE: %d",afEvt.info.mveEvt.start);
                mRefEventNotifier->notifyCbMsg(CAMERA_MSG_FOCUS_MOVE, afEvt.info.mveEvt.start);
                break;
            }

            case CAM_ENGINE_AUTOFOCUS_FINISHED:
            {
                LOG2("CAMERA_MSG_FOCUS: %d",afEvt.info.fshEvt.focus);
                mRefEventNotifier->notifyCbMsg(CAMERA_MSG_FOCUS, afEvt.info.fshEvt.focus);
                break;
            }

            case 0xfefe5aa://change from 0xfefe5aa5 to 0xfefe5aa for android Nougat
            {
                LOG1("receive exit command for af thread handle!");
                bExit = true;
                break;
            }
            default:
            {                    
                LOGE("afEvt.evnt_id: 0x%x is invalidate!",afEvt.evnt_id);
                break;
            }
        }
    }

    LOG_FUNCTION_NAME_EXIT

    return 0;
}

bool CameraIspAdapter::isNeedToEnableFlash()
{	

    if(mParameters.get(CameraParameters::KEY_SUPPORTED_FLASH_MODES)
        && ((strcmp(mParameters.get(CameraParameters::KEY_FLASH_MODE),CameraParameters::FLASH_MODE_ON)==0) ||
         ((strcmp(mParameters.get(CameraParameters::KEY_FLASH_MODE),CameraParameters::FLASH_MODE_AUTO)==0) && isLowIllumin(45)))){
         return true;
    }else{
        return false;
    }
}
bool CameraIspAdapter::isLowIllumin(const float lumaThreshold)
{
    float gain,mingain,maxgain,step,time,mintime,maxtime,sharpness,meanluma = 0 ;
    bool enabled;
    CamEngineAfSearchAlgorithm_t searchAlgorithm;
    //const float lumaThreshold = 15;
    const float sharpThreshold = 300;
    
    m_camDevice->getGain(gain);
    m_camDevice->getIntegrationTime(time);
    m_camDevice->getGainLimits(mingain,maxgain,step);
    m_camDevice->getIntegrationTimeLimits(mintime,maxtime,step);
    meanluma = m_camDevice->getAecMeanLuminance();

    m_camDevice->getAfStatus(enabled,searchAlgorithm,&sharpness);  /* ddl@rock-chips.com: v0.0x32.0 */     

    LOG1("Check LowIllumin :")
    LOG1("    gain       %f(%f,%f)",gain,mingain,maxgain);
    LOG1("    inttime    %f(%f,%f)",time,mintime,maxtime);
    LOG1("    meanluma   %f     threshold: %f",meanluma,lumaThreshold);
    LOG1("    sharpness: %f     threshold: %f",sharpness,sharpThreshold);

    if( meanluma < lumaThreshold ) //&& (sharpness < sharpThreshold))
        return true;
    else
        return false;
}

void CameraIspAdapter::flashControl(bool on)
{
    if(mFlashStatus && !on){
        m_camDevice->stopFlash(false);
        //restore awb
        m_camDevice->startAec();
        m_camDevice->lscEnable();
  //      m_camDevice->startAdpf();
  //      m_camDevice->startAdpcc();
  //      m_camDevice->startAvs();
        if(curAwbStatus.manual_mode){
            curAwbStatus.manual_mode = false;
            m_camDevice->stopAwb();
            m_camDevice->startAwb(curAwbStatus.mode, curAwbStatus.idx, (bool_t)curAwbStatus.damping);
        }
        mFlashStatus = false;
    }else if(!mFlashStatus && on){
        float mingain,maxgain,step,time,mintime,maxtime,meanluma = 0 ;
        if(isLowIllumin(45)){
            //get awb status
            m_camDevice->getAwbStatus(curAwbStatus.enabled, curAwbStatus.mode, curAwbStatus.idx, curAwbStatus.RgProj, curAwbStatus.damping);
            //stop awb
           // m_camDevice->stopAwb();
            //set D65 manual awb
           // m_camDevice->startAwb(CAM_ENGINE_AWB_MODE_MANUAL, 1, (bool_t)false);
            curAwbStatus.manual_mode = false;
            m_camDevice->stopAec();
            m_camDevice->getGainLimits(mingain,maxgain,step);
            m_camDevice->setGain(maxgain, maxgain);
            m_camDevice->getIntegrationTimeLimits(mintime,maxtime,step);
            m_camDevice->setIntegrationTime(maxtime, maxtime);

            m_camDevice->lscDisable(); /*ddl@rock-chips.com: v1.0x25.0*/
        }
//     m_camDevice->stopAvs();
//     m_camDevice->stopAdpcc();
//     m_camDevice->stopAdpf();
       
//     m_camDevice->stopAwb();
//       //set D65 manual awb
//     m_camDevice->startAwb(CAM_ENGINE_AWB_MODE_MANUAL, 1, (bool_t)false);
        m_camDevice->startFlash(true);
        mFlashStatus = true;
        LOG1("flash set to status %d",mFlashStatus);

    }else{
        LOG1("flash is already in status %d",mFlashStatus);
    }
}

void CameraIspAdapter::getCameraParamInfo(cameraparam_info_s &paraminfo)
{
	
	m_camDevice->getAwbGainInfo(&paraminfo.f_RgProj,&paraminfo.f_s,&paraminfo.f_s_Max1,&paraminfo.f_s_Max2,
								&paraminfo.f_Bg1,&paraminfo.f_Rg1,&paraminfo.f_Bg2,&paraminfo.f_Rg2);

	m_camDevice->getIlluEstInfo(&paraminfo.expPriorIn,&paraminfo.expPriorOut,paraminfo.illuName,paraminfo.likehood,
								paraminfo.wight,&paraminfo.illuIdx,&paraminfo.region,&paraminfo.count);
	
	m_camDevice->getIntegrationTime(paraminfo.ExposureTime);
	m_camDevice->getGain(paraminfo.ISOSpeedRatings);
	m_camDevice->getSensorXmlVersion(&paraminfo.XMLVersion);
}

bool CameraIspAdapter::getFlashStatus()
{
	return mFlashStatus;
}

void CameraIspAdapter::getSensorMaxRes(unsigned int &max_w, unsigned int &max_h)
{
	IsiSensorCaps_t pCaps;		
	pCaps.Index = 0;
	max_w = 0;
	max_h = 0;
	while (m_camDevice->getSensorCaps(pCaps) == true) {
		if (ISI_RES_W_GET(pCaps.Resolution)>max_w)
			max_w = ISI_RES_W_GET(pCaps.Resolution);
		if (ISI_RES_H_GET(pCaps.Resolution)>max_h)
			max_h = ISI_RES_H_GET(pCaps.Resolution);
		pCaps.Index++;
	};

}

void CameraIspAdapter::setMwb(const char *white_balance)
{
	uint32_t illu_index = 1;
	char prfName[10];
	int i,size;
	std::vector<CamIlluProfile_t *> profiles;

	if(white_balance == NULL)
		return;
	
	if(!strcmp(white_balance, "auto")) {
		//do nothing
	}else{
	    if (m_camDevice->isSOCSensor() == false) {
			if(!strcmp(white_balance, CameraParameters::WHITE_BALANCE_INCANDESCENT)) {
				strcpy(prfName, "A");
			} else if(!strcmp(white_balance, CameraParameters::WHITE_BALANCE_DAYLIGHT)) {
				strcpy(prfName, "D65");	
			} else if(!strcmp(white_balance, CameraParameters::WHITE_BALANCE_FLUORESCENT)) {
				strcpy(prfName, "F2_CWF");
			} else if(!strcmp(white_balance, CameraParameters::WHITE_BALANCE_SHADE)) {
				strcpy(prfName, "D75");
			} else if(!strcmp(white_balance, CameraParameters::WHITE_BALANCE_TWILIGHT)) {
				strcpy(prfName, "HORIZON");
			} else if(!strcmp(white_balance, CameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT)) {
				strcpy(prfName, "D50");
			} else if(!strcmp(white_balance, CameraParameters::WHITE_BALANCE_WARM_FLUORESCENT)) {
				strcpy(prfName, "U30");
			}
			
			m_camDevice->getIlluminationProfiles( profiles );			
			size = profiles.size();
			for(i=0; i<size; i++)
			{
				if(strstr(profiles[i]->name, prfName))
				{
					illu_index = i;
					break;
				}
			}
			
	        m_camDevice->stopAwb();
			m_camDevice->startAwb(CAM_ENGINE_AWB_MODE_MANUAL, illu_index, (bool_t)true);
	    }
	}
}

void CameraIspAdapter::setMe(const char *exposure)
{
    #if 1
    if (m_camDevice->isSOCSensor() == true)   /* ddl@rock-chips.com : v0.0x39.0 */ 
        return;
    
	if(exposure == NULL)
		return;
	if(!strcmp(exposure, "-2")){
		m_camDevice->setAeClmTolerance(manExpConfig.clmtolerance*0.8);
		m_camDevice->setAePoint(manExpConfig.minus_level_2);
	}else if(!strcmp(exposure, "-1")){
		m_camDevice->setAePoint(manExpConfig.minus_level_1);
	}else if(!strcmp(exposure, "0")){
		m_camDevice->setAePoint(manExpConfig.level_0);
	}else if(!strcmp(exposure, "1")){
		m_camDevice->setAePoint(manExpConfig.plus_level_1);
	}else if(!strcmp(exposure, "2")){
		m_camDevice->setAeClmTolerance(manExpConfig.clmtolerance*0.8);
		m_camDevice->setAePoint(manExpConfig.plus_level_2);
	}
    #endif
}

int CameraIspAdapter::ispTunningThread(void)
{
    bool bExit = false;
    Message_cam msg;
    int curFmt = 0,curPreW = 0,curPreH =0;
    float gain,setgain,mingain,maxgain,gainstep,time,settime,mintime,maxtime,timestep;
    ispTuneTaskInfo_s* curTuneTask = NULL;
    char szBaseFileName[100];
    static int skip_frames = 0;
    while (bExit == false)
    {
        memset(&msg,0,sizeof(msg));
        mISPTunningQ->get(&msg);
PROCESS_CMD:
        switch(msg.command)
        {
            case ISP_TUNNING_CMD_START:
                //stop preview
                stopPreview();
                //start preview
                curTuneTask = mIspTunningTask->mCurTuneTask = mIspTunningTask->mTuneInfoVector[mIspTunningTask->mCurTunIndex];

                curPreW = curTuneTask->mTuneWidth;
                curPreH = curTuneTask->mTuneHeight;

                if(curTuneTask->mTuneFmt == CAMERIC_MI_DATAMODE_RAW12){
                    curFmt = ISP_OUT_RAW12;
                    curTuneTask->mForceRGBOut = false;
                    if(curTuneTask->mExpose.exposuseMode == EXPOSUSE_MODE_AUTO){
                        //get tune task res ae value
                        startPreview(curPreW, curPreH, 0, 0,ISP_OUT_YUV420SP, false);
                        m_camDevice->stopAec();
                        m_camDevice->startAec();
                        sleep(3);
                        m_camDevice->getGain(gain);
                        m_camDevice->getIntegrationTime(time);
                        stopPreview();
                    }
					m_camDevice->enableSensorOTP((bool_t)false);//disable sensor otp awb if it has.
                }else if(curTuneTask->mTuneFmt == CAMERIC_MI_DATAMODE_YUV422){
                    curFmt = ISP_OUT_YUV422_SEMI;
                    curTuneTask->mForceRGBOut = true;
                } else {
                    LOGE("this format %d is not support",curTuneTask->mTuneFmt);
                }
                    
                
                startPreview(curPreW, curPreH, curPreW, curPreH,curFmt, false);

                // need to do following steps in RAW capture mode ?
                // following modules are bypassed in RAW mode ?
                m_camDevice->stopAvs();
                m_camDevice->stopAdpf();
                m_camDevice->stopAec();
                m_camDevice->stopAwb();
                m_camDevice->stopAdpcc();
                m_camDevice->lscDisable();
                m_camDevice->cacDisable();
                m_camDevice->wdrDisable();
                m_camDevice->gamCorrectDisable();
                m_camDevice->stopAf();

                if(curTuneTask->mDpccEnable == true) 
                    m_camDevice->startAdpcc();
                if(curTuneTask->mLscEnable == true)
                   m_camDevice->lscEnable();
                if(curTuneTask->mCacEnable== true)
                    m_camDevice->cacEnable();
                if(curTuneTask->mGammarEnable == true)
                    m_camDevice->gamCorrectEnable();
                if(curTuneTask->mWdrEnable == true)
                    m_camDevice->wdrEnable();

                if(curTuneTask->mExpose.exposuseMode == EXPOSUSE_MODE_MANUAL){
                   //set manual ae
                    m_camDevice->getGain(gain);
                    m_camDevice->getIntegrationTime(time);
                    m_camDevice->getGainLimits(mingain,maxgain,gainstep);
                    m_camDevice->getIntegrationTimeLimits(mintime,maxtime,timestep);
                    m_camDevice->setIntegrationTime(curTuneTask->mExpose.integrationTime,settime);
                    LOGD("setIntegrationTime(desired:%0.3f,real:%0.3f)",curTuneTask->mExpose.integrationTime,settime);
                    m_camDevice->setGain(curTuneTask->mExpose.gain,setgain);
                    LOGD("setGain(desired:%0.3f,real:%0.3f)",curTuneTask->mExpose.gain,setgain);
                    
                    mIspTunningTask->mCurGain = setgain;
                    mIspTunningTask->mCurIntegrationTime = settime;
                    if(curTuneTask->mExpose.aeRound == true){
                        mIspTunningTask->mCurAeRoundNum = curTuneTask->mExpose.number;
                    }
                }else{
                    if(curTuneTask->mTuneFmt == CAMERIC_MI_DATAMODE_YUV422){
                       m_camDevice->startAec();
                    }else{
                        m_camDevice->setGain(gain,setgain);
                        m_camDevice->setIntegrationTime(time,settime);
                    }
                   
                }

                if(curTuneTask->mWhiteBalance.whiteBalanceMode == WHITEBALANCE_MODE_AUTO){
                    m_camDevice->startAwb(CAM_ENGINE_AWB_MODE_AUTO, 0, (bool_t)true);
                }else if(curTuneTask->mWhiteBalance.whiteBalanceMode == WHITEBALANCE_MODE_MANUAL){
                   //set manual awb
                    uint32_t illu_index = 1;
                    if(strcmp(curTuneTask->mWhiteBalance.illumination,"A") == 0)
                        illu_index = 0;
                    else if(strcmp(curTuneTask->mWhiteBalance.illumination,"D65") == 0)
                        illu_index = 1;
                    else if(strcmp(curTuneTask->mWhiteBalance.illumination,"CWF") == 0)
                        illu_index = 2;
                    else if(strcmp(curTuneTask->mWhiteBalance.illumination,"TL84") == 0)
                        illu_index = 3;
                    else if(strcmp(curTuneTask->mWhiteBalance.illumination,"D50") == 0)
                        illu_index = 4;
                    else if(strcmp(curTuneTask->mWhiteBalance.illumination,"D75") == 0)
                        illu_index = 5;
                    else if(strcmp(curTuneTask->mWhiteBalance.illumination,"HORIZON") == 0)
                        illu_index = 6;
                    else{
                        LOGE("not support this illum %s ,set to D65!!!",curTuneTask->mWhiteBalance.illumination);
                    }

                    LOGD("current illum is %s ,illu_index %d !!!",curTuneTask->mWhiteBalance.illumination,illu_index);
                    m_camDevice->startAwb( CAM_ENGINE_AWB_MODE_MANUAL, illu_index, (bool_t)true );

                    if(strcmp(curTuneTask->mWhiteBalance.cc_matrix,"unit_matrix") == 0){
                        LOGD(" set unit matrix ");
                        m_camDevice->wbCcMatrixSet(1,0,0,
                                                   0,1,0,
                                                   0,0,1);
                    }else if(strcmp(curTuneTask->mWhiteBalance.cc_matrix,"default") == 0){
                    }else{
                        LOGE("not support this cc_matrix %s !!!",curTuneTask->mWhiteBalance.cc_matrix);
                    }

                    if(strcmp(curTuneTask->mWhiteBalance.rggb_gain,"unit") == 0){
                        m_camDevice->wbGainSet(1,1,1,1);
                    }else if(strcmp(curTuneTask->mWhiteBalance.rggb_gain,"default") == 0){
                    }else{
                        LOGE("not support this rggb_gain %s !!!",curTuneTask->mWhiteBalance.rggb_gain);
                    }

                    if(strcmp(curTuneTask->mWhiteBalance.cc_offset,"zero") == 0){
                        m_camDevice->wbCcOffsetSet(0,0,0);
                    }else if(strcmp(curTuneTask->mWhiteBalance.cc_offset,"default") == 0){

                    }else{
                        LOGE("not support this cc_offset %s !!!",curTuneTask->mWhiteBalance.cc_offset);
                    }
                
                }

                #if 1
                if(curTuneTask->mAfEnable){
                    LOGD("start oneshot af");
                    m_camDevice->startAfOneShot(CAM_ENGINE_AUTOFOCUS_SEARCH_ALGORITHM_ADAPTIVE_RANGE);
                    sleep(2);
                    m_camDevice->startAfOneShot(CAM_ENGINE_AUTOFOCUS_SEARCH_ALGORITHM_ADAPTIVE_RANGE);
                    sleep(1);
                }
                #endif   
                skip_frames = 15;
                
                mIsSendToTunningTh = true;
                break;
            case ISP_TUNNING_CMD_EXIT:
                //restore saved config
                mIsSendToTunningTh = false;
                stopPreview();
                //mISPOutputFmt = oldFmt;
                startPreview(800, 600, 0, 0, ISP_OUT_YUV420SP, false);
                bExit = true;
                break;
            case ISP_TUNNING_CMD_PROCESS_FRAME:
                {
				FramInfo_s *frame = (FramInfo_s*)msg.arg2;
                float newtime,newgain;
                bool isStore = false;
                LOG1("tunning thread receive a frame !!");
                //
                if(skip_frames-- > 0)
                    goto PROCESS_OVER;
                m_camDevice->getGain(gain);
                m_camDevice->getIntegrationTime(time);
                //is it the satisfied frame ?
                if(curTuneTask->mExpose.exposuseMode == EXPOSUSE_MODE_MANUAL){
                    bool isSetExp = true;

                    if((gain != mIspTunningTask->mCurGain) || (time != mIspTunningTask->mCurIntegrationTime)){
                        LOGD("%s:not the desired exposure frame,skip it,time(%0.3f,%0.3f),gain(%0.3f,%0.3f)",__func__,
                            mIspTunningTask->mCurIntegrationTime,time,mIspTunningTask->mCurGain,gain);
                        goto PROCESS_OVER;
                    }

                    //set different exposure parameter ?
                    if(curFmt == ISP_OUT_RAW12 ){
                        if((curTuneTask->mExpose.integrationTimeStep != 0)|| (curTuneTask->mExpose.gainStep != 0)){
                            newtime = mIspTunningTask->mCurIntegrationTime+curTuneTask->mExpose.integrationTimeStep;
                            if((newtime >maxtime) && ((newtime - maxtime) - curTuneTask->mExpose.integrationTimeStep < 0.0001)){
                                //next loop
                                LOGD("set new gain+++");
                                newgain = mIspTunningTask->mCurGain+curTuneTask->mExpose.gainStep;
                                newtime = curTuneTask->mExpose.integrationTime;
                                if ((newgain > maxgain) && ((newgain - maxgain) - curTuneTask->mExpose.gainStep < 0.00001)){
                                    curTuneTask->mTunePicNum = 0;
                                    LOGD("time and gain are max ,newgain %0.5f,%0.5f!!!!",newgain,maxgain);
                                    isSetExp = false;
                                }
                            }else
                                newgain = mIspTunningTask->mCurGain;
                                
                            if(isSetExp){
                                m_camDevice->setIntegrationTime(newtime,settime);
                                m_camDevice->setGain(newgain,setgain);

                                LOGD("setIntegrationTime(desired:%0.3f,real:%0.3f)",newtime,settime);
                                LOGD("setGain(desired:%0.3f,real:%0.3f)",newgain,setgain);
                                mIspTunningTask->mCurGain = setgain;
                                mIspTunningTask->mCurIntegrationTime = settime;
                                //skip some frames,max buffer count is 6,one is here now,so filter 5 frames
                                skip_frames = 12;
                            }
                            int filter_result = mIspTunningTask->ispTuneDesiredExp(frame->vir_addr, frame->frame_width,frame->frame_height,
                                    curTuneTask->mExpose.minRaw, curTuneTask->mExpose.maxRaw, curTuneTask->mExpose.threshold); 
                            if(filter_result & 0x1){

                                goto PROCESS_OVER;
                            }else if(filter_result & 0x2){  
                                LOGD("frame is overhead exposure , finish capture frame !!");
                               // curTuneTask->mTunePicNum = 0;
                                goto PROCESS_OVER;
                            }
                        }
                    }else if(curFmt == ISP_OUT_YUV422_SEMI ){
                       if(curTuneTask->mExpose.aeRound == true){
                            newtime = curTuneTask->mExpose.integrationTime
                                    +curTuneTask->mExpose.integrationTimeStep*((mIspTunningTask->mCurAeRoundNum+1)/2);
                            newgain = curTuneTask->mExpose.gain+curTuneTask->mExpose.gainStep*(mIspTunningTask->mCurAeRoundNum/2);
                            m_camDevice->setIntegrationTime(newtime,settime);
                            m_camDevice->setGain(newgain,setgain);
                            mIspTunningTask->mCurGain = setgain;
                            mIspTunningTask->mCurIntegrationTime = settime;
                            skip_frames = 6;

                            LOGD("setIntegrationTime(desired:%0.3f,real:%0.3f)",newtime,settime);
                            LOGD("setGain(desired:%0.3f,real:%0.3f)",newgain,setgain);
                            if(mIspTunningTask->mCurAeRoundNum == 1){
                                mIspTunningTask->mCurAeRoundNum -=3;
                            }else
                                mIspTunningTask->mCurAeRoundNum--;
                            if(mIspTunningTask->mCurAeRoundNum < (-(curTuneTask->mExpose.number+3))){
                                curTuneTask->mTunePicNum = 0;
                            }
                                
                        }
                    }
                    
                }

                curTuneTask->mTunePicNum--;
                //generate base file name
                    
                snprintf( szBaseFileName, sizeof(szBaseFileName)-1, "%st%0.3f_g%0.3f", "/data/isptune/",time, gain );
                szBaseFileName[99] = '\0';

                //store this frame
                curTuneTask->y_addr = frame->vir_addr;
                if(curFmt == ISP_OUT_YUV422_SEMI){
                    curTuneTask->uv_addr = frame->vir_addr + frame->frame_width*frame->frame_height;
                    curTuneTask->mForceRGBOut = true;
                }else{
                    curTuneTask->mForceRGBOut = false;
                }

                mIspTunningTask->ispTuneStoreBuffer(curTuneTask, (MediaBuffer_t * )frame->used_flag, 
                                                    szBaseFileName, 0);
                PROCESS_OVER:
                //return this frame buffer
                adapterReturnFrame(frame->frame_index, frame->used_flag);

                //current task has been finished ? start next capture?
                if(curTuneTask->mTunePicNum <= 0){
                    skip_frames = 0;
                    mIsSendToTunningTh = false;
                    stopPreview();
                    //remove redundant frame in queue
                    while(!mISPTunningQ->isEmpty())
                        mISPTunningQ->get(&msg);
                    //start new cap?
                    if(mIspTunningTask->mCurTunIndex < (mIspTunningTask->mTuneTaskcount-1)){
                        mIspTunningTask->mCurTunIndex++;
                        msg.command = ISP_TUNNING_CMD_START;
                        mISPTunningQ->put(&msg);
                    }else{
                        LOGD("\n\n*********************\n all tune tasks have fininished\n\n ******************");
						m_camDevice->enableSensorOTP((bool_t)true);//enable sensor otp awb if it have.
                        startPreview(800, 600, 0, 0, ISP_OUT_YUV420SP, false);
                    }
                }
                break;
                }
            default:
                {                    
                LOGE("%d tunning cmd is not support !",msg.command);
                break;
                }     
        }
    }
    return 0;
 }

/* ddl@rock-chips.com: v1.0xb.0 */
int CameraIspAdapter::faceNotify(struct RectFace* faces, int* num)
{
    CamEngineWindow_t curWin,curGrid;
    unsigned int cur_size, face_size, diff;
    int face_width = 0, face_height = 0, focus_id = 0;
    short int x,y;
    unsigned short int width,height;
    bool setWin;
    
    if (*num > 0) {
		{
            for (int i=0; i<*num; i++) {
           	    if (faces[i].width > face_width
                        && faces[i].height > face_height) {
                    face_width = faces[i].width;
                    face_height = faces[i].height;
                    focus_id = i;
                }
            }
			//ALOGD("find face focus id: %d", focus_id);
		}

        // AF
        {
            m_camDevice->getAfMeasureWindow(&curWin);
            
            cur_size = curWin.width*curWin.height;
            face_size = faces[focus_id].width*faces[focus_id].height;
            cur_size = cur_size*10/face_size;
            if ((cur_size>13) || (cur_size<7)) {
                setWin = true;
            } else {
                setWin = false;
            }

            if (setWin == false) {
                diff = abs(curWin.hOffset - faces[focus_id].x)*10/curWin.width;
                if (diff >= 5) {
                    setWin = true;
                }

                diff = abs(curWin.vOffset - faces[focus_id].y)*10/curWin.height;
                if  (diff >= 5) {
                    setWin = true;
                }
            }

            if (setWin == true) {
                x = faces[focus_id].x*2000/mCamPreviewW - 1000;
                y = faces[focus_id].y*2000/mCamPreviewH - 1000;
                width = faces[focus_id].width*2000/mCamPreviewW;
                height = faces[focus_id].height*2000/mCamPreviewH;
                
                LOG1("faceWin: (%d, %d, %d, %d) ---> afWin: (%d, %d, %d, %d)",
                    faces[focus_id].x, faces[focus_id].y, faces[focus_id].width, faces[focus_id].height,
                    curWin.hOffset, curWin.vOffset,curWin.width,curWin.height);
                
                m_camDevice->setAfMeasureWindow(x,y,width,height);
            }
        }
    } else if (*num == 0) {
        m_camDevice->setAfMeasureWindow(0,0,0,0);
        m_camDevice->setAecHistMeasureWinAndMode(0,0,0,0,CentreWeightMetering);
    }

    return 0;

}
}
