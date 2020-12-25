/**
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

package android.security.cts;

import android.platform.test.annotations.SecurityTest;

public class Poc17_03 extends SecurityTestCase {

    /**
     *  b/31824853
     */
    @SecurityTest
    public void testPocCVE_2016_8479() throws Exception {
        if (containsDriver(getDevice(), "/dev/kgsl-3d0")) {
            AdbUtils.runPocNoOutput("CVE-2016-8479", getDevice(), 180);
            // CTS begins the next test before device finishes rebooting,
            // sleep to allow time for device to reboot.
            Thread.sleep(30000);
        }
    }

    /**
     *  b/33940449
     */
    @SecurityTest
    public void testPocCVE_2017_0508() throws Exception {
        if (containsDriver(getDevice(), "/dev/ion") &&
            containsDriver(getDevice(), "/dev/dri/renderD129")) {
            AdbUtils.runPocNoOutput("CVE-2017-0508", getDevice(), 30);
            // CTS begins the next test before device finishes rebooting,
            // sleep to allow time for device to reboot.
            Thread.sleep(60000);
        }
    }

    /**
     *  b/33899363
     */
    @SecurityTest
    public void testPocCVE_2017_0333() throws Exception {
        if (containsDriver(getDevice(), "/dev/dri/renderD128")) {
            AdbUtils.runPocNoOutput("CVE-2017-0333", getDevice(), 30);
            // Device takes up to 30 seconds to crash after ioctl call
            Thread.sleep(30000);
        }
    }
}
