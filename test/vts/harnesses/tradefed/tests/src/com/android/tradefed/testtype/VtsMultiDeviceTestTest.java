/*
 * Copyright (C) 2016 The Android Open Source Project
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
package com.android.tradefed.testtype;

import com.android.tradefed.build.IBuildInfo;
import com.android.tradefed.build.IFolderBuildInfo;
import com.android.tradefed.device.DeviceNotAvailableException;
import com.android.tradefed.device.ITestDevice;
import com.android.tradefed.result.ITestInvocationListener;
import com.android.tradefed.util.CommandResult;
import com.android.tradefed.util.CommandStatus;
import com.android.tradefed.util.IRunUtil;
import com.android.tradefed.util.ProcessHelper;
import com.android.tradefed.util.RunInterruptedException;
import com.android.tradefed.util.StreamUtil;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Map;
import junit.framework.TestCase;
import org.easymock.EasyMock;
import org.json.JSONObject;

/**
 * Unit tests for {@link VtsMultiDeviceTest}.
 * This class requires testcase config files.
 * The working directory is assumed to be
 * test/
 * which contains the same config as the build output
 * out/host/linux-x86/vts/android-vts/testcases/
 */
public class VtsMultiDeviceTestTest extends TestCase {

    private ITestInvocationListener mMockInvocationListener = null;
    private VtsMultiDeviceTest mTest = null;
    private ProcessHelper mProcessHelper = null;
    private String mPython = null;
    private static final String PYTHON_DIR = "mock/";
    private static final String TEST_CASE_PATH =
        "vts/testcases/host/sample/SampleLightTest";

