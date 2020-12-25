package com.android.settings.display;

import android.R.integer;
import android.util.Log;

import com.android.settings.utils.ReflectUtils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Drm Display Setting.
 */

public class DrmDisplaySetting {
    private final static String TAG = "DrmDisplaySetting";

    public final static int DISPLAY_TYPE_MAIN = 0;
    public final static int DISPLAY_TYPE_AUX = 1;

    private static void logd(String text) {
        Log.d(TAG, TAG + " - " + text);
    }

    public static List<DisplayInfo> getDisplayInfoList() {
        List<DisplayInfo> displayInfos = new ArrayList<DisplayInfo>();
        Object rkDisplayOutputManager = null;

        try {
            rkDisplayOutputManager = Class.forName("android.os.RkDisplayOutputManager").newInstance();
            logd("getDisplayInfoList->rkDisplayOutputManager->name:" + rkDisplayOutputManager.getClass().getName());
        } catch (Exception e) {
        }
        logd(" getDisplayInfoList 1");
        int[] mainTypes = (int[]) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getIfaceList", new Class[]{int.class}, new Object[]{DISPLAY_TYPE_MAIN});
        logd(" getDisplayInfoList 2");
        int[] externalTypes = (int[]) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getIfaceList", new Class[]{int.class}, new Object[]{DISPLAY_TYPE_MAIN});
        logd(" getDisplayInfoList 3");

        if (mainTypes != null && mainTypes.length > 0) {
            int currMainType = (Integer) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getCurrentInterface", new Class[]{int.class}, new Object[]{DISPLAY_TYPE_MAIN});
            DisplayInfo displayInfo = new DisplayInfo();
            displayInfo.setDisplayId(DISPLAY_TYPE_MAIN);
            logd(" getDisplayInfoList 4");
            displayInfo.setDescription((String) ReflectUtils.invokeMethod(rkDisplayOutputManager, "typetoface", new Class[]{int.class}, new Object[]{currMainType}));
            logd(" getDisplayInfoList 5");
            displayInfo.setType(currMainType);
            String[] orginModes = (String[]) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getModeList", new Class[]{int.class, int.class}, new Object[]{DISPLAY_TYPE_MAIN, currMainType});
            orginModes = filterOrginModes(orginModes);
            displayInfo.setOrginModes(orginModes);
            displayInfo.setModes(getFilterModeList(orginModes));
            logd(" getDisplayInfoList 6");
            displayInfos.add(displayInfo);
        }
        if (externalTypes != null && externalTypes.length > 0) {
            int currExternalType = (Integer) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getCurrentInterface", new Class[]{int.class}, new Object[]{1});
            DisplayInfo displayInfo = new DisplayInfo();
            displayInfo.setType(currExternalType);
            String[] orginModes = (String[]) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getModeList", new Class[]{int.class, int.class}, new Object[]{1, externalTypes});
            orginModes = filterOrginModes(orginModes);
            displayInfo.setOrginModes(orginModes);
            displayInfo.setModes(getFilterModeList(orginModes));
            displayInfo.setDescription((String) ReflectUtils.invokeMethod(rkDisplayOutputManager, "typetoface", new Class[]{int.class}, new Integer[]{currExternalType}));
            displayInfo.setDisplayId(1);
            displayInfos.add(displayInfo);
        }
        return displayInfos;
    }

    public static List<String> getDisplayModes(DisplayInfo di) {
        List<String> res = new ArrayList<String>();
        if (di.getDisplayId() == DISPLAY_TYPE_MAIN) {
            di = getMainDisplayInfo();
        } else if (di.getDisplayId() == DISPLAY_TYPE_AUX) {
            di = getAuxDisplayInfo();
        }
        if (di != null) {
            String[] modes = di.getOrginModes();
            if (modes != null && modes.length != 0) {
                res = Arrays.asList(modes);
            }
        }
        return res;
    }

    public static String getCurDisplayMode(DisplayInfo di) {
        if (di.getDisplayId() == DISPLAY_TYPE_MAIN) {
            logd("DrmDisplaySetting getCurDisplayMode DISPLAY_TYPE_MAIN" + System.currentTimeMillis());
            return getCurMainMode();
        } else if (di.getDisplayId() == DISPLAY_TYPE_AUX) {
            logd("DrmDisplaySetting getCurDisplayMode DISPLAY_TYPE_AUX " + System.currentTimeMillis());
            return getCurAuxMode();
        }
        return null;
    }

