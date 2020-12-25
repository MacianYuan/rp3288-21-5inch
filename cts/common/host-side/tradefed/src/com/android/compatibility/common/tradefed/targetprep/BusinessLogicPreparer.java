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
package com.android.compatibility.common.tradefed.targetprep;

import com.android.compatibility.common.tradefed.build.CompatibilityBuildHelper;
import com.android.compatibility.common.tradefed.util.DynamicConfigFileReader;
import com.android.compatibility.common.util.BusinessLogic;
import com.android.compatibility.common.util.FeatureUtil;
import com.android.compatibility.common.util.PropertyUtil;
import com.android.tradefed.build.IBuildInfo;
import com.android.tradefed.config.Option;
import com.android.tradefed.config.OptionClass;
import com.android.tradefed.device.DeviceNotAvailableException;
import com.android.tradefed.device.ITestDevice;
import com.android.tradefed.log.LogUtil.CLog;
import com.android.tradefed.targetprep.BuildError;
import com.android.tradefed.targetprep.ITargetCleaner;
import com.android.tradefed.targetprep.TargetSetupError;
import com.android.tradefed.testtype.suite.TestSuiteInfo;
import com.android.tradefed.util.FileUtil;
import com.android.tradefed.util.MultiMap;
import com.android.tradefed.util.StreamUtil;
import com.android.tradefed.util.net.HttpHelper;
import com.android.tradefed.util.net.IHttpHelper;

import org.xmlpull.v1.XmlPullParserException;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * Pushes business Logic to the host and the test device, for use by test cases in the test suite.
 */
@OptionClass(alias="business-logic-preparer")
public class BusinessLogicPreparer implements ITargetCleaner {

    /* Placeholder in the service URL for the suite to be configured */
    private static final String SUITE_PLACEHOLDER = "{suite-name}";

    /* String for creating files to store the business logic configuration on the host */
    private static final String FILE_LOCATION = "business-logic";
    /* Extension of business logic files */
    private static final String FILE_EXT = ".bl";

    /* Dynamic config constants */
    private static final String DYNAMIC_CONFIG_FEATURES_KEY = "business_logic_device_features";
    private static final String DYNAMIC_CONFIG_PROPERTIES_KEY = "business_logic_device_properties";

    @Option(name = "business-logic-url", description = "The URL to use when accessing the " +
            "business logic service, parameters not included", mandatory = true)
    private String mUrl;

    @Option(name = "business-logic-api-key", description = "The API key to use when accessing " +
            "the business logic service.", mandatory = true)
    private String mApiKey;

    @Option(name = "cleanup", description = "Whether to remove config files from the test " +
            "target after test completion.")
    private boolean mCleanup = true;

    @Option(name = "ignore-business-logic-failure", description = "Whether to proceed with the " +
            "suite invocation if retrieval of business logic fails.")
    private boolean mIgnoreFailure = false;

    private String mDeviceFilePushed;
    private String mHostFilePushed;

    /**
     * {@inheritDoc}
     */
    @Override
    public void setUp(ITestDevice device, IBuildInfo buildInfo) throws TargetSetupError, BuildError,
            DeviceNotAvailableException {
        String requestString = buildRequestString(device, buildInfo);
        // Retrieve business logic string from service
        String businessLogicString = null;
        try {
            URL request = new URL(requestString);
            businessLogicString = StreamUtil.getStringFromStream(request.openStream());
        } catch (IOException e) {
            if (mIgnoreFailure) {
                CLog.e("Failed to connect to business logic service.\nProceeding with test"
                        + "invocation, tests depending on the remote configuration will fail.\n");
                return;
            } else {
                throw new TargetSetupError(String.format(
                    "Cannot connect to business logic service for suite %s.\nIf this problem "
                    + "persists, re-invoking with option '--ignore-business-logic-failure' will "
                    + "cause tests to execute anyways (though tests depending on the remote "
                    + "configuration will fail).", TestSuiteInfo.getInstance().getName()), e,
                    device.getDeviceDescriptor());
            }
        }
        // Push business logic string to host file
        try {
            File hostFile = FileUtil.createTempFile(FILE_LOCATION, FILE_EXT);
            FileUtil.writeToFile(businessLogicString, hostFile);
            mHostFilePushed = hostFile.getAbsolutePath();
            CompatibilityBuildHelper buildHelper = new CompatibilityBuildHelper(buildInfo);
            buildHelper.setBusinessLogicHostFile(hostFile);
        } catch (IOException e) {
            throw new TargetSetupError(String.format(
                    "Retrieved business logic for suite %s could not be written to host",
                    TestSuiteInfo.getInstance().getName()), device.getDeviceDescriptor());
        }
        // Push business logic string to device file
        removeDeviceFile(device); // remove any existing business logic file from device
        if (device.pushString(businessLogicString, BusinessLogic.DEVICE_FILE)) {
            mDeviceFilePushed = BusinessLogic.DEVICE_FILE;
        } else {
            throw new TargetSetupError(String.format(
                    "Retrieved business logic for suite %s could not be written to device %s",
                    TestSuiteInfo.getInstance().getName(), device.getSerialNumber()),
                    device.getDeviceDescriptor());
        }
    }

