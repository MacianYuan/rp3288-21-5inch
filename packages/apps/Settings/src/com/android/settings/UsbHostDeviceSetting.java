package com.android.settings;

import android.app.Dialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.support.v14.preference.SwitchPreference;
import android.support.v7.preference.ListPreference;
import android.support.v7.preference.Preference;
import android.support.v7.preference.PreferenceScreen;
import android.support.v7.preference.Preference.OnPreferenceChangeListener;
import android.provider.Settings;
import android.content.res.Resources;

import android.util.Log;

import com.android.internal.logging.nano.MetricsProto.MetricsEvent;

import java.io.File;
import java.io.FileOutputStream;

public class UsbHostDeviceSetting extends SettingsPreferenceFragment implements OnPreferenceChangeListener {
    /**
     * Called when the activity is first created.
     */
    private static final String KEY_USB_MODE_LOCATION = "usb_mode";
    private static final String UsbDeviceHostCtlPatch = "/sys/bus/platform/drivers/usb20_otg/force_usb_mode";
   
    private static final String TAG = "UsbHostDeviceDebug";

    public boolean dbg = true;

    private int UsbHostState = 1;
    private int UsbDeviceState = 2;

    private ListPreference mUsbMode;

    private SharedPreferences mSharedPreference;
    private SharedPreferences.Editor mEdit;
    private SettingsApplication mScreenshot;

    private Context mContext;
    private Dialog dialog;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.usb_host_device);

        mContext = getActivity();

        mUsbMode = (ListPreference) findPreference(KEY_USB_MODE_LOCATION);
        mUsbMode.setOnPreferenceChangeListener(this);
    }


    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        // TODO Auto-generated method stub
        if (preference == mUsbMode) {
            String value = (String) newValue;
            //mEdit.putString("storageLocation",value);
            if (value.equals("device")) {
		 if(dbg) Log.d(TAG, "usb mode is device");
                 WriteGpioState(UsbDeviceHostCtlPatch, (UsbDeviceState+"").getBytes());
            } else if (value.equals("host")) {
		 if(dbg) Log.d(TAG, "usb mode is host");
		 WriteGpioState(UsbDeviceHostCtlPatch, (UsbHostState+"").getBytes());
            }

        }
        return true;
    }


    private String WriteGpioState(String path, byte[] buffer) {
        try {
            File file = new File(path);
            FileOutputStream fos = new FileOutputStream(file);
            fos.write(buffer);
            fos.close();
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            if(dbg) Log.e(TAG, "writeProc() write error!");
            return "write error!";
        }
        return (buffer.toString());
    }

    @Override
    public int getMetricsCategory() {
        // TODO Auto-generated method stub
        return 5;
    }
}
