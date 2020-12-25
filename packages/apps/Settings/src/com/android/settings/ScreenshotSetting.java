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

import com.android.internal.logging.nano.MetricsProto.MetricsEvent;

public class ScreenshotSetting extends SettingsPreferenceFragment implements OnPreferenceChangeListener {
    /**
     * Called when the activity is first created.
     */
    private static final String KEY_SCREENSHOT_DELAY = "screenshot_delay";
    private static final String KEY_SCREENSHOT_STORAGE_LOCATION = "screenshot_storage";
    private static final String KEY_SCREENSHOT_SHOW = "screenshot_show";
    private static final String KEY_SCREENSHOT_VERSION = "screenshot_version";

    private ListPreference mDelay;
    private ListPreference mStorage;
    private SwitchPreference mShow;

    private SharedPreferences mSharedPreference;
    private SharedPreferences.Editor mEdit;
    private SettingsApplication mScreenshot;

    private Context mContext;
    private Dialog dialog;
    private static final String INTERNAL_STORAGE = "internal_storage";
    private static final String EXTERNAL_SD_STORAGE = "external_sd_storage";
    private static final String EXTERNAL_USB_STORAGE = "internal_usb_storage";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.screenshot);

        mContext = getActivity();
        mDelay = (ListPreference) findPreference(KEY_SCREENSHOT_DELAY);
        mStorage = (ListPreference) findPreference(KEY_SCREENSHOT_STORAGE_LOCATION);
        mShow = (SwitchPreference) findPreference(KEY_SCREENSHOT_SHOW);

        mShow.setOnPreferenceChangeListener(this);
        mDelay.setOnPreferenceChangeListener(this);
        mStorage.setOnPreferenceChangeListener(this);

        mSharedPreference = this.getPreferenceScreen().getSharedPreferences();
        mEdit = mSharedPreference.edit();

        String summary_delay = mDelay.getSharedPreferences().getString("screenshot_delay", "15");
        mDelay.setSummary(summary_delay + getString(R.string.later));
        mDelay.setValue(summary_delay);
        String summary_storage = Settings.System.getString(getContentResolver(), "screenshot_location");
        if (EXTERNAL_SD_STORAGE.equals(summary_storage)) {
            summary_storage = "sdcard";
        } else if (EXTERNAL_USB_STORAGE.equals(summary_storage)) {
            summary_storage = "usb";
        } else {
            summary_storage = "flash";
        }
        mStorage.setValue(summary_storage);
        mStorage.setSummary(mStorage.getEntry());
        boolean isShow = Settings.System.getInt(mContext.getContentResolver(), "screenshot_button_show", 1) == 1;
        mShow.setChecked(isShow);
        Resources res = mContext.getResources();
        boolean mHasNavigationBar = res.getBoolean(com.android.internal.R.bool.config_showNavigationBar);
        if (!mHasNavigationBar) {
            getPreferenceScreen().removePreference(mShow);
        }

        mScreenshot = (SettingsApplication) getActivity().getApplication();
    }


    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        // TODO Auto-generated method stub
        if (preference == mDelay) {
            int value = Integer.parseInt((String) newValue);
            mDelay.setSummary((String) newValue + getString(R.string.later));
            mScreenshot.startScreenshot(value);
        } else if (preference == mStorage) {
            String value = (String) newValue;
            //mEdit.putString("storageLocation",value);
            if (value.equals("flash")) {
                Settings.System.putString(getContentResolver(), "screenshot_location", INTERNAL_STORAGE);
                mStorage.setSummary(mStorage.getEntries()[0]);
            } else if (value.equals("sdcard")) {
                Settings.System.putString(getContentResolver(), "screenshot_location", EXTERNAL_SD_STORAGE);
                mStorage.setSummary(mStorage.getEntries()[1]);
            } else if (value.equals("usb")) {
                Settings.System.putString(getContentResolver(), "screenshot_location", EXTERNAL_USB_STORAGE);
                mStorage.setSummary(mStorage.getEntries()[2]);
            }

        }
        return true;
    }

    @Override
    public boolean onPreferenceTreeClick(Preference preference) {
        // TODO Auto-generated method stub
        if (preference == mShow) {
            boolean show = mShow.isChecked();
            Settings.System.putInt(getContentResolver(), "screenshot_button_show", show ? 1 : 0);
        }
        return super.onPreferenceTreeClick(preference);
    }

    @Override
    public int getMetricsCategory() {
        // TODO Auto-generated method stub
        return 5;
    }

}
