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

package com.android.compatibility.common.tradefed.testtype;

import com.android.compatibility.common.tradefed.util.RetryFilterHelper;
import com.android.compatibility.common.tradefed.util.VtsRetryFilterHelper;
import com.android.tradefed.config.ConfigurationException;
import com.android.tradefed.config.OptionClass;
import com.android.tradefed.config.OptionCopier;
import com.android.tradefed.config.OptionSetter;
import com.android.tradefed.log.LogUtil.CLog;
import com.android.tradefed.testtype.IRemoteTest;

/**
 * A Test for running Compatibility Suites
 */
@OptionClass(alias = "compatibility")
public class CompatibilityTestMultiDevice extends CompatibilityTest {
    /**
     * Create a new {@link CompatibilityTest} that will run a sublist of
     * modules.
     */
    public CompatibilityTestMultiDevice(
            int totalShards, IModuleRepo moduleRepo, Integer shardIndex) {
        super(totalShards, moduleRepo, shardIndex);
    }

    /**
     * Create a new {@link CompatibilityTestMultiDevice} that will run the default list of
     * modules.
     */
    public CompatibilityTestMultiDevice() {}

    /**
     * {@inheritDoc}
     */
    @Override
    public IRemoteTest getTestShard(int shardCount, int shardIndex) {
        CompatibilityTestMultiDevice test =
                new CompatibilityTestMultiDevice(shardCount, getModuleRepo(), shardIndex);
        OptionCopier.copyOptionsNoThrow(this, test);
        // Set the shard count because the copy option on the previous line
        // copies over the mShard value
        try {
            OptionSetter setter = new OptionSetter(test);
            setter.setOptionValue("shards", "0");
        } catch (ConfigurationException e) {
            CLog.e(e);
        }
        return test;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected RetryFilterHelper createRetryFilterHelper(Integer retrySessionId) {
        return new VtsRetryFilterHelper(getBuildHelper(), retrySessionId, getSubPlan(),
                getIncludeFilters(), getExcludeFilters(), getAbiName(), getModuleName(),
                getTestName(), getRetryType());
    }
}