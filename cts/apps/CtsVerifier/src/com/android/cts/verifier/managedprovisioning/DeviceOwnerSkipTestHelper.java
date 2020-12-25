/*
 * Copyright (C) 2018 The Android Open Source Project
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
package com.android.cts.verifier.managedprovisioning;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.admin.DevicePolicyManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import com.android.cts.verifier.R;

public class DeviceOwnerSkipTestHelper {
    private static final String TAG = "DeviceOwnerSkipTestHelper";
    private static final int REQUEST_CODE_SKIP_TEST = 1997;
    private static final String PREF_DEVICE_OWNER = "device-owner";
    private static final String KEY_SKIPPED = "SKIPPED";
    private static final ComponentName SKIP_TEST_CHECK_ACTIVITY = new ComponentName(
            "com.android.cts.emptydeviceowner",
            "com.android.cts.emptydeviceowner.SkipTestCheckActivity");

    private Activity mTestActivity;
    private SharedPreferences mPrefs;
    private Runnable mSetupTestRunnable;

    public DeviceOwnerSkipTestHelper(Activity testActivity, Runnable setupTestRunnable) {
        mTestActivity = testActivity;
        mSetupTestRunnable = setupTestRunnable;
        mPrefs = testActivity.getSharedPreferences(PREF_DEVICE_OWNER, Context.MODE_PRIVATE);
    }

    public void setupSkipCheckUi() {
        mTestActivity.findViewById(R.id.check_preconditions).setOnClickListener(
                v -> new AlertDialog.Builder(mTestActivity)
                        .setIcon(android.R.drawable.ic_dialog_info)
                        .setTitle(R.string.device_owner_precondition_dialog_title)
                        .setMessage(R.string.device_owner_precondition_dialog_text)
                        .setPositiveButton(android.R.string.ok,
                                (dialog, which) -> startSkipCheckActivity())
                        .show());
        mTestActivity.findViewById(R.id.set_device_owner_button).setVisibility(View.GONE);

        if (mPrefs.contains(KEY_SKIPPED)) {
            boolean shouldSkip = mPrefs.getBoolean(KEY_SKIPPED, false);
            updateUi(shouldSkip);
        }
    }

    private void startSkipCheckActivity() {
        DevicePolicyManager dpm = mTestActivity.getSystemService(DevicePolicyManager.class);
        if (!dpm.isDeviceOwnerApp(SKIP_TEST_CHECK_ACTIVITY.getPackageName())) {
            Toast.makeText(mTestActivity,
                    R.string.empty_device_owner_not_configure_properly,
                    Toast.LENGTH_LONG).show();
            return;
        }

        Intent intent = new Intent();
        intent.setComponent(SKIP_TEST_CHECK_ACTIVITY);
        mTestActivity.startActivityForResult(intent, REQUEST_CODE_SKIP_TEST);
    }

    public boolean handleSkipCheckActivityResult(int requestCode, int resultCode, Intent data) {
        if (REQUEST_CODE_SKIP_TEST != requestCode) {
            return false;
        }
        boolean shouldSkip = data.getBooleanExtra(Intent.EXTRA_RETURN_RESULT, false);
        Log.d(TAG, "handleSkipCheckActivityResult: shouldSkip = " + shouldSkip);
        mPrefs.edit().putBoolean(KEY_SKIPPED, shouldSkip).apply();
        updateUi(shouldSkip);
        return true;
    }

    private void updateUi(boolean shouldSkip) {
        if (shouldSkip) {
            Toast.makeText(mTestActivity,
                    R.string.device_owner_skipped_preloaded_accounts,
                    Toast.LENGTH_LONG).show();
        } else {
            mTestActivity.findViewById(R.id.check_preconditions).setVisibility(View.GONE);
            mTestActivity.findViewById(R.id.set_device_owner_button).setVisibility(View.VISIBLE);
            mSetupTestRunnable.run();
        }
    }
}