    public static String getCurMainMode() {
        return getMainMode();
    }

    public static String getCurAuxMode() {
        return getAuxMode();
    }

    public static void setDisplayModeTemp(DisplayInfo di, int index) {
        String mode = getDisplayModes(di).get(index);
        setDisplayModeTemp(di, mode);
    }

    public static void setDisplayModeTemp(DisplayInfo di, String mode) {
        if (di.getDisplayId() == DISPLAY_TYPE_MAIN) {
            setMainModeTemp(mode);
        } else if (di.getDisplayId() == DISPLAY_TYPE_AUX) {
            setAuxModeTemp(mode);
        }
    }

    public static void saveConfig() {
        Object rkDisplayOutputManager = null;
        try {
            rkDisplayOutputManager = Class.forName("android.os.RkDisplayOutputManager").newInstance();
        } catch (Exception e) {
            // no handle
        }
        if (rkDisplayOutputManager != null) {
            int result = (Integer) ReflectUtils.invokeMethodNoParameter(rkDisplayOutputManager, "saveConfig");
        }
    }

    public static void updateDisplayInfos() {
        Object rkDisplayOutputManager = null;
        try {
            rkDisplayOutputManager = Class.forName("android.os.RkDisplayOutputManager").newInstance();
        } catch (Exception e) {
            // no handle
        }
        if (rkDisplayOutputManager != null) {
            logd("updateDisplayInfos");
            int result = (Integer) ReflectUtils.invokeMethodNoParameter(rkDisplayOutputManager, "updateDisplayInfos");
        }
    }

    public static void confirmSaveDisplayMode(DisplayInfo di, boolean isSave) {
        if (di == null) {
            return;
        }
        if (di.getDisplayId() == DISPLAY_TYPE_MAIN) {
            confirmSaveMainMode(isSave);
        } else if (di.getDisplayId() == DISPLAY_TYPE_AUX) {
            confirmSaveAuxMode(isSave);
        }
        saveConfig();
    }

    private static String tmpSetMainMode = null;
    private static String curSetMainMode = "Auto";

    public static DisplayInfo getDisplayInfo(int displayId) {
        if (DISPLAY_TYPE_MAIN == displayId) {
            return getMainDisplayInfo();
        } else if (DISPLAY_TYPE_AUX == displayId) {
            return getAuxDisplayInfo();
        }
        return null;
    }

    public static DisplayInfo getMainDisplayInfo() {
        Object rkDisplayOutputManager = null;
        try {
            rkDisplayOutputManager = Class.forName("android.os.RkDisplayOutputManager").newInstance();
            logd("getDisplayInfoList->rkDisplayOutputManager->name:" + rkDisplayOutputManager.getClass().getName());
        } catch (Exception e) {
        }
        if (rkDisplayOutputManager == null)
            return null;
        logd(" getMainDisplayInfo 1");
        int[] mainTypes = (int[]) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getIfaceList", new Class[]{int.class}, new Object[]{DISPLAY_TYPE_MAIN});
        logd(" getMainDisplayInfo 2");
        if (mainTypes != null && mainTypes.length > 0) {
            int currMainType = (Integer) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getCurrentInterface", new Class[]{int.class}, new Object[]{DISPLAY_TYPE_MAIN});
            DisplayInfo displayInfo = new DisplayInfo();
            displayInfo.setDisplayId(DISPLAY_TYPE_MAIN);
            logd(" getMainDisplayInfo 3");
            displayInfo.setDescription((String) ReflectUtils.invokeMethod(rkDisplayOutputManager, "typetoface", new Class[]{int.class}, new Object[]{currMainType}));
            logd(" getMainDisplayInfo 4");
            displayInfo.setType(currMainType);
            String[] orginModes = (String[]) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getModeList", new Class[]{int.class, int.class}, new Object[]{DISPLAY_TYPE_MAIN, currMainType});
            orginModes = filterOrginModes(orginModes);
            displayInfo.setOrginModes(orginModes);
            displayInfo.setModes(getFilterModeList(orginModes));
            logd(" getMainDisplayInfo 5");
            return displayInfo;
        }
        return null;
    }