    /**
     * Helper to initialize the various EasyMocks we'll need.
     */
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mMockInvocationListener = EasyMock.createMock(ITestInvocationListener.class);
        mProcessHelper = null;
        mPython = "python";
        mTest = new VtsMultiDeviceTest() {
            @Override
            protected ProcessHelper createProcessHelper(String[] cmd) {
                assertCommand(cmd);
                try {
                    createResult(cmd[3]);
                } catch (Exception e) {
                    throw new RuntimeException(e);
                }
                return mProcessHelper;
            }
        };
        mTest.setBuild(createMockBuildInfo());
        mTest.setTestCasePath(TEST_CASE_PATH);
        mTest.setTestConfigPath(VtsMultiDeviceTest.DEFAULT_TESTCASE_CONFIG_PATH);
    }

    /**
     * Check VTS Python command strings.
     */
    private void assertCommand(String[] cmd) {
        assertEquals(cmd[0], PYTHON_DIR + mPython);
        assertEquals(cmd[1], "-m");
        assertEquals(cmd[2], TEST_CASE_PATH.replace("/", "."));
        assertTrue(cmd[3].endsWith(".json"));
        assertEquals(cmd.length, 4);
    }

    /**
     * Create files in log directory.
     */
    private void createResult(String jsonPath) throws Exception {
        String logPath = null;
        try (FileInputStream fi = new FileInputStream(jsonPath)) {
            JSONObject configJson = new JSONObject(StreamUtil.getStringFromStream(fi));
            logPath = (String) configJson.get(VtsMultiDeviceTest.LOG_PATH);
        }
        // create a test result on log path
        try (FileWriter fw = new FileWriter(new File(logPath, "test_run_summary.json"))) {
            JSONObject outJson = new JSONObject();
            fw.write(outJson.toString());
        }
        new File(logPath, VtsMultiDeviceTest.REPORT_MESSAGE_FILE_NAME).createNewFile();
    }

    /**
     * Create a process helper which mocks status of a running process.
     */
    private static ProcessHelper createMockProcessHelper(CommandStatus... status) {
        Process process;
        try {
            process = new ProcessBuilder("true").start();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
        LinkedList<CommandStatus> nextStatus = new LinkedList<CommandStatus>(Arrays.asList(status));
        return new ProcessHelper(process) {
            @Override
            public CommandStatus waitForProcess(long timeoutMsecs) throws RunInterruptedException {
                CommandStatus currentStatus = nextStatus.removeFirst();
                if (currentStatus == null) {
                    throw new RunInterruptedException();
                }
                return currentStatus;
            }

            @Override
            public boolean isRunning() {
                return !nextStatus.isEmpty();
            }
        };
    }

    /**
     * Create a mock IBuildInfo with necessary getter methods.
     */
    private static IBuildInfo createMockBuildInfo() {
        Map<String, String> buildAttributes = new HashMap<String, String>();
        buildAttributes.put("ROOT_DIR", "DIR_NOT_EXIST");
        buildAttributes.put("ROOT_DIR2", "DIR_NOT_EXIST");
        buildAttributes.put("SUITE_NAME", "JUNIT_TEST_SUITE");
        IFolderBuildInfo buildInfo = EasyMock.createMock(IFolderBuildInfo.class);
        EasyMock.expect(buildInfo.getBuildId()).
                andReturn("BUILD_ID");
        EasyMock.expect(buildInfo.getBuildTargetName()).
                andReturn("BUILD_TARGET_NAME");
        EasyMock.expect(buildInfo.getTestTag()).
                andReturn("TEST_TAG").atLeastOnce();
        EasyMock.expect(buildInfo.getDeviceSerial()).
                andReturn("1234567890ABCXYZ").atLeastOnce();
        EasyMock.expect(buildInfo.getRootDir()).
                andReturn(new File("DIR_NOT_EXIST"));
        EasyMock.expect(buildInfo.getBuildAttributes()).
                andReturn(buildAttributes).atLeastOnce();
        EasyMock.expect(buildInfo.getFile(EasyMock.eq("vts"))).
                andReturn(null);
        EasyMock.expect(buildInfo.getFile(EasyMock.eq("PYTHONPATH"))).
                andReturn(new File("DIR_NOT_EXIST")).atLeastOnce();
        EasyMock.expect(buildInfo.getFile(EasyMock.eq("VIRTUALENVPATH"))).
                andReturn(new File("DIR_NOT_EXIST"));
        EasyMock.expect(buildInfo.getFile(EasyMock.anyObject())).andReturn(null).anyTimes();
        EasyMock.replay(buildInfo);
        return buildInfo;
    }

    /**
     * Create a mock IRunUtil which sets environment variables and finds Python binary file.
     */
    private IRunUtil createMockRunUtil(String findFileCommand) {
        IRunUtil runUtil = EasyMock.createMock(IRunUtil.class);
        runUtil.setEnvVariable(EasyMock.eq("VTS"), EasyMock.eq("1"));
        EasyMock.expectLastCall();
        CommandResult findResult = new CommandResult();
        findResult.setStatus(CommandStatus.SUCCESS);
        findResult.setStdout(PYTHON_DIR + mPython);
        EasyMock.expect(runUtil.runTimedCmd(EasyMock.anyLong(), EasyMock.eq(findFileCommand),
                                EasyMock.eq(mPython)))
                .andReturn(findResult);
        EasyMock.replay(runUtil);
        return runUtil;
    }

    /**
     * Create a mock ITestDevice with necessary getter methods.
     */
    private static ITestDevice createMockDevice() {
        // TestDevice
        ITestDevice device = EasyMock.createMock(ITestDevice.class);
        try {
            EasyMock.expect(device.getSerialNumber()).
                andReturn("1234567890ABCXYZ").atLeastOnce();
            EasyMock.expect(device.getBuildAlias()).
                andReturn("BUILD_ALIAS");
            EasyMock.expect(device.getBuildFlavor()).
                andReturn("BUILD_FLAVOR");
            EasyMock.expect(device.getBuildId()).
                andReturn("BUILD_ID");
            EasyMock.expect(device.getProperty("ro.vts.coverage")).
                andReturn(null);
            EasyMock.expect(device.getProductType()).
                andReturn("PRODUCT_TYPE");
            EasyMock.expect(device.getProductVariant()).
                andReturn("PRODUCT_VARIANT");
            EasyMock.expect(device.executeShellCommand(EasyMock.startsWith("log")))
                    .andReturn(null)
                    .anyTimes();
        } catch (DeviceNotAvailableException e) {
            fail();
        }
        EasyMock.replay(device);
        return device;
    }

    /**
     * Test the run method with a normal input.
     */
    public void testRunNormalInput() {
        mProcessHelper = createMockProcessHelper(CommandStatus.SUCCESS);
        mTest.setDevice(createMockDevice());
        mTest.setRunUtil(createMockRunUtil("which"));
        try {
            mTest.run(mMockInvocationListener);
        } catch (IllegalArgumentException e) {
            // not expected
            fail();
            e.printStackTrace();
        } catch (DeviceNotAvailableException e) {
            // not expected
            fail();
            e.printStackTrace();
        }
    }

    /**
     * Test the run method with a normal input and Windows environment variable.
     */
    public void testRunNormalInputOnWindows()
            throws IllegalArgumentException, DeviceNotAvailableException {
        mProcessHelper = createMockProcessHelper(CommandStatus.SUCCESS);
        mPython = "python.exe";
        String originalName = System.getProperty(VtsMultiDeviceTest.OS_NAME);
        System.setProperty(VtsMultiDeviceTest.OS_NAME, VtsMultiDeviceTest.WINDOWS);
        mTest.setDevice(createMockDevice());
        mTest.setRunUtil(createMockRunUtil("where"));
        try {
            mTest.run(mMockInvocationListener);
        } finally {
            System.setProperty(VtsMultiDeviceTest.OS_NAME, originalName);
        }
    }

    /**
     * Test the run method when the device is set null.
     */
    public void testRunDeviceNotAvailable() {
        mTest.setDevice(null);
        try {
            mTest.run(mMockInvocationListener);
            fail();
       } catch (IllegalArgumentException e) {
            // not expected
            fail();
       } catch (DeviceNotAvailableException e) {
            // expected
       }
    }

    /**
     * Test the run method with abnormal input data.
     */
    public void testRunNormalInputCommandFailed() {
        mProcessHelper = createMockProcessHelper(CommandStatus.FAILED);
        mTest.setDevice(createMockDevice());
        mTest.setRunUtil(createMockRunUtil("which"));
        try {
            mTest.run(mMockInvocationListener);
            fail();
        } catch (RuntimeException e) {
            if (!"Failed to run VTS test".equals(e.getMessage())) {
                fail();
            }
            // expected
        } catch (DeviceNotAvailableException e) {
            // not expected
            fail();
            e.printStackTrace();
        }
    }

    /**
     * Test the run method in which the command times out.
     */
    public void testRunNormalInputTimeout() {
        mProcessHelper = createMockProcessHelper(CommandStatus.TIMED_OUT, CommandStatus.TIMED_OUT);
        mTest.setDevice(createMockDevice());
        mTest.setRunUtil(createMockRunUtil("which"));
        try {
            mTest.run(mMockInvocationListener);
        } catch (IllegalArgumentException e) {
            fail();
        } catch (DeviceNotAvailableException e) {
            fail();
        }
    }

    /**
     * Test the run method in which the command is interrupted.
     */
    public void testRunNormalInputInterrupted() {
        mProcessHelper = createMockProcessHelper(null, CommandStatus.SUCCESS);
        mTest.setDevice(createMockDevice());
        mTest.setRunUtil(createMockRunUtil("which"));
        try {
            mTest.run(mMockInvocationListener);
            fail();
        } catch (IllegalArgumentException e) {
            fail();
        } catch (RunInterruptedException e) {
            // expected
        } catch (DeviceNotAvailableException e) {
            fail();
        }
    }
}
