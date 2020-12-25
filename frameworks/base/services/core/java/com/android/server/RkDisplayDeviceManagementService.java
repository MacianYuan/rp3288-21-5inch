/* $_FOR_ROCKCHIP_RBOX_$ */
//$_rbox_$_modify_$_zhengyang_20120220: Rbox android display management service

/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.server;


import android.content.Context;
import android.content.Intent;
import android.os.IRkDisplayDeviceManagementService;
import android.os.SystemProperties;
import android.util.Slog;
import android.util.Log;
import java.util.ArrayList;
import java.util.List;
import android.graphics.Rect;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;

import com.android.server.rkdisplay.RkDisplayModes;
import com.android.server.rkdisplay.HdmiReceiver;

/**
 * @hide
 */

class RkDisplayDeviceManagementService extends IRkDisplayDeviceManagementService.Stub {
    private static final String TAG = "RkDisplayDeviceManagementService";
    private static final String ACTION_PLUGGED = "android.intent.action.HDMI_PLUGGED";
    private static final boolean DBG = true;
    private static final String DISPLAYD_TAG = "DisplaydConnector";
    private final RkDisplayModes mdrmModes;
    private final HdmiReceiver mHdmiReceiver;
    private final int MAIN_DISPLAY = 0;
    private final int AUX_DISPLAY = 1;

    public final int DISPLAY_OVERSCAN_X = 0;
    public final int DISPLAY_OVERSCAN_Y = 1;
    public final int DISPLAY_OVERSCAN_LEFT = 2;
    public final int DISPLAY_OVERSCAN_RIGHT = 3;
    public final int DISPLAY_OVERSCAN_TOP = 4;
    public final int DISPLAY_OVERSCAN_BOTTOM = 5;
    public final int DISPLAY_OVERSCAN_ALL = 6;

    private final int DEFAULT_BRIGHTNESS = 50;
    private final int DEFAULT_CONTRAST = 50;
    private final int DEFAULT_SATURATION = 50;
    private final int DEFAULT_HUE = 50;

    private int timeline=0;
    /**
     * Binder context for this service
     */
    private Context mContext;


    /**
     * Constructs a new NetworkManagementService instance
     *
     * @param context  Binder context for this service
     */
    public RkDisplayDeviceManagementService(Context context) {
        mContext = context;
        mdrmModes = new RkDisplayModes();
        mdrmModes.init();
        IntentFilter hdmiFilter = new IntentFilter();
        hdmiFilter.addAction(ACTION_PLUGGED);
        mHdmiReceiver = new HdmiReceiver(mdrmModes);
        mContext.registerReceiver(mHdmiReceiver,hdmiFilter);
    }

    public String[] listInterfaces(int display) {
        String[] ifaces = new String[1];
        ifaces[0] = mdrmModes.getbuild_in(display);
        return ifaces;
    }

    public String getCurrentInterface(int display){
        return mdrmModes.getbuild_in(display);
    }

    public int getDisplayNumbers(){
        return mdrmModes.getNumConnectors();
    }

    public String[] getModelist(int display, String iface){
        List<String> hdmiResoList = mdrmModes.getModeList(display);
        if (hdmiResoList != null)
            return hdmiResoList.toArray(new String[hdmiResoList.size()]);
        else
            return  null;
    }

    public String getMode(int display, String iface){
        String mode = mdrmModes.getCurMode(display, iface);
        Log.d(TAG, "getMode: " + mode + " display " +display);
        return mode;
    }

    public int getDpyConnState(int display){
        return mdrmModes.getConnectionState(display);
    }

    public void setMode(int display, String iface, String mode) {
        boolean isSameProperty = false;
        String lastMode;

        lastMode = mdrmModes.getCurMode(display, iface);
        mdrmModes.setMode(display, iface, mode);
        isSameProperty = lastMode.equals(mode);
        Log.d(TAG, "setMode ---- lastMode: "+lastMode + "  mode:"+ mode +"  isSameProperty  "+ isSameProperty);
        Log.v(TAG, "lastMode " + lastMode + " display " +display);
        if (!isSameProperty || mode.equals("Auto")) {
            timeline++;
            SystemProperties.set("sys.display.timeline", Integer.toString(timeline));
        }
    }

    public String[] getSupportCorlorList(int display, String iface){
        List<String> corlorList = mdrmModes.getSupportCorlorList(display);
        if (corlorList != null)
            return corlorList.toArray(new String[corlorList.size()]);
        else
            return null;
    }

    public String getCurColorMode(int display, String iface){
        String colorMode = mdrmModes.getCurColorMode(display);
        return colorMode;
    }