    /** Helper to populate the business logic service request with info about the device. */
    private String buildRequestString(ITestDevice device, IBuildInfo buildInfo)
            throws DeviceNotAvailableException {
        CompatibilityBuildHelper buildHelper = new CompatibilityBuildHelper(buildInfo);
        String baseUrl = mUrl.replace(SUITE_PLACEHOLDER, getSuiteName());
        MultiMap<String, String> paramMap = new MultiMap<>();
        paramMap.put("key", mApiKey);
        paramMap.put("suite_version", buildHelper.getSuiteVersion());
        paramMap.put("oem", PropertyUtil.getManufacturer(device));
        for (String feature : getBusinessLogicFeatures(device, buildInfo)) {
            paramMap.put("features", feature);
        }
        for (String property : getBusinessLogicProperties(device, buildInfo)) {
            paramMap.put("properties", property);
        }
        IHttpHelper helper = new HttpHelper();
        return helper.buildUrl(baseUrl, paramMap);
    }

    /* Get device properties list, with element format "<property_name>:<property_value>" */
    private List<String> getBusinessLogicProperties(ITestDevice device, IBuildInfo buildInfo)
            throws DeviceNotAvailableException {
        List<String> properties = new ArrayList<>();
        Map<String, String> clientIds = PropertyUtil.getClientIds(device);
        for (Map.Entry<String, String> id : clientIds.entrySet()) {
            // add client IDs to the list of properties
            properties.add(String.format("%s:%s", id.getKey(), id.getValue()));
        }

        try {
            List<String> propertyNames = DynamicConfigFileReader.getValuesFromConfig(buildInfo,
                    getSuiteName(), DYNAMIC_CONFIG_PROPERTIES_KEY);
            for (String name : propertyNames) {
                // Use String.valueOf in case property is undefined for the device ("null")
                String value = String.valueOf(device.getProperty(name));
                properties.add(String.format("%s:%s", name, value));
            }
        } catch (XmlPullParserException | IOException e) {
            CLog.e("Failed to pull business logic properties from dynamic config");
        }
        return properties;
    }

    /* Get device features list */
    private List<String> getBusinessLogicFeatures(ITestDevice device, IBuildInfo buildInfo)
            throws DeviceNotAvailableException {
        try {
            List<String> dynamicConfigFeatures = DynamicConfigFileReader.getValuesFromConfig(
                    buildInfo, getSuiteName(), DYNAMIC_CONFIG_FEATURES_KEY);
            Set<String> deviceFeatures = FeatureUtil.getAllFeatures(device);
            dynamicConfigFeatures.retainAll(deviceFeatures);
            return dynamicConfigFeatures;
        } catch (XmlPullParserException | IOException e) {
            CLog.e("Failed to pull business logic features from dynamic config");
            return new ArrayList<>();
        }
    }

    private String getSuiteName() {
        return TestSuiteInfo.getInstance().getName().toLowerCase();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void tearDown(ITestDevice device, IBuildInfo buildInfo, Throwable e)
            throws DeviceNotAvailableException {
        // Clean up host file
        if (mCleanup) {
            if (mHostFilePushed != null) {
                FileUtil.deleteFile(new File(mHostFilePushed));
            }
            if (mDeviceFilePushed != null && !(e instanceof DeviceNotAvailableException)) {
                removeDeviceFile(device);
            }
        }
    }

    /** Remove business logic file from the device */
    private static void removeDeviceFile(ITestDevice device) throws DeviceNotAvailableException {
        device.executeShellCommand(String.format("rm -rf %s", BusinessLogic.DEVICE_FILE));
    }
}
