/*
 * Copyright (C) 2017 The Android Open Source Project
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
package android.autofillservice.cts;

import static android.autofillservice.cts.Helper.FILL_TIMEOUT_MS;

import static com.google.common.truth.Truth.assertWithMessage;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.util.SparseArray;
import android.view.autofill.AutofillManager;
import android.view.autofill.AutofillValue;
import android.webkit.WebView;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

/**
 * Custom {@link WebView} used to assert contents were autofilled.
 */
public class MyWebView extends WebView {

    private static final String TAG = "MyWebView";
    private FillExpectation mExpectation;

    public MyWebView(Context context, AttributeSet attrs) {
        super(context, attrs);
        Log.d(TAG, "isAutofillEnabled() on constructor? " + isAutofillEnabled());
    }

    public void expectAutofill(String username, String password) {
        mExpectation = new FillExpectation(username, password);
    }

    public void assertAutofilled() throws Exception {
        assertWithMessage("expectAutofill() not called").that(mExpectation).isNotNull();
        final boolean set = mExpectation.mLatch.await(FILL_TIMEOUT_MS, TimeUnit.MILLISECONDS);
        if (mExpectation.mException != null) {
            throw mExpectation.mException;
        }
        assertWithMessage("Timeout (%s ms) expecting autofill()", FILL_TIMEOUT_MS)
                .that(set).isTrue();
        assertWithMessage("Wrong value for username").that(mExpectation.mActualUsername)
                .isEqualTo(mExpectation.mExpectedUsername);
        assertWithMessage("Wrong value for password").that(mExpectation.mActualPassword)
                .isEqualTo(mExpectation.mExpectedPassword);
    }

    @Override
    public void autofill(SparseArray<AutofillValue> values) {
        super.autofill(values);

        if (mExpectation == null) return;

        try {
            if (values == null || values.size() != 2) {
                mExpectation.mException =
                        new IllegalArgumentException("Invalid values on autofill(): " + values);
            } else {
                try {
                    mExpectation.mActualUsername = values.valueAt(0).getTextValue().toString();
                    mExpectation.mActualPassword = values.valueAt(1).getTextValue().toString();
                } catch (Exception e) {
                    mExpectation.mException = e;
                }
            }
        } finally {
            mExpectation.mLatch.countDown();
        }
    }

    boolean isAutofillEnabled() {
        return getContext().getSystemService(AutofillManager.class).isEnabled();
    }

    private class FillExpectation {
        private final CountDownLatch mLatch = new CountDownLatch(1);
        private final String mExpectedUsername;
        private final String mExpectedPassword;
        private String mActualUsername;
        private String mActualPassword;
        private Exception mException;

        FillExpectation(String username, String password) {
            this.mExpectedUsername = username;
            this.mExpectedPassword = password;
        }
    }
}