    private static String getMainMode() {
        Object rkDisplayOutputManager = null;
        try {
            rkDisplayOutputManager = Class.forName("android.os.RkDisplayOutputManager").newInstance();
            logd("getDisplayInfoList->rkDisplayOutputManager->name:" + rkDisplayOutputManager.getClass().getName());
        } catch (Exception e) {
            // no handle
        }
        if (rkDisplayOutputManager == null)
            return null;
        logd(" getMainMode 1");
        int[] mainTypes = (int[]) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getIfaceList", new Class[]{int.class}, new Object[]{DISPLAY_TYPE_MAIN});
        logd(" getMainMode 2");
        if (mainTypes != null && mainTypes.length > 0) {
            int currMainType = (Integer) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getCurrentInterface", new Class[]{int.class}, new Object[]{DISPLAY_TYPE_MAIN});
            return (String) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getCurrentMode", new Class[]{int.class, int.class}, new Object[]{DISPLAY_TYPE_MAIN, currMainType});
        }
        return null;
    }

    private static void setMainModeTemp(String mode) {
        setMainMode(mode);
        tmpSetMainMode = mode;
    }

    private static void confirmSaveMainMode(boolean isSave) {
        if (tmpSetMainMode == null) {
            return;
        }
        if (isSave) {
            curSetMainMode = tmpSetMainMode;
        } else {
            setMainMode(curSetMainMode);
            tmpSetMainMode = null;
        }
    }

    private static void setMainMode(String mode) {
        Object rkDisplayOutputManager = null;
        try {
            rkDisplayOutputManager = Class.forName("android.os.RkDisplayOutputManager").newInstance();
            logd("getDisplayInfoList->rkDisplayOutputManager->name:" + rkDisplayOutputManager.getClass().getName());
        } catch (Exception e) {
        }
        if (rkDisplayOutputManager == null)
            return;
        logd(" setMainMode 1");
        int[] mainTypes = (int[]) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getIfaceList", new Class[]{int.class}, new Object[]{DISPLAY_TYPE_MAIN});
        logd(" setMainMode 2");
        if (mainTypes != null && mainTypes.length > 0) {
            logd(" setMainMode mode = " + mode);
            int currMainType = (Integer) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getCurrentInterface", new Class[]{int.class}, new Object[]{DISPLAY_TYPE_MAIN});
            ReflectUtils.invokeMethod(rkDisplayOutputManager, "setMode", new Class[]{int.class, int.class, String.class}, new Object[]{DISPLAY_TYPE_MAIN, currMainType, mode});
        }
        logd(" setMainMode 3");
    }

    private static String tmpSetAuxMode = null;
    private static String curSetAuxMode = "1920x1080p60";

    public static DisplayInfo getAuxDisplayInfo() {
        Object rkDisplayOutputManager = null;
        try {
            rkDisplayOutputManager = Class.forName("android.os.RkDisplayOutputManager").newInstance();
            logd("getDisplayInfoList->rkDisplayOutputManager->name:" + rkDisplayOutputManager.getClass().getName());
        } catch (Exception e) {
        }
        logd(" getAuxDisplayInfo 1");
        int[] externalTypes = (int[]) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getIfaceList", new Class[]{int.class}, new Object[]{DISPLAY_TYPE_AUX});
        logd(" getAuxDisplayInfo 2");
        if (externalTypes != null && externalTypes.length > 0) {
            int currMainType = (Integer) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getCurrentInterface", new Class[]{int.class}, new Object[]{DISPLAY_TYPE_AUX});
            DisplayInfo displayInfo = new DisplayInfo();
            displayInfo.setDisplayId(DISPLAY_TYPE_AUX);
            logd(" getAuxDisplayInfo 3");
            displayInfo.setDescription((String) ReflectUtils.invokeMethod(rkDisplayOutputManager, "typetoface", new Class[]{int.class}, new Object[]{currMainType}));
            logd(" getAuxDisplayInfo 4");
            displayInfo.setType(currMainType);
            String[] orginModes = (String[]) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getModeList", new Class[]{int.class, int.class}, new Object[]{DISPLAY_TYPE_AUX, currMainType});
            orginModes = filterOrginModes(orginModes);
            displayInfo.setOrginModes(orginModes);
            displayInfo.setModes(getFilterModeList(orginModes));
            logd(" getAuxDisplayInfo 5");
            return displayInfo;
        }
        return null;
    }