    public void setColorMode(int display, String iface, String format){
        boolean isSameProperty = false;

        if (display == MAIN_DISPLAY) {
            String lastColor = SystemProperties.get("persist.sys.color.main");
            isSameProperty = lastColor.equals(format);
            Log.d(TAG, "dpy:"+display+"   lastColor:"+lastColor+"	 format:"+format+"	 update:"+isSameProperty);
            SystemProperties.set("persist.sys.color.main", format);
        } else {
            String lastColor = SystemProperties.get("persist.sys.color.aux");
            isSameProperty = lastColor.equals(format);
            Log.d(TAG, "dpy:"+display+"   iface:"+iface+"	 format:"+format+"	 update:"+isSameProperty);
            SystemProperties.set("persist.sys.color.aux", format);
        }
        if (!isSameProperty) {
            timeline++;
            SystemProperties.set("sys.display.timeline", Integer.toString(timeline));
        }
    }

    public void setHdrMode(int display, String iface, int hdrMode){
        boolean isSameProperty = false;

        if (display == MAIN_DISPLAY) {
            String lastHdrMode = SystemProperties.get("persist.sys.hdr_mode.main");
            isSameProperty = lastHdrMode.equals(Integer.toString(hdrMode));
            SystemProperties.set("persist.sys.hdr_mode.main", Integer.toString(hdrMode));
        } else {
            String lastHdrMode = SystemProperties.get("persist.sys.hdr_mode.aux");
            isSameProperty = lastHdrMode.equals(Integer.toString(hdrMode));
            SystemProperties.set("persist.sys.hdr_mode.aux", Integer.toString(hdrMode));
        }

        if (!isSameProperty) {
            timeline++;
            SystemProperties.set("sys.display.timeline", Integer.toString(timeline));
        }
    }

    public void setScreenScale(int display, int direction, int value){
        String OverScan;
        StringBuilder builder = new StringBuilder();
        boolean isSameProperty = false;

        if(display == MAIN_DISPLAY)
            OverScan = SystemProperties.get("persist.sys.overscan.main", "overscan 100,100,100,100");
        else if(display == AUX_DISPLAY)
            OverScan = SystemProperties.get("persist.sys.overscan.aux", "overscan 100,100,100,100");
        else
            return;

        String split = OverScan.substring(9);
        String [] tmpValue = split.split("\\,");
        Rect rect = new Rect(100,100,100,100);
        if (tmpValue != null) {
            rect.left = Integer.parseInt(tmpValue[0]);
            rect.top = Integer.parseInt(tmpValue[1]);
            rect.right = Integer.parseInt(tmpValue[2]);
            rect.bottom = Integer.parseInt(tmpValue[3]);
        }
        if (value > 100)
            value = 100;
        else if (value < 50)
            value = 50;
        if(direction == DISPLAY_OVERSCAN_X) {
            rect.left = value;
            rect.right = value;
        } else if(direction == DISPLAY_OVERSCAN_Y) {
            rect.top = value;
            rect.bottom = value;
        } else if(direction == DISPLAY_OVERSCAN_LEFT)
            rect.left = value;
        else if(direction == DISPLAY_OVERSCAN_RIGHT)
            rect.right = value;
        else if(direction == DISPLAY_OVERSCAN_TOP)
            rect.top = value;
        else if(direction == DISPLAY_OVERSCAN_BOTTOM)
            rect.bottom = value;

        builder.append("overscan ").append(rect.left).append(",").append(rect.top).append(",").append(rect.right)
        .append(",").append(rect.bottom);

        if (display == MAIN_DISPLAY) {
            isSameProperty = OverScan.equals(builder.toString());
            SystemProperties.set("persist.sys.overscan.main", builder.toString());
        } else {
            isSameProperty = OverScan.equals(builder.toString());
            SystemProperties.set("persist.sys.overscan.aux", builder.toString());
        }
        Log.d(TAG, "dpy:"+display+" direction:"+direction+"value:"+value+" result:"+builder.toString()+"update:"+isSameProperty);
        if (!isSameProperty) {
            timeline++;
            SystemProperties.set("sys.display.timeline", Integer.toString(timeline));
        }
    }

    public void setDisplaySize(int display, int width, int height){
    }

    public int get3DModes(int display, String iface){
        return 0;
    }

    public int getCur3DMode(int display, String iface){
        return 0;
    }

    public void set3DMode(int display, String iface, int mode) {
    }

