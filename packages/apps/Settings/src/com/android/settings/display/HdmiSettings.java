package com.android.settings;

import android.app.DialogFragment;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.display.DisplayManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.support.v14.preference.SwitchPreference;
import android.support.v7.preference.CheckBoxPreference;
import android.support.v7.preference.ListPreference;
import android.support.v7.preference.Preference;
import android.support.v7.preference.PreferenceCategory;
import android.support.v7.preference.PreferenceScreen;
import android.support.v7.preference.Preference.OnPreferenceChangeListener;
import android.util.Log;
import android.view.IWindowManager;
import android.view.LayoutInflater;
import android.view.Surface;
import android.view.View;
import android.view.ViewGroup;

import com.android.internal.logging.nano.MetricsProto.MetricsEvent;
import com.android.settings.display.*;
import com.android.settings.R;
import com.android.settings.SettingsPreferenceFragment;

import static com.android.settings.HdmiSettings.DISPLAY_SHOW_SETTINGS.DOUBLE_SHOW;
import static com.android.settings.HdmiSettings.DISPLAY_SHOW_SETTINGS.ONLY_SHOW_AUX;
import static com.android.settings.HdmiSettings.DISPLAY_SHOW_SETTINGS.ONLY_SHOW_MAIN;

public class HdmiSettings extends SettingsPreferenceFragment
        implements OnPreferenceChangeListener, Preference.OnPreferenceClickListener {
    /**
     * Called when the activity is first created.
     */
    private static final String TAG = "HdmiSettings";
    private static final String KEY_SYSTEM_ROTATION = "system_rotation";
    private static final String KEY_MAIN_CATEGORY = "main_category";
    private static final String KEY_MAIN_SWITCH = "main_switch";
    private static final String KEY_MAIN_RESOLUTION = "main_resolution";
    private static final String KEY_MAIN_SCALE = "main_screen_scale";
    private static final String KEY_AUX_CATEGORY = "aux_category";
    private static final String KEY_AUX_SWITCH = "aux_switch";
    private static final String KEY_AUX_RESOLUTION = "aux_resolution";
    private static final String KEY_AUX_SCALE = "aux_screen_scale";
    private static final String KEY_AUX_SCREEN_VH = "aux_screen_vh";
    private static final String KEY_AUX_SCREEN_VH_LIST = "aux_screen_vhlist";
    private static final String SYS_HDMI_STATE = "sys.hdmi_status.aux";
    private static final String SYS_DP_STATE = "sys.dp_status.aux";
    /**
     * TODO
     * 目前hwc只配置了hdmi和dp的开关，如果是其他的设备，需要配合修改，才能进行开关
     * sys.hdmi_status.aux：/sys/devices/platform/display-subsystem/drm/card0/card0-HDMI-A-1/status
     * sys.hdmi_status.aux：/sys/devices/platform/display-subsystem/drm/card0/card0-DP-1/status
     */
    private String sys_main_state = SYS_HDMI_STATE;
    private String sys_aux_state = SYS_DP_STATE;

    private ListPreference mSystemRotation;
    private PreferenceCategory mMainCategory;
    private SwitchPreference mMainSwitch;
    private ListPreference mMainResolution;
    private Preference mMainScale;
    private PreferenceCategory mAuxCategory;
    private SwitchPreference mAuxSwitch;
    private ListPreference mAuxResolution;
    private Preference mAuxScale;
    private CheckBoxPreference mAuxScreenVH;
    private ListPreference mAuxScreenVHList;
    private Context context;
    private String mOldMainResolution;
    private String mOldAuxResolution;
    protected DisplayInfo mMainDisplayInfo;
    protected DisplayInfo mAuxDisplayInfo;
    private DisplayManager mDisplayManager;
    private DisplayListener mDisplayListener;
    private IWindowManager mWindowManager;
    private DISPLAY_SHOW_SETTINGS mShowSettings = ONLY_SHOW_AUX;

    enum DISPLAY_SHOW_SETTINGS {
        ONLY_SHOW_MAIN,
        ONLY_SHOW_AUX,
        DOUBLE_SHOW
    }

    private final BroadcastReceiver HdmiListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context ctxt, Intent receivedIt) {
            String action = receivedIt.getAction();
            String HDMIINTENT = "android.intent.action.HDMI_PLUGGED";
            if (action.equals(HDMIINTENT)) {
                boolean state = receivedIt.getBooleanExtra("state", false);
                if (state) {
                    Log.d(TAG, "BroadcastReceiver.onReceive() : Connected HDMI-TV");
                } else {
                    Log.d(TAG, "BroadcastReceiver.onReceive() : Disconnected HDMI-TV");
                }
            }
        }
    };

    @Override
    public int getMetricsCategory() {
        return MetricsEvent.DISPLAY;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        context = getActivity();
        mDisplayManager = (DisplayManager) context.getSystemService(Context.DISPLAY_SERVICE);
        mWindowManager = IWindowManager.Stub.asInterface(
                ServiceManager.getService(Context.WINDOW_SERVICE));
        mDisplayListener = new DisplayListener();
        addPreferencesFromResource(R.xml.hdmi_settings);
        int value = SystemProperties.getInt("persist.hdmi.ui.state", 0);
        switch (value) {
            case 0: {
                mShowSettings = ONLY_SHOW_AUX;
                sys_aux_state = SYS_HDMI_STATE;
                break;
            }
            case 1: {
                mShowSettings = ONLY_SHOW_MAIN;
                sys_main_state = SYS_HDMI_STATE;
                break;
            }
            default: {
                mShowSettings = DOUBLE_SHOW;
                String primary = SystemProperties.get("sys.hwc.device.primary", "");
                String extend = SystemProperties.get("sys.hwc.device.extend", "");
                if (primary.contains("HDMI")) {//配置hdmi为主显
                    sys_main_state = SYS_HDMI_STATE;
                    sys_aux_state = SYS_DP_STATE;
                } else if (extend.contains("HDMI")) {//主显不配hdmi,副显配置hdmi
                    sys_aux_state = SYS_HDMI_STATE;
                    sys_main_state = SYS_DP_STATE;
                }
                break;
            }
        }
        init();
        Log.d(TAG, "---------onCreate---------------------");
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        Log.d(TAG, "onCreateView----------------------------------------");
        return super.onCreateView(inflater, container, savedInstanceState);
    }

    @Override
    public void onResume() {
        super.onResume();
        IntentFilter filter = new IntentFilter("android.intent.action.HDMI_PLUGGED");
        getContext().registerReceiver(HdmiListener, filter);
        refreshState();
        mDisplayManager.registerDisplayListener(mDisplayListener, null);
    }

    public void onPause() {
        super.onPause();
        Log.d(TAG, "onPause----------------");
        mDisplayManager.unregisterDisplayListener(mDisplayListener);
        getContext().unregisterReceiver(HdmiListener);
    }

    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy----------------");
    }

    private void init() {
        //boolean showSystemRotation = mShowSettings != DISPLAY_SHOW_SETTINGS.ONLY_SHOW_AUX;
        boolean showSystemRotation = false;
        if (showSystemRotation) {
            mSystemRotation = (ListPreference) findPreference(KEY_SYSTEM_ROTATION);
            mSystemRotation.setOnPreferenceChangeListener(this);
            try {
                int rotation = mWindowManager.getDefaultDisplayRotation();
                switch (rotation) {
                    case Surface.ROTATION_0:
                        mSystemRotation.setValue("0");
                        break;
                    case Surface.ROTATION_90:
                        mSystemRotation.setValue("90");
                        break;
                    case Surface.ROTATION_180:
                        mSystemRotation.setValue("180");
                        break;
                    case Surface.ROTATION_270:
                        mSystemRotation.setValue("270");
                        break;
                    default:
                        mSystemRotation.setValue("0");
                }
            } catch (Exception e) {
                Log.e(TAG, e.toString());
            }
        } else {
            removePreference(KEY_SYSTEM_ROTATION);
        }
        //main
        if (mShowSettings != ONLY_SHOW_AUX) {
            mMainDisplayInfo = getDisplayInfo(0);
            //restore main switch value
            mMainCategory = (PreferenceCategory) findPreference(KEY_MAIN_CATEGORY);
            if (mShowSettings == DOUBLE_SHOW) {
                mMainCategory.setTitle(R.string.screen_main_title);
            }
            String switchState = SystemProperties.get(sys_main_state, "on");
            mMainSwitch = (SwitchPreference) findPreference(KEY_MAIN_SWITCH);
            if ("on".equals(switchState)) {
                mMainSwitch.setChecked(true);
            } else {
                mMainSwitch.setChecked(false);
            }
            mMainSwitch.setOnPreferenceChangeListener(this);
            mMainCategory.removePreference(mMainSwitch);
            mMainResolution = (ListPreference) findPreference(KEY_MAIN_RESOLUTION);
            mMainResolution.setOnPreferenceChangeListener(this);
            mMainResolution.setOnPreferenceClickListener(this);
            if (mMainDisplayInfo != null) {
                mMainResolution.setEntries(DrmDisplaySetting.getDisplayModes(mMainDisplayInfo).toArray(new String[0]));
                mMainResolution.setEntryValues(DrmDisplaySetting.getDisplayModes(mMainDisplayInfo).toArray(new String[0]));
            }
            mMainScale = findPreference(KEY_MAIN_SCALE);
            mMainScale.setOnPreferenceClickListener(this);
            //mMainCategory.removePreference(mMainSwitch);
        } else {
            removePreference(KEY_MAIN_CATEGORY);
        }


        //aux
        if (mShowSettings != ONLY_SHOW_MAIN) {
            mAuxDisplayInfo = getDisplayInfo(1);
            mAuxCategory = (PreferenceCategory) findPreference(KEY_AUX_CATEGORY);
            if (mShowSettings == DOUBLE_SHOW) {
                mAuxCategory.setTitle(R.string.screen_aux_title);
            }
            mAuxSwitch = (SwitchPreference) findPreference(KEY_AUX_SWITCH);
            String switchState = SystemProperties.get(sys_aux_state, "on");
            if ("on".equals(switchState)) {
                mAuxSwitch.setChecked(true);
            } else {
                mAuxSwitch.setChecked(false);
            }
            mAuxSwitch.setOnPreferenceChangeListener(this);
            mAuxCategory.removePreference(mAuxSwitch);
            mAuxResolution = (ListPreference) findPreference(KEY_AUX_RESOLUTION);
            mAuxResolution.setOnPreferenceChangeListener(this);
            mAuxResolution.setOnPreferenceClickListener(this);
            if (mAuxDisplayInfo != null) {
                mAuxResolution.setEntries(DrmDisplaySetting.getDisplayModes(mAuxDisplayInfo).toArray(new String[0]));
                mAuxResolution.setEntryValues(DrmDisplaySetting.getDisplayModes(mAuxDisplayInfo).toArray(new String[0]));
            }
            mAuxScale = findPreference(KEY_AUX_SCALE);
            mAuxScale.setOnPreferenceClickListener(this);

            mAuxScreenVH = (CheckBoxPreference) findPreference(KEY_AUX_SCREEN_VH);
            mAuxScreenVH.setChecked(SystemProperties.getBoolean("persist.sys.rotation.efull", false));
            mAuxScreenVH.setOnPreferenceChangeListener(this);
            mAuxCategory.removePreference(mAuxScreenVH);
            mAuxScreenVHList = (ListPreference) findPreference(KEY_AUX_SCREEN_VH_LIST);
            mAuxScreenVHList.setOnPreferenceChangeListener(this);
            mAuxScreenVHList.setOnPreferenceClickListener(this);
            mAuxCategory.removePreference(mAuxScreenVHList);
        } else {
            removePreference(KEY_AUX_CATEGORY);
        }
    }

    protected DisplayInfo getDisplayInfo(int displayId) {
        DrmDisplaySetting.updateDisplayInfos();
        return DrmDisplaySetting.getDisplayInfo(displayId);
    }

    /**
     * 获取当前分辨率值
     */
    public void updateMainResolutionValue() {
        String resolutionValue = null;
        mMainDisplayInfo = getDisplayInfo(0);
        if (mMainDisplayInfo != null) {
            resolutionValue = DrmDisplaySetting.getCurDisplayMode(mMainDisplayInfo);
        }
        Log.i(TAG, "main resolutionValue:" + resolutionValue);
        mOldMainResolution = resolutionValue;
        if (resolutionValue != null) {
            mMainResolution.setValue(resolutionValue);
        }
    }

    public void updateAuxResolutionValue() {
        String resolutionValue = null;
        mAuxDisplayInfo = getDisplayInfo(1);
        if (mAuxDisplayInfo != null) {
            resolutionValue = DrmDisplaySetting.getCurDisplayMode(mAuxDisplayInfo);
        }
        Log.i(TAG, "aux resolutionValue:" + resolutionValue);
        mOldAuxResolution = resolutionValue;
        if (resolutionValue != null) {
            mAuxResolution.setValue(resolutionValue);
        }
    }

    private void updateMainState() {
//        Display[] allDisplays = mDisplayManager.getDisplays();
//        String switchValue = SystemProperties.get("sys.hdmi_status.aux", "on");
//        if (allDisplays == null || allDisplays.length < 2 || switchValue.equals("off")) {
//            mHdmiResolution.setEnabled(false);
//            mHdmiScale.setEnabled(false);
//            mHdmiRotation.setEnabled(false);
//        } else {
        new Handler().postDelayed(new Runnable() {
            public void run() {
                mMainDisplayInfo = getDisplayInfo(0);
                //增加延迟，保证数据能够拿到
                if (mMainDisplayInfo != null) {
                    mMainResolution.setEntries(DrmDisplaySetting.getDisplayModes(mMainDisplayInfo).toArray(new String[0]));
                    mMainResolution.setEntryValues(DrmDisplaySetting.getDisplayModes(mMainDisplayInfo).toArray(new String[0]));
                    updateMainResolutionValue();
                    mMainResolution.setEnabled(true);
                    mMainScale.setEnabled(true);
                } else {
                    mMainResolution.setEnabled(false);
                    mMainScale.setEnabled(false);
                }

            }
        }, 1000);
    }

    private void updateAuxState() {
//        Display[] allDisplays = mDisplayManager.getDisplays();
//        String switchValue = SystemProperties.get("sys.hdmi_status.aux", "on");
//        if (allDisplays == null || allDisplays.length < 2 || switchValue.equals("off")) {
//            mHdmiResolution.setEnabled(false);
//            mHdmiScale.setEnabled(false);
//            mHdmiRotation.setEnabled(false);
//        } else {
        new Handler().postDelayed(new Runnable() {
            public void run() {
                mAuxDisplayInfo = getDisplayInfo(1);
                //增加延迟，保证数据能够拿到
                if (mAuxDisplayInfo != null) {
                    mAuxResolution.setEntries(DrmDisplaySetting.getDisplayModes(mAuxDisplayInfo).toArray(new String[0]));
                    mAuxResolution.setEntryValues(DrmDisplaySetting.getDisplayModes(mAuxDisplayInfo).toArray(new String[0]));
                    updateAuxResolutionValue();
                    mAuxResolution.setEnabled(true);
                    mAuxScale.setEnabled(true);
                    mAuxScreenVH.setEnabled(true);
                    mAuxScreenVHList.setEnabled(true);
                } else {
                    mAuxResolution.setEnabled(false);
                    mAuxScale.setEnabled(false);
                    mAuxScreenVH.setEnabled(false);
                    mAuxScreenVHList.setEnabled(false);
                }
            }
        }, 1000);
    }

    protected void showConfirmSetMainModeDialog() {
        mMainDisplayInfo = getDisplayInfo(0);
        if (mMainDisplayInfo != null) {
            DialogFragment df = ConfirmSetModeDialogFragment.newInstance(mMainDisplayInfo, new ConfirmSetModeDialogFragment.OnDialogDismissListener() {
                @Override
                public void onDismiss(boolean isok) {
                    Log.i(TAG, "showConfirmSetModeDialog->onDismiss->isok:" + isok);
                    Log.i(TAG, "showConfirmSetModeDialog->onDismiss->mOldResolution:" + mOldMainResolution);
                    updateMainResolutionValue();
                }
            });
            df.show(getFragmentManager(), "ConfirmDialog");
        }
    }

    protected void showConfirmSetAuxModeDialog() {
        mAuxDisplayInfo = getDisplayInfo(1);
        if (mAuxDisplayInfo != null) {
            DialogFragment df = ConfirmSetModeDialogFragment.newInstance(mAuxDisplayInfo, new ConfirmSetModeDialogFragment.OnDialogDismissListener() {
                @Override
                public void onDismiss(boolean isok) {
                    Log.i(TAG, "showConfirmSetModeDialog->onDismiss->isok:" + isok);
                    Log.i(TAG, "showConfirmSetModeDialog->onDismiss->mOldAuxResolution:" + mOldAuxResolution);
                    updateAuxResolutionValue();
                }
            });
            df.show(getFragmentManager(), "ConfirmDialog");
        }
    }

    @Override
    public boolean onPreferenceTreeClick(Preference preference) {
        // TODO Auto-generated method stub
        return true;
    }

    @Override
    public boolean onPreferenceClick(Preference preference) {
        if (preference == mMainScale) {
            Intent screenScaleIntent = new Intent(getActivity(), ScreenScaleActivity.class);
            mMainDisplayInfo = getDisplayInfo(0);
            if (mMainDisplayInfo != null) {
                screenScaleIntent.putExtra(ScreenScaleActivity.EXTRA_DISPLAY_INFO, mMainDisplayInfo);
                startActivity(screenScaleIntent);
            }
        } else if (preference == mMainResolution) {
            updateMainState();
        } else if (preference == mAuxScreenVHList) {
            String value = SystemProperties.get("persist.sys.rotation.einit", "0");
            mAuxScreenVHList.setValue(value);
        } else if (preference == mAuxScale) {
            Intent screenScaleIntent = new Intent(getActivity(), ScreenScaleActivity.class);
            mAuxDisplayInfo = getDisplayInfo(1);
            if (mAuxDisplayInfo != null) {
                screenScaleIntent.putExtra(ScreenScaleActivity.EXTRA_DISPLAY_INFO, mAuxDisplayInfo);
                startActivity(screenScaleIntent);
            }
        } else if (preference == mAuxResolution) {
            updateAuxState();
        }
        return true;
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object obj) {
        Log.i(TAG, "onPreferenceChange:" + obj);
        String key = preference.getKey();
        Log.d(TAG, key);
        if (preference == mMainResolution) {
            if (KEY_MAIN_RESOLUTION.equals(key)) {
                if (obj.equals(mOldMainResolution))
                    return true;
                int index = mMainResolution.findIndexOfValue((String) obj);
                Log.i(TAG, "onMainPreferenceChange: index= " + index);
                mMainDisplayInfo = getDisplayInfo(0);
                if (mMainDisplayInfo != null) {
                    DrmDisplaySetting.setDisplayModeTemp(mMainDisplayInfo, index);
                    showConfirmSetMainModeDialog();
                }
            }
        } else if (preference == mAuxResolution) {
            if (KEY_AUX_RESOLUTION.equals(key)) {
                if (obj.equals(mOldAuxResolution))
                    return true;
                int index = mAuxResolution.findIndexOfValue((String) obj);
                Log.i(TAG, "onAuxPreferenceChange: index= " + index);
                mAuxDisplayInfo = getDisplayInfo(1);
                if (mAuxDisplayInfo != null) {
                    DrmDisplaySetting.setDisplayModeTemp(mAuxDisplayInfo, index);
                    showConfirmSetAuxModeDialog();
                }
            }
        } else if (KEY_MAIN_SWITCH.equals(key)) {
            if (Boolean.parseBoolean(obj.toString())) {
                SystemProperties.set(sys_main_state, "on");
            } else {
                SystemProperties.set(sys_main_state, "off");
            }
            updateMainState();
        } else if (KEY_AUX_SWITCH.equals(key)) {
            if (Boolean.parseBoolean(obj.toString())) {
                SystemProperties.set(sys_aux_state, "on");
            } else {
                SystemProperties.set(sys_aux_state, "off");
            }
            updateAuxState();
        } else if (preference == mSystemRotation) {
            if (KEY_SYSTEM_ROTATION.equals(key)) {
                try {
                    int value = Integer.parseInt((String) obj);
                    android.os.SystemProperties.set("persist.sys.orientation", (String) obj);
                    Log.d(TAG, "freezeRotation~~~value:" + (String) obj);
                    if (value == 0) {
                        mWindowManager.freezeRotation(Surface.ROTATION_0);
                    } else if (value == 90) {
                        mWindowManager.freezeRotation(Surface.ROTATION_90);
                    } else if (value == 180) {
                        mWindowManager.freezeRotation(Surface.ROTATION_180);
                    } else if (value == 270) {
                        mWindowManager.freezeRotation(Surface.ROTATION_270);
                    } else {
                        return true;
                    }
                    //android.os.SystemProperties.set("sys.boot_completed", "1");
                } catch (Exception e) {
                    Log.e(TAG, "freezeRotation error");
                }
            }
        } else if (preference == mAuxScreenVH) {
            if ((Boolean) obj) {
                SystemProperties.set("persist.sys.rotation.efull", "true");
            } else {
                SystemProperties.set("persist.sys.rotation.efull", "false");
            }
            SystemProperties.set(sys_aux_state, "off");
            mAuxSwitch.setEnabled(false);
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    SystemProperties.set(sys_aux_state, "on");
                    mAuxSwitch.setEnabled(true);
                }
            }, 500);
        } else if (preference == mAuxScreenVHList) {
            SystemProperties.set("persist.sys.rotation.einit", obj.toString());
            //mDisplayManager.forceScheduleTraversalLocked();
            SystemProperties.set(sys_aux_state, "off");
            mAuxSwitch.setEnabled(false);
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    SystemProperties.set(sys_aux_state, "on");
                    mAuxSwitch.setEnabled(true);
                }
            }, 500);

        }
        return true;
    }

    public static boolean isAvailable() {
        return "true".equals(SystemProperties.get("ro.rk.hdmisetting"));
    }

    private void refreshState() {
        if (mShowSettings != ONLY_SHOW_AUX) {
            updateMainState();
        }
        if (mShowSettings != ONLY_SHOW_MAIN) {
            updateAuxState();
        }
    }

    class DisplayListener implements DisplayManager.DisplayListener {
        @Override
        public void onDisplayAdded(int displayId) {
            refreshState();
        }

        @Override
        public void onDisplayChanged(int displayId) {
            refreshState();
        }

        @Override
        public void onDisplayRemoved(int displayId) {
            refreshState();
        }
    }
}