    private static String getAuxMode() {
        Object rkDisplayOutputManager = null;
        try {
            rkDisplayOutputManager = Class.forName("android.os.RkDisplayOutputManager").newInstance();
            logd("getDisplayInfoList->rkDisplayOutputManager->name:" + rkDisplayOutputManager.getClass().getName());
        } catch (Exception e) {
            // no handle
        }
        if (rkDisplayOutputManager == null)
            return null;
        logd(" getAuxMode 1");
        int[] mainTypes = (int[]) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getIfaceList", new Class[]{int.class}, new Object[]{DISPLAY_TYPE_AUX});
        logd(" getAuxMode 2");
        if (mainTypes != null && mainTypes.length > 0) {
            int currMainType = (Integer) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getCurrentInterface", new Class[]{int.class}, new Object[]{DISPLAY_TYPE_AUX});
            return (String) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getCurrentMode", new Class[]{int.class, int.class}, new Object[]{DISPLAY_TYPE_AUX, currMainType});
        }
        return null;
    }

    private static void setAuxModeTemp(String reso) {
        setAuxMode(reso);
        tmpSetAuxMode = reso;
    }

    private static void confirmSaveAuxMode(boolean isSave) {
        if (tmpSetAuxMode == null) {
            return;
        }
        if (isSave) {
            curSetAuxMode = tmpSetAuxMode;
        } else {
            setAuxMode(curSetAuxMode);
            tmpSetAuxMode = null;
        }
    }

    private static void setAuxMode(String reso) {
        Object rkDisplayOutputManager = null;
        try {
            rkDisplayOutputManager = Class.forName("android.os.RkDisplayOutputManager").newInstance();
            logd("getDisplayInfoList->rkDisplayOutputManager->name:" + rkDisplayOutputManager.getClass().getName());
        } catch (Exception e) {
        }
        if (rkDisplayOutputManager == null)
            return;
        logd(" setAuxMode 1");
        int[] mainTypes = (int[]) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getIfaceList", new Class[]{int.class}, new Object[]{DISPLAY_TYPE_AUX});
        logd(" setAuxMode 2");
        if (mainTypes != null && mainTypes.length > 0) {
            int currMainType = (Integer) ReflectUtils.invokeMethod(rkDisplayOutputManager, "getCurrentInterface", new Class[]{int.class}, new Object[]{DISPLAY_TYPE_AUX});
            ReflectUtils.invokeMethod(rkDisplayOutputManager, "setMode", new Class[]{int.class, int.class, String.class}, new Object[]{DISPLAY_TYPE_AUX, currMainType, reso});
        }
    }

    private static String[] filterOrginModes(String[] modes) {
        if (modes == null)
            return null;
        List<String> filterModeList = new ArrayList<String>();
        List<String> resModeList = new ArrayList<String>();
        for (int i = 0; i < modes.length; ++i) {
            logd("filterOrginModes->mode:" + modes[i]);
            String itemMode = modes[i];
            int endIndex = itemMode.indexOf("-");
            if (endIndex > 0)
                itemMode = itemMode.substring(0, endIndex);
            if (!resModeList.contains(itemMode)) {
                resModeList.add(itemMode);
                if (!filterModeList.contains(modes[i]))
                    filterModeList.add(modes[i]);
            }
        }
        return filterModeList.toArray(new String[0]);
    }

    private static String[] getFilterModeList(String[] modes) {
        if (modes == null)
            return null;
        String[] filterModes = new String[modes.length];
        for (int i = 0; i < modes.length; ++i) {
            String itemMode = modes[i];
            int endIndex = itemMode.indexOf("-");
            if (endIndex > 0)
                itemMode = itemMode.substring(0, endIndex);
            filterModes[i] = itemMode;
        }
        return filterModes;
    }
}