    public void setBrightness(int display, int brightness) {
        StringBuilder builder = new StringBuilder();
        boolean isSameProperty = false;

        if (brightness >= 0 && brightness <= 100)
            builder.append(brightness);
        else
            builder.append(DEFAULT_BRIGHTNESS);

        if (display == MAIN_DISPLAY) {
            String lastBrightness = SystemProperties.get("persist.sys.brightness.main");
            isSameProperty = lastBrightness.equals(builder.toString());
            SystemProperties.set("persist.sys.brightness.main", builder.toString());
        } else {
            String lastBrightness = SystemProperties.get("persist.sys.brightness.aux");
            isSameProperty = lastBrightness.equals(builder.toString());
            SystemProperties.set("persist.sys.brightness.aux", builder.toString());
        }
        if (!isSameProperty) {
            timeline++;
            SystemProperties.set("sys.display.timeline", Integer.toString(timeline));
        }
    }

    public void setContrast(int display, int contrast)  {
        StringBuilder builder = new StringBuilder();
        boolean isSameProperty = false;

        if (contrast >= 0 && contrast <= 100)
            builder.append(contrast);
        else
            builder.append(DEFAULT_CONTRAST);
        if (display == MAIN_DISPLAY) {
            String lastContrast = SystemProperties.get("persist.sys.contrast.main");
            isSameProperty = lastContrast.equals(builder.toString());
            SystemProperties.set("persist.sys.contrast.main", builder.toString());
        } else {
            String lastContrast = SystemProperties.get("persist.sys.contrast.aux");
            isSameProperty = lastContrast.equals(builder.toString());
            SystemProperties.set("persist.sys.contrast.aux", builder.toString());
        }

        if (!isSameProperty) {
            timeline++;
            SystemProperties.set("sys.display.timeline", Integer.toString(timeline));
        }
    }
//"persist.sys.saturation.main"
    public void setSaturation(int display, int saturation) {
        StringBuilder builder = new StringBuilder();
        boolean isSameProperty = false;

        if (saturation >= 0 && saturation <= 100)
            builder.append(saturation);
        else
            builder.append(DEFAULT_SATURATION);
        if (display == MAIN_DISPLAY) {
            String lastSaturation = SystemProperties.get("persist.sys.saturation.main");
            isSameProperty = lastSaturation.equals(builder.toString());
            SystemProperties.set("persist.sys.saturation.main", builder.toString());
        } else {
            String lastSaturation = SystemProperties.get("persist.sys.saturation.aux");
            isSameProperty = lastSaturation.equals(builder.toString());
            SystemProperties.set("persist.sys.saturation.aux", builder.toString());
        }
        if (!isSameProperty) {
            timeline++;
            SystemProperties.set("sys.display.timeline", Integer.toString(timeline));
        }
    }

    public void setHue(int display, int degree){
        StringBuilder builder = new StringBuilder();
        boolean isSameProperty = false;

        if (degree >= 0 && degree <= 100)
            builder.append(degree);
        else
            builder.append(DEFAULT_HUE);
        if (display == MAIN_DISPLAY) {
            String lastHue = SystemProperties.get("persist.sys.hue.main");
            isSameProperty = lastHue.equals(builder.toString());
            SystemProperties.set("persist.sys.hue.main", builder.toString());
        } else {
            String lastHue = SystemProperties.get("persist.sys.hue.aux");
            isSameProperty = lastHue.equals(builder.toString());
            SystemProperties.set("persist.sys.hue.aux", builder.toString());
        }

        if (!isSameProperty) {
            timeline++;
            SystemProperties.set("sys.display.timeline", Integer.toString(timeline));
        }
    }

    public int[] getBcsh(int display) {
        int[] bcsh = new int[4];
        bcsh = mdrmModes.getBcsh(display);
        Log.d(TAG, "getBcsh: " +  bcsh[0] + " " + bcsh[1] + " " +  bcsh[2] + " " + bcsh[3]);
        return bcsh;
    }

    public int[] getOverscan(int display) {
        int[] mOverscan = new int[4];
        mOverscan = mdrmModes.getOverscan(display);
        Log.d(TAG, "getOverscan: " +  mOverscan[0] + " " + mOverscan[1] + " " +  mOverscan[2] + " " + mOverscan[3]);
        return mOverscan;
    }

    public int setGamma(int dpy,int size,int[] red, int[] green, int[] blue){
        return mdrmModes.setGamma(dpy, size, red, green, blue);
    }
    public void updateDisplayInfos(){
        mdrmModes.updateDisplayInfos();
    }

    public int saveConfig() {
        //mdrmModes.saveConfig();
	SystemProperties.set("persist.sys.saveconfig", "1");
        timeline++;
        SystemProperties.set("sys.display.timeline", Integer.toString(timeline));
        return 0;
    }
}
