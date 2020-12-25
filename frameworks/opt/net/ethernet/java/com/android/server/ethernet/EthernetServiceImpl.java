/*
 * Copyright (C) 2014 The Android Open Source Project
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

package com.android.server.ethernet;

import android.content.Context;
import android.content.pm.PackageManager;
import android.net.IEthernetManager;
import android.net.IEthernetServiceListener;
import android.net.IpConfiguration;
import android.net.IpConfiguration.IpAssignment;
import android.net.IpConfiguration.ProxySettings;
import android.os.Binder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.provider.Settings;
import android.util.Log;
import android.util.PrintWriterPrinter;

import com.android.internal.util.IndentingPrintWriter;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * EthernetServiceImpl handles remote Ethernet operation requests by implementing
 * the IEthernetManager interface.
 *
 * @hide
 */
public class EthernetServiceImpl extends IEthernetManager.Stub {
    private static final String TAG = "EthernetServiceImpl";

    private final Context mContext;
    private final EthernetConfigStore mEthernetConfigStore;
    private final AtomicBoolean mStarted = new AtomicBoolean(false);
    private IpConfiguration mIpConfiguration;

    private Handler mHandler;
    private final EthernetNetworkFactory mTracker;
    private final RemoteCallbackList<IEthernetServiceListener> mListeners =
            new RemoteCallbackList<IEthernetServiceListener>();

    public EthernetServiceImpl(Context context) {
        mContext = context;
        Log.i(TAG, "Creating EthernetConfigStore");
        mEthernetConfigStore = new EthernetConfigStore();
        mIpConfiguration = mEthernetConfigStore.readIpAndProxyConfigurations();

        Log.i(TAG, "Read stored IP configuration: " + mIpConfiguration);

        mTracker = new EthernetNetworkFactory(mListeners);
    }

    private void enforceAccessPermission() {
        mContext.enforceCallingOrSelfPermission(
                android.Manifest.permission.ACCESS_NETWORK_STATE,
                "EthernetService");
    }

    private void enforceConnectivityInternalPermission() {
        mContext.enforceCallingOrSelfPermission(
                android.Manifest.permission.CONNECTIVITY_INTERNAL,
                "ConnectivityService");
    }

    public void start() {
        Log.i(TAG, "Starting Ethernet service");

        HandlerThread handlerThread = new HandlerThread("EthernetServiceThread");
        handlerThread.start();
        mHandler = new Handler(handlerThread.getLooper());

        mTracker.start(mContext, mHandler);

        mStarted.set(true);
    }

    /**
     * Get Ethernet configuration
     * @return the Ethernet Configuration, contained in {@link IpConfiguration}.
     */
    @Override
    public IpConfiguration getConfiguration() {
        enforceAccessPermission();

        synchronized (mIpConfiguration) {
            return new IpConfiguration(mIpConfiguration);
        }
    }

    /**
     * Set Ethernet configuration
     */
    @Override
    public void setConfiguration(IpConfiguration config) {
        if (!mStarted.get()) {
            Log.w(TAG, "System isn't ready enough to change ethernet configuration");
        }

        enforceConnectivityInternalPermission();
        Log.d(TAG, "setConfiguration: " + config.pppoeAccount + ", " + config.pppoePassword);

        synchronized (mIpConfiguration) {
            mEthernetConfigStore.writeIpAndProxyConfigurations(config);

            // TODO: this does not check proxy settings, gateways, etc.
            // Fix this by making IpConfiguration a complete representation of static configuration.
            if (true/*!config.equals(mIpConfiguration)*/) {
                mIpConfiguration = new IpConfiguration(config);
                if (false) { // old android original method
                   mTracker.stop();
                   mTracker.start(mContext, mHandler);
                } else { // new method
                    mTracker.reconnect("eth0");
                }
            }
        }
    }

    /**
     * Indicates whether the system currently has one or more
     * Ethernet interfaces.
     */
    @Override
    public boolean isAvailable() {
        enforceAccessPermission();
        return mTracker.isTrackingInterface();
    }

    /**
     * Addes a listener.
     * @param listener A {@link IEthernetServiceListener} to add.
     */
    public void addListener(IEthernetServiceListener listener) {
        if (listener == null) {
            throw new IllegalArgumentException("listener must not be null");
        }
        enforceAccessPermission();
        mListeners.register(listener);
    }

    /**
     * Removes a listener.
     * @param listener A {@link IEthernetServiceListener} to remove.
     */
    public void removeListener(IEthernetServiceListener listener) {
        if (listener == null) {
            throw new IllegalArgumentException("listener must not be null");
        }
        enforceAccessPermission();
        mListeners.unregister(listener);
    }
    
    @Override
    public int getEthernetCarrierState(String ifname) {
        enforceAccessPermission();
        return mTracker.getEthernetCarrierState(ifname);
    }

    @Override
    public String getEthernetMacAddress(String ifname) {
        enforceAccessPermission();
        return mTracker.getEthernetMacAddress(ifname);
    }

    @Override
    public int getEthernetConnectState() {
        enforceAccessPermission();
        return mTracker.mEthernetCurrentState;
    }

    @Override
    public String getIpAddress() {
        enforceAccessPermission();
        return mTracker.getIpAddress();
    }

    @Override
    public String getNetmask() {
        enforceAccessPermission();
        return mTracker.getNetmask();
    }

    @Override
    public String getGateway() {
        enforceAccessPermission();
        return mTracker.getGateway();
    }

    @Override
    public String getDns() {
        enforceAccessPermission();
        return mTracker.getDns();
    }

    @Override
    public String dumpCurrentState(int state) {
        enforceAccessPermission();
        return mTracker.dumpEthCurrentState(state);
    }

    @Override
    public void reconnect(String iface) {
        enforceAccessPermission();
        mTracker.reconnect(iface);
    }

    @Override
    public void disconnect(String iface) {
        enforceAccessPermission();
        mTracker.disconnect(iface);
    }

    @Override
    protected void dump(FileDescriptor fd, PrintWriter writer, String[] args) {
        final IndentingPrintWriter pw = new IndentingPrintWriter(writer, "  ");
        if (mContext.checkCallingOrSelfPermission(android.Manifest.permission.DUMP)
                != PackageManager.PERMISSION_GRANTED) {
            pw.println("Permission Denial: can't dump EthernetService from pid="
                    + Binder.getCallingPid()
                    + ", uid=" + Binder.getCallingUid());
            return;
        }

        pw.println("Current Ethernet state: ");
        pw.increaseIndent();
        mTracker.dump(fd, pw, args);
        pw.decreaseIndent();

        pw.println();
        pw.println("Stored Ethernet configuration: ");
        pw.increaseIndent();
        pw.println(mIpConfiguration);
        pw.decreaseIndent();

        pw.println("Handler:");
        pw.increaseIndent();
        mHandler.dump(new PrintWriterPrinter(pw), "EthernetServiceImpl");
        pw.decreaseIndent();
    }
}
