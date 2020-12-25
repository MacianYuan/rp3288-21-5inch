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

package com.android.tradefed.util;

import com.android.tradefed.util.CommandStatus;
import com.android.tradefed.util.IRunUtil;
import com.android.tradefed.util.ProcessHelper;
import com.android.tradefed.util.RunInterruptedException;
import com.android.tradefed.util.RunUtil;

import junit.framework.TestCase;

import java.io.IOException;

/**
 * Test cases for {@link ProcessHelper}.
 */
public class ProcessHelperTest extends TestCase {
    private ProcessHelper mProcess;

    /**
     * Reset the ProcessHelper
     */
    @Override
    public void setUp() {
        mProcess = null;
    }

    /**
     * Terminate the process, join threads and close IO streams.
     */
    @Override
    public void tearDown() {
        if (mProcess != null) {
            mProcess.cleanUp();
        }
    }

    /**
     * Test running a process that returns zero.
     */
    public void testSuccess() throws IOException {
        mProcess = new ProcessHelper(new ProcessBuilder("echo", "123").start());
        CommandStatus status = mProcess.waitForProcess(1000);
        assertEquals(CommandStatus.SUCCESS, status);
        assertFalse(mProcess.isRunning());
        assertTrue(mProcess.getStdout().equals("123\n"));
        assertTrue(mProcess.getStderr().isEmpty());
    }

    /**
     * Test running a process that returns non-zero.
     */
    public void testFailure() throws IOException {
        mProcess = new ProcessHelper(new ProcessBuilder("ls", "--WRONG-OPTION").start());
        CommandStatus status = mProcess.waitForProcess(1000);
        assertEquals(CommandStatus.FAILED, status);
        assertFalse(mProcess.isRunning());
        assertTrue(mProcess.getStdout().isEmpty());
        assertTrue(mProcess.getStderr().contains("unrecognized option"));
    }

    /**
     * Test running a process that times out.
     */
    public void testTimeout() throws IOException {
        mProcess = new ProcessHelper(new ProcessBuilder("cat").start());
        CommandStatus status = mProcess.waitForProcess(30);
        assertEquals(CommandStatus.TIMED_OUT, status);
        assertTrue(mProcess.isRunning());
    }

    /**
     * Test running a process and being interrupted.
     */
    public void testInterrupt() throws IOException, InterruptedException {
        mProcess = new ProcessHelper(new ProcessBuilder("cat").start());
        IRunUtil runUtil = RunUtil.getDefault();
        Thread testThread = Thread.currentThread();

        Thread timer = new Thread() {
            @Override
            public void run() {
                try {
                    Thread.sleep(50);
                } catch (InterruptedException e) {
                    fail();
                }
                runUtil.interrupt(testThread, "unit test");
            }
        };

        runUtil.allowInterrupt(true);
        timer.start();
        try {
            mProcess.waitForProcess(100);
            fail();
        } catch (RunInterruptedException e) {
            assertTrue(mProcess.isRunning());
        } finally {
            timer.join(1000);
        }
    }
}
