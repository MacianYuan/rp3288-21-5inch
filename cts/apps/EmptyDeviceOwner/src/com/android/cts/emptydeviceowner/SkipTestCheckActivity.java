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
package com.android.cts.emptydeviceowner;

import static android.app.admin.DevicePolicyManager.ACCOUNT_FEATURE_DEVICE_OR_PROFILE_OWNER_ALLOWED;
import static android.app.admin.DevicePolicyManager
        .ACCOUNT_FEATURE_DEVICE_OR_PROFILE_OWNER_DISALLOWED;
import static android.content.pm.ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;

import android.accounts.Account;
import android.accounts.AccountManager;
import android.app.Activity;
import android.app.admin.DevicePolicyManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.PersistableBundle;
import android.util.Log;

import java.util.Arrays;

public class SkipTestCheckActivity extends Activity {
    private static final String TAG = "SkipTestCheckActivity";

    /**
     * Return whether device owner tests should be skipped.
     * <p>
     * Some OEM devices have pre-loaded accounts, and thus failed the set-device-owner command.
     * In O, we introduced
     * {@link DevicePolicyManager#ACCOUNT_FEATURE_DEVICE_OR_PROFILE_OWNER_ALLOWED} to make testing
     * in these devices possible by allowing the command to work if all preloaded accounts have
     * the feature flag. But this requires the DO app to be "test-only", and we don't want
     * to make CtsVerifier test-only. Thus, since P+ we introduce a dummy test-only device owner
     * app, we set it as device owner and then use
     * {@link DevicePolicyManager#transferOwnership(ComponentName, ComponentName, PersistableBundle)}
     * to transfer the ownership to the CtsVerifier app.
     * <p>
     * The workaround requires P+ to work, so we need to skip the test if it is running in pre
     * P devices and all accounts has the feature flag
     * {@link DevicePolicyManager#ACCOUNT_FEATURE_DEVICE_OR_PROFILE_OWNER_ALLOWED and none of them
     * has the {@link DevicePolicyManager#ACCOUNT_FEATURE_DEVICE_OR_PROFILE_OWNER_DISALLOWED}.
     *
     * @param context
     * @return {@code true} if device owners test should be skipped, {@code false} otherwise.
     */
    public static boolean shouldSkipDeviceOwnerTestForPreloadedAccounts(Context context) {
        AccountManager am = context.getSystemService(AccountManager.class);
        final Account accounts[] = am.getAccounts();
        if (accounts.length == 0) {
            return false;
        }
        for (Account account : accounts) {
            Log.d(TAG, "shouldSkipDeviceOwnerTestForPreloadedAccounts: found " + account);
        }
        if (!Arrays.stream(accounts).allMatch(
                account -> hasAccountFeature(
                        am, account, ACCOUNT_FEATURE_DEVICE_OR_PROFILE_OWNER_ALLOWED))) {
            Log.w(TAG, "Some accounts does not have the DEVICE_OR_PROFILE_OWNER_ALLOWED flag");
            return false;
        }
        if (Arrays.stream(accounts).anyMatch(
                account -> hasAccountFeature(
                        am, account, ACCOUNT_FEATURE_DEVICE_OR_PROFILE_OWNER_DISALLOWED))) {
            Log.w(TAG, "Some accounts have the DEVICE_OR_PROFILE_OWNER_DISALLOWED flag");
            return false;
        }
        return true;
    }

    private static boolean hasAccountFeature(AccountManager am, Account account, String feature) {
        try {
            return am.hasFeatures(account, new String[]{feature}, null, null).getResult();
        } catch (Exception e) {
            Log.e(TAG, "hasAccountFeatures: ", e);
        }
        return false;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRequestedOrientation(SCREEN_ORIENTATION_PORTRAIT);
        new Thread(() -> {
            DevicePolicyManager dpm = getSystemService(DevicePolicyManager.class);
            boolean shouldSkip = false;
            boolean isDeviceOwner = dpm.isDeviceOwnerApp(getPackageName());
            if (isDeviceOwner) {
                shouldSkip = shouldSkipDeviceOwnerTestForPreloadedAccounts(this);;
            }

            Intent resultIntent = new Intent();
            resultIntent.putExtra(Intent.EXTRA_RETURN_RESULT, shouldSkip);
            setResult(Activity.RESULT_OK, resultIntent);

            if (isDeviceOwner) {
                getSystemService(DevicePolicyManager.class).clearDeviceOwnerApp(getPackageName());
            }
            finish();
        }).start();
    }
}
