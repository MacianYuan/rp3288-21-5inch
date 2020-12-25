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

package com.android.compatibility.common.tradefed.util;

import com.android.compatibility.common.tradefed.build.CompatibilityBuildHelper;
import com.android.compatibility.common.tradefed.util.RetryFilterHelper;
import com.android.compatibility.common.tradefed.util.RetryType;
import com.android.tradefed.device.DeviceNotAvailableException;
import com.android.tradefed.device.ITestDevice;
import java.io.File;
import java.net.URL;
import java.util.HashSet;
import org.easymock.EasyMock;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

/**
 * Unit tests for {@link VtsRetryFilterHelper}.
 */
@RunWith(JUnit4.class)
public class VtsRetryFilterHelperTest {
    private final String RESULTS_DIR = "/util/results";
    private final String VENDOR_FINGERPRINT =
            "Android/aosp_sailfish/sailfish:8.0.0/OC/4311111:userdebug/test-keys";
    private final String WRONG_FINGERPRINT =
            "Android/other_device/other_device:8.0.0/OC/4311112:userdebug/test-keys";
    private final String VENDOR_FINGERPRINT_PROPERTY = "ro.vendor.build.fingerprint";
    private CompatibilityBuildHelper mBuildHelper;
    private RetryFilterHelper mHelper;

    @Before
    public void setUp() throws Exception {
        URL resultsDir = getClass().getResource(RESULTS_DIR);
        mBuildHelper = new CompatibilityBuildHelper(null) {
            @Override
            public File getResultsDir() {
                return new File(resultsDir.getPath());
            }
        };
        mHelper = new VtsRetryFilterHelper(mBuildHelper, 0, "SUB_PLAN", new HashSet<String>(),
                new HashSet<String>(), "ABI_NAME", "MODULE_NAME", "TEST_NAME", RetryType.FAILED);
    }

    /**
     * Create a mock {@link ITestDevice} with fingerprint properties.
     * @param vendorFingerprint The vendor fingerprint of the device.
     * @return The mock device.
     * @throws DeviceNotAvailableException
     */
    private ITestDevice createMockDevice(String vendorFingerprint)
            throws DeviceNotAvailableException {
        ITestDevice mockDevice = EasyMock.createMock(ITestDevice.class);
        EasyMock.expect(mockDevice.getProperty(VENDOR_FINGERPRINT_PROPERTY))
                .andReturn(vendorFingerprint);
        EasyMock.replay(mockDevice);
        return mockDevice;
    }

    /**
     * Test ValidateBuildFingerprint without error.
     * @throws DeviceNotAvailableException
     */
    @Test
    public void testValidateBuildFingerprint() throws DeviceNotAvailableException {
        mHelper.validateBuildFingerprint(createMockDevice(VENDOR_FINGERPRINT));
    }

    /**
     * Test ValidateBuildFingerprint with the incorrect fingerprint.
     * @throws DeviceNotAvailableException
     */
    @Test(expected = IllegalArgumentException.class)
    public void testMismatchSystemFingerprint() throws DeviceNotAvailableException {
        mHelper.validateBuildFingerprint(createMockDevice(WRONG_FINGERPRINT));
    }
}
