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
import android.net.ConnectivityManager;
import android.net.DhcpResults;
import android.net.EthernetManager;
import android.net.PppoeManager;
import android.net.IEthernetServiceListener;
import android.net.InterfaceConfiguration;
import android.net.IpConfiguration;
import android.net.IpConfiguration.IpAssignment;
import android.net.IpConfiguration.ProxySettings;
import android.net.LinkProperties;
import android.net.NetworkAgent;
import android.net.NetworkCapabilities;
import android.net.NetworkFactory;
import android.net.NetworkInfo;
import android.net.NetworkInfo.DetailedState;
import android.net.StaticIpConfiguration;
import android.net.ip.IpManager;
import android.net.ip.IpManager.ProvisioningConfiguration;
import android.net.RouteInfo;
import android.net.LinkAddress;
import android.net.NetworkUtils;

import android.os.Handler;
import android.os.IBinder;
import android.os.INetworkManagementService;
import android.os.Looper;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.text.TextUtils;
import android.util.Log;
import android.content.Intent;
import android.os.UserHandle;
import android.provider.Settings;

import java.io.FileDescriptor;
import java.io.File;
import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.Exception;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;

import com.android.internal.util.IndentingPrintWriter;
import com.android.server.net.BaseNetworkObserver;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.Arrays;
import java.util.concurrent.CountDownLatch;


/**
 * Manages connectivity for an Ethernet interface.
 *
 * Ethernet Interfaces may be present at boot time or appear after boot (e.g.,
 * for Ethernet adapters connected over USB). This class currently supports
 * only one interface. When an interface appears on the system (or is present
 * at boot time) this class will start tracking it and bring it up, and will
 * attempt to connect when requested. Any other interfaces that subsequently
 * appear will be ignored until the tracked interface disappears. Only
 * interfaces whose names match the <code>config_ethernet_iface_regex</code>
 * regular expression are tracked.
 *
 * This class reports a static network score of 70 when it is tracking an
 * interface and that interface's link is up, and a score of 0 otherwise.
 *
 * @hide
 */
class EthernetNetworkFactory {
    private static final String NETWORK_TYPE = "Ethernet";
    private static final String TAG = "EthernetNetworkFactory";
    private static final int NETWORK_SCORE = 70;
    private static final boolean DBG = true;
    private static final boolean VDBG = false;

    /** Tracks interface changes. Called from NetworkManagementService. */
    private InterfaceObserver mInterfaceObserver;

    /** For static IP configuration */
    private EthernetManager mEthernetManager;
    private PppoeManager mPppoeManager;

    /** To set link state and configure IP addresses. */
    private INetworkManagementService mNMService;

    /** All code runs here, including start(). */
    private Handler mHandler;

    /* To communicate with ConnectivityManager */
    private NetworkCapabilities mNetworkCapabilities;
    private NetworkAgent mNetworkAgent;
    private LocalNetworkFactory mFactory;
    private Context mContext;

    /** Product-dependent regular expression of interface names we track. */
    private static String mIfaceMatch = "";

    /** To notify Ethernet status. */
    private final RemoteCallbackList<IEthernetServiceListener> mListeners;

    /** Data members. All accesses to these must be on the handler thread. */
    private String mIface = "";
    private static String mIfaceTmp = "";
    private static String mPppIface = "ppp0";
    private boolean mPppoeConnected = false;
    private String mHwAddr;
    private boolean mLinkUp;
    private NetworkInfo mNetworkInfo;
    private LinkProperties mLinkProperties;
    private IpManager mIpManager;
    private boolean mNetworkRequested = false;
    public int mEthernetCurrentState = EthernetManager.ETHER_STATE_DISCONNECTED;
    private boolean mReconnecting;
    private IpAssignment mConnectMode;
    
    public String dumpEthCurrentState(int curState) {
        if (curState == EthernetManager.ETHER_STATE_DISCONNECTED)
            return "DISCONNECTED";
        else if (curState == EthernetManager.ETHER_STATE_CONNECTING)
            return "CONNECTING";
        else if (curState == EthernetManager.ETHER_STATE_CONNECTED)
            return "CONNECTED";
        else if (curState == EthernetManager.ETHER_STATE_DISCONNECTING)
            return "DISCONNECTING";
        return "DISCONNECTED";
    }

    private void sendEthernetStateChangedBroadcast(int curState) {
        if (mEthernetCurrentState == curState)
            return;
        Log.d(TAG, "sendEthernetStateChangedBroadcast: curState = " + dumpEthCurrentState(curState));
        mEthernetCurrentState = curState;
        final Intent intent = new Intent(EthernetManager.ETHERNET_STATE_CHANGED_ACTION);
        intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
        intent.putExtra(EthernetManager.EXTRA_ETHERNET_STATE, curState);
        mContext.sendStickyBroadcastAsUser(intent, UserHandle.ALL);
    }

    private String ReadFromFile(File file) {
        if((file != null) && file.exists()) {
            try {
                FileInputStream fin= new FileInputStream(file);
                BufferedReader reader= new BufferedReader(new InputStreamReader(fin));
                String flag = reader.readLine();
                fin.close();
                return flag;
            } catch(Exception e) {
                e.printStackTrace();
            }
        }
        return null;
    }

    EthernetNetworkFactory(RemoteCallbackList<IEthernetServiceListener> listeners) {
        initNetworkCapabilities();
        clearInfo();
        mListeners = listeners;
    }

    private class LocalNetworkFactory extends NetworkFactory {
        LocalNetworkFactory(String name, Context context, Looper looper) {
            super(looper, context, name, new NetworkCapabilities());
        }

        protected void startNetwork() {
            if (!mNetworkRequested) {
                mNetworkRequested = true;
                maybeStartIpManager();
            }
        }

        protected void stopNetwork() {
            mNetworkRequested = false;
            stopIpManager();
            sendEthernetStateChangedBroadcast(EthernetManager.ETHER_STATE_DISCONNECTED);
        }
    }

    private void clearInfo() {
        mLinkProperties = new LinkProperties();
        mNetworkInfo = new NetworkInfo(ConnectivityManager.TYPE_ETHERNET, 0, NETWORK_TYPE, "");
        mNetworkInfo.setExtraInfo(mHwAddr);
        mNetworkInfo.setIsAvailable(isTrackingInterface());
        try {
                mNMService.clearInterfaceAddresses(mIface);
        } catch (Exception e) {
                Log.e(TAG, "Failed to clear addresses or disable ipv6" + e);
        }
    }

    private void stopIpManager() {
        if (mIpManager != null) {
            mIpManager.shutdown();
            mIpManager = null;
        }
        // ConnectivityService will only forget our NetworkAgent if we send it a NetworkInfo object
        // with a state of DISCONNECTED or SUSPENDED. So we can't simply clear our NetworkInfo here:
        // that sets the state to IDLE, and ConnectivityService will still think we're connected.
        //
        mNetworkInfo.setDetailedState(DetailedState.DISCONNECTED, null, mHwAddr);
        synchronized(EthernetNetworkFactory.this) {
                if (mNetworkAgent != null) {
                        updateAgent();
                        mNetworkAgent = null;
                }
        }
        clearInfo();
    }

    /**
     * Updates interface state variables.
     * Called on link state changes or on startup.
     */
    private void updateInterfaceState(String iface, boolean up) {
        if (!mIface.equals(iface)) {
            return;
        }
        
        if (!mReconnecting)
            Log.d(TAG, "updateInterface: " + iface + " link " + (up ? "up" : "down"));
    
        if (up && mEthernetCurrentState != EthernetManager.ETHER_STATE_DISCONNECTED) {
            Log.d(TAG, "Already connected or connecting, skip connect");
            return;
        }

        mLinkUp = up;
        if (up) {
            maybeStartIpManager();
        } else {
            sendEthernetStateChangedBroadcast(EthernetManager.ETHER_STATE_DISCONNECTING);
            stopIpManager();
            if (mConnectMode == IpAssignment.PPPOE) {
                    mPppoeConnected = false;
                    Log.d(TAG, "pppoe stop");
                    mPppoeManager.stopPppoe();
            }
            sendEthernetStateChangedBroadcast(EthernetManager.ETHER_STATE_DISCONNECTED);
        }
    }
    
        // first disconnect, then connect
    public void reconnect(String iface) {
        Log.d(TAG, "reconnect:");
        mReconnecting = true;

        if (iface == null)
            iface = mIface;

        Log.d(TAG, "first disconnect");
        updateInterfaceState(iface, false);

        try {
            Thread.sleep(1000);
        } catch (InterruptedException ignore) {
        }

        Log.d(TAG, "then connect");
        updateInterfaceState(iface, true);
        mReconnecting = false;
    }

    public void disconnect(String iface) {
        Log.d(TAG, "disconnect:");
        mReconnecting = true;

        if (iface == null)
            iface = mIface;

        updateInterfaceState(iface, false);
        mReconnecting = false;
    }

    private class InterfaceObserver extends BaseNetworkObserver {
        @Override
        public void interfaceLinkStateChanged(String iface, boolean up) {
            Log.d(TAG, "interfaceLinkStateChanged: " + iface);
            mHandler.post(() -> {
                updateInterfaceState(iface, up);
            });
        }

        @Override
        public void interfaceAdded(String iface) {
            Log.d(TAG, "interfaceAdded: " + iface);
            mHandler.post(() -> {
                maybeTrackInterface(iface);
            });
            if (mPppIface.equals(iface) && mPppoeManager.getPppoePhyIface().equals(mIface)) {
                pppoeConnected();    
            }
        }

        @Override
        public void interfaceRemoved(String iface) {
            Log.d(TAG, "interfaceRemoved: " + iface);
            mHandler.post(() -> {
                if (stopTrackingInterface(iface)) {
                    trackFirstAvailableInterface();
                }
            });
            if (mPppIface.equals(iface) && mPppoeManager.getPppoePhyIface().equals(mIface)) {
                pppoeDisconnected();    
            }
        }
    }

    private void pppoeDisconnected() {
        if (!mPppoeConnected) {
            if (VDBG) Log.d(TAG, "Pppoe already disconnected, skip disconnect");
            return ;    
        }
        try {
            Thread.sleep(2000);        //wait pppd killed
        } catch (InterruptedException ignore) {
        }
        if (mEthernetCurrentState == EthernetManager.ETHER_STATE_DISCONNECTED) {
            if (VDBG) Log.d(TAG, "Already disconnected, skip disconnect");
            return ;
        }
        Log.d(TAG, "pppoe stop by terminated");
        stopIpManager();
        mPppoeConnected = false;
        sendEthernetStateChangedBroadcast(EthernetManager.ETHER_STATE_DISCONNECTED);
    }
 
    private void pppoeConnected() {
        if (mPppoeConnected) {
            if (VDBG) Log.d(TAG, "Pppoe already connected, skip connect");
            return ;    
        }
        try {
             Thread.sleep(4000);          //wait pppoe connected
        } catch (InterruptedException ignore) {
        }
        
        if(mPppoeManager.getPppoeState() != PppoeManager.PPPOE_STATE_CONNECT) {
            if (VDBG) Log.d(TAG, "getPppoeState() != PPPOE_STATE_CONNECT , skip pppoeConnected");
            return;
        }
        Log.d(TAG, "pppoe auto connected");
        LinkProperties mPppLinkProperties = mPppoeManager.getLinkProperties();
        String iface = mPppoeManager.getPppIfaceName();
        mPppLinkProperties.setInterfaceName(iface);
        mHandler.post(() -> onIpLayerStarted(mPppLinkProperties));
        mPppoeConnected = true;
    }

    private void setInterfaceUp(String iface) {
        // Bring up the interface so we get link status indications.
        Log.d(TAG, "setInterfaceUp: " + iface);
        try {
            mNMService.setInterfaceUp(iface);
            String hwAddr = null;
            InterfaceConfiguration config = mNMService.getInterfaceConfig(iface);

            if (config == null) {
                Log.e(TAG, "Null interface config for " + iface + ". Bailing out.");
                return;
            }

            if (!isTrackingInterface()) {
                setInterfaceInfo(iface, config.getHardwareAddress());
                mNetworkInfo.setIsAvailable(true);
                mNetworkInfo.setExtraInfo(mHwAddr);
            } else {
                Log.e(TAG, "Interface unexpectedly changed from " + iface + " to " + mIface);
                mNMService.setInterfaceDown(iface);
            }
        } catch (RemoteException | IllegalStateException e) {
            // Either the system is crashing or the interface has disappeared. Just ignore the
            // error; we haven't modified any state because we only do that if our calls succeed.
            Log.e(TAG, "Error upping interface " + mIface + ": " + e);
        }
    }

    private boolean maybeTrackInterface(String iface) {
        // If we don't already have an interface, and if this interface matches
        // our regex, start tracking it.
        if (!iface.matches(mIfaceMatch) || isTrackingInterface())
            return false;

        Log.d(TAG, "Started tracking interface " + iface);
        setInterfaceUp(iface);
        return true;
    }

    private boolean stopTrackingInterface(String iface) {
        if (!iface.equals(mIface))
            return false;

        Log.d(TAG, "Stopped tracking interface " + iface);
        setInterfaceInfo("", null);
        stopIpManager();
        sendEthernetStateChangedBroadcast(EthernetManager.ETHER_STATE_DISCONNECTED);
        return true;
    }

    private boolean setStaticIpAddress(StaticIpConfiguration staticConfig) {
        if (staticConfig.ipAddress != null &&
                staticConfig.gateway != null &&
                staticConfig.dnsServers.size() > 0) {
            try {
                Log.i(TAG, "Applying static IPv4 configuration to " + mIface + ": " + staticConfig);
                InterfaceConfiguration config = mNMService.getInterfaceConfig(mIface);
                config.setLinkAddress(staticConfig.ipAddress);
                mNMService.setInterfaceConfig(mIface, config);
                return true;
            } catch(RemoteException|IllegalStateException e) {
               Log.e(TAG, "Setting static IP address failed: " + e.getMessage());
            }
        } else {
            Log.e(TAG, "Invalid static IP configuration.");
        }
        return false;
    }

    public void updateAgent() {
        synchronized (EthernetNetworkFactory.this) {
                if (mNetworkAgent == null) return;
                if (DBG) {
                        Log.i(TAG, "Updating mNetworkAgent with: " +
                        mNetworkCapabilities + ", " +
                        mNetworkInfo + ", " +
                        mLinkProperties);
                }
                mNetworkAgent.sendNetworkCapabilities(mNetworkCapabilities);
                mNetworkAgent.sendNetworkInfo(mNetworkInfo);
                mNetworkAgent.sendLinkProperties(mLinkProperties);
                // never set the network score below 0.
                mNetworkAgent.sendNetworkScore(mLinkUp? NETWORK_SCORE : 0);
        }
    }

    void onIpLayerStarted(LinkProperties linkProperties) {
        synchronized (EthernetNetworkFactory.this) {
                if (mNetworkAgent != null) {
                        Log.e(TAG, "Already have a NetworkAgent - aborting new request");
                        stopIpManager();
                        sendEthernetStateChangedBroadcast(EthernetManager.ETHER_STATE_DISCONNECTED);
                        return;
                }
        }
        Log.d(TAG, "IP success: lp = " + linkProperties);
        mLinkProperties = linkProperties;
        mNetworkInfo.setDetailedState(DetailedState.CONNECTED, null, mHwAddr);
        sendEthernetStateChangedBroadcast(EthernetManager.ETHER_STATE_CONNECTED);

        // Create our NetworkAgent.
        mNetworkAgent = new NetworkAgent(mHandler.getLooper(), mContext,
                NETWORK_TYPE, mNetworkInfo, mNetworkCapabilities, mLinkProperties,
                NETWORK_SCORE) {
            public void unwanted() {
                synchronized(EthernetNetworkFactory.this) {
                        Log.d(TAG, "unwanted");
                        if (this == mNetworkAgent) {
                                stopIpManager();
                                sendEthernetStateChangedBroadcast(EthernetManager.ETHER_STATE_DISCONNECTED);
                        } else if (mNetworkAgent != null) {
                                Log.d(TAG, "Ignoring unwanted as we have a more modern " + "instance");
                        }  // Otherwise, we've already called stopIpManager.
                }
            }
        };
    }

    void onIpLayerStopped(LinkProperties linkProperties) {
        // This cannot happen due to provisioning timeout, because our timeout is 0. It can only
        // happen if we're provisioned and we lose provisioning.
        stopIpManager();
        sendEthernetStateChangedBroadcast(EthernetManager.ETHER_STATE_DISCONNECTED);
        maybeStartIpManager();
    }

    void updateLinkProperties(LinkProperties linkProperties) {
        mLinkProperties = linkProperties;
        synchronized(EthernetNetworkFactory.this) {
                if (mNetworkAgent != null) {
                        mNetworkAgent.sendLinkProperties(linkProperties);
                }
        }
    }

    public void maybeStartIpManager() {
        if (mNetworkRequested && mIpManager == null && isTrackingInterface()) {
            startIpManager();
        }
    }

    public void startIpManager() {
        if (DBG) {
            Log.d(TAG, String.format("starting IpManager(%s): mNetworkInfo=%s", mIface,
                    mNetworkInfo));
        }
        int carrier = getEthernetCarrierState(mIface);
        Log.d(TAG, "startIpManager: " + mIface + " carrier = " + carrier);
        if (carrier != 1) {
            return;
        }
        if (mEthernetCurrentState == EthernetManager.ETHER_STATE_CONNECTED) {
             Log.d(TAG, "Already connected, skip connect");
             return ;
        }

        sendEthernetStateChangedBroadcast(EthernetManager.ETHER_STATE_CONNECTING);
        final Thread ipProvisioningThread = new Thread(new Runnable() {
        public void run() {
        LinkProperties linkProperties;
        IpConfiguration config = mEthernetManager.getConfiguration();
        mConnectMode = config.getIpAssignment();
        final String tcpBufferSizes = mContext.getResources().getString(
                    com.android.internal.R.string.config_ethernet_tcp_buffers);
        if (mPppoeManager == null && mConnectMode == IpAssignment.PPPOE) {
            mConnectMode = IpAssignment.DHCP;
            Log.d(TAG, "mPppoeManager == null, set mConnectMode to DHCP");
        }
        if (config.getIpAssignment() == IpAssignment.STATIC) {
            Log.d(TAG, "config STATIC");
            if (!setStaticIpAddress(config.getStaticIpConfiguration())) {
                // We've already logged an error.
                sendEthernetStateChangedBroadcast(EthernetManager.ETHER_STATE_DISCONNECTED);
                return;
            }
            linkProperties = config.getStaticIpConfiguration().toLinkProperties(mIface);
            linkProperties.setTcpBufferSizes(tcpBufferSizes);
            if (config.getProxySettings() == ProxySettings.STATIC ||
                                config.getProxySettings() == ProxySettings.PAC) {
                linkProperties.setHttpProxy(config.getHttpProxy());
            }
            mHandler.post(() -> onIpLayerStarted(linkProperties));
        } else if (config.getIpAssignment() == IpAssignment.PPPOE) {
            Log.d(TAG, "config PPPOE");
            Log.d(TAG, "start pppoe connect: " + config.pppoeAccount + ", " + config.pppoePassword);
            mPppoeConnected = true; 
            mPppoeManager.connect(config.pppoeAccount, config.pppoePassword, mIface);

            int state = mPppoeManager.getPppoeState();
            Log.d(TAG, "end pppoe connect: state = " + mPppoeManager.dumpCurrentState(state));
            if (state == PppoeManager.PPPOE_STATE_CONNECT) {
                linkProperties = mPppoeManager.getLinkProperties();
                String iface = mPppoeManager.getPppIfaceName();
                linkProperties.setInterfaceName(iface);
                linkProperties.setTcpBufferSizes(tcpBufferSizes);
                if (config.getProxySettings() == ProxySettings.STATIC ||
                    config.getProxySettings() == ProxySettings.PAC) {
                    linkProperties.setHttpProxy(config.getHttpProxy());
                }
                mHandler.post(() -> onIpLayerStarted(linkProperties));
            } else {
                Log.e(TAG, "pppoe connect failed.");
                mPppoeConnected = false; 
                sendEthernetStateChangedBroadcast(EthernetManager.ETHER_STATE_DISCONNECTED);
                return;
            }
        } else {
            Log.d(TAG, "config DHCP");
            mNetworkInfo.setDetailedState(DetailedState.OBTAINING_IPADDR, null, mHwAddr);
            IpManager.Callback ipmCallback = new IpManager.Callback() {
                @Override
                public void onProvisioningSuccess(LinkProperties newLp) {
                    Log.d(TAG, "onProvisioningSuccess: lp = " + newLp);
                    mHandler.post(() -> onIpLayerStarted(newLp));
                }

                @Override
                public void onProvisioningFailure(LinkProperties newLp) {
                    Log.d(TAG, "onProvisioningFailure: lp = " + newLp);
                    mHandler.post(() -> onIpLayerStopped(newLp));
                }

                @Override
                public void onLinkPropertiesChange(LinkProperties newLp) {
                    Log.d(TAG, "onLinkPropertiesChange: lp = " + newLp);
                    mHandler.post(() -> updateLinkProperties(newLp));
                }
            };

            synchronized(EthernetNetworkFactory.this) {
            stopIpManager();
            if (!isTrackingInterface()) {
                sendEthernetStateChangedBroadcast(EthernetManager.ETHER_STATE_DISCONNECTED);
                return;
            }
            mIpManager = new IpManager(mContext, mIface, ipmCallback);

            if (config.getProxySettings() == ProxySettings.STATIC ||
                    config.getProxySettings() == ProxySettings.PAC) {
                mIpManager.setHttpProxy(config.getHttpProxy());
            }

            if (!TextUtils.isEmpty(tcpBufferSizes)) {
                mIpManager.setTcpBufferSizes(tcpBufferSizes);
            }

            final ProvisioningConfiguration provisioningConfiguration =
                    mIpManager.buildProvisioningConfiguration()
                            .withProvisioningTimeoutMs(0)
                            .build();
            mIpManager.startProvisioning(provisioningConfiguration);
            
            }
        }
        }});ipProvisioningThread.start();
    }

    /**
     * Begin monitoring connectivity
     */
    public void start(Context context, Handler handler) {
        mHandler = handler;

        // The services we use.
        IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
        mNMService = INetworkManagementService.Stub.asInterface(b);
        mEthernetManager = (EthernetManager) context.getSystemService(Context.ETHERNET_SERVICE);
        mPppoeManager = (PppoeManager) context.getSystemService(Context.PPPOE_SERVICE);

        // Interface match regex.
        mIfaceMatch = context.getResources().getString(
                com.android.internal.R.string.config_ethernet_iface_regex);
        Log.d(TAG, "EthernetNetworkFactory start " + mIfaceMatch);

        // Create and register our NetworkFactory.
        mFactory = new LocalNetworkFactory(NETWORK_TYPE, context, mHandler.getLooper());
        mFactory.setCapabilityFilter(mNetworkCapabilities);
        mFactory.setScoreFilter(NETWORK_SCORE);
        mFactory.register();

        mContext = context;
        mReconnecting = false;
        mConnectMode = IpAssignment.DHCP;

        // Start tracking interface change events.
        mInterfaceObserver = new InterfaceObserver();
        try {
            mNMService.registerObserver(mInterfaceObserver);
        } catch (RemoteException e) {
            Log.e(TAG, "Could not register InterfaceObserver " + e);
        }

        // If an Ethernet interface is already connected, start tracking that.
        // Otherwise, the first Ethernet interface to appear will be tracked.
        mHandler.post(() -> trackFirstAvailableInterface());
    }

    public void trackFirstAvailableInterface() {
        try {
            final String[] ifaces = mNMService.listInterfaces();
            for (String iface : ifaces) {
                if (maybeTrackInterface(iface)) {
                    // We have our interface. Track it.
                    // Note: if the interface already has link (e.g., if we crashed and got
                    // restarted while it was running), we need to fake a link up notification so we
                    // start configuring it.
                    //if (mNMService.getInterfaceConfig(iface).hasFlag("running")) {
                    mIfaceTmp = iface;
                    new Thread(new Runnable() {
                        public void run() {
                            // carrier is always 1 when kernel boot up no matter RJ45 plugin or not,
                            // sleep a little time to wait kernel's correct carrier status
                            try {
                                Thread.sleep(3000);
                            } catch (InterruptedException ignore) {
                            }
                            int carrier = getEthernetCarrierState(mIfaceTmp);
                            Log.d(TAG, mIfaceTmp + " carrier = " + carrier);
                            if (carrier == 1) {
                                updateInterfaceState(mIfaceTmp, true);
                            } else if (mEthernetCurrentState != EthernetManager.ETHER_STATE_DISCONNECTED ){
                                updateInterfaceState(mIfaceTmp, false);
                            }
                        }
                    }).start();
                    break;
                }
            }
        } catch (RemoteException|IllegalStateException e) {
            Log.e(TAG, "Could not get list of interfaces " + e);
        }
    }

    public void stop() {
        Log.d(TAG, "EthernetNetworkFactory stop");
        stopIpManager();
        setInterfaceInfo("", null);
        mFactory.unregister();
        sendEthernetStateChangedBroadcast(EthernetManager.ETHER_STATE_DISCONNECTED);
    }

    private void initNetworkCapabilities() {
        mNetworkCapabilities = new NetworkCapabilities();
        mNetworkCapabilities.addTransportType(NetworkCapabilities.TRANSPORT_ETHERNET);
        mNetworkCapabilities.addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET);
        mNetworkCapabilities.addCapability(NetworkCapabilities.NET_CAPABILITY_NOT_RESTRICTED);
        // We have no useful data on bandwidth. Say 100M up and 100M down. :-(
        mNetworkCapabilities.setLinkUpstreamBandwidthKbps(100 * 1000);
        mNetworkCapabilities.setLinkDownstreamBandwidthKbps(100 * 1000);
    }

    public boolean isTrackingInterface() {
        return !TextUtils.isEmpty(mIface);
    }
    
    public int getEthernetCarrierState(String ifname) {
        if(ifname != "") {
            try {
                File file = new File("/sys/class/net/" + ifname + "/carrier");
                String carrier = ReadFromFile(file);
                return Integer.parseInt(carrier);
            } catch(Exception e) {
                e.printStackTrace();
                return 0;
            }
        } else {
            return 0;
        }
    }
    
        public String getEthernetMacAddress(String ifname) {
        if(ifname != "") {
            try {
                File file = new File("/sys/class/net/" + ifname + "/address");
                String address = ReadFromFile(file);
                return address;
            } catch(Exception e) {
                e.printStackTrace();
                return "";
            }
        } else {
            return "";
        }
    }

    public String getIpAddress() {
        IpConfiguration config = mEthernetManager.getConfiguration();
        if (config.getIpAssignment() == IpAssignment.STATIC) {
            return config.getStaticIpConfiguration().ipAddress.getAddress().getHostAddress();
        } else {
            for (LinkAddress l : mLinkProperties.getLinkAddresses()) {
                InetAddress source = l.getAddress();
                //Log.d(TAG, "getIpAddress: " + source.getHostAddress());
                if (source instanceof Inet4Address) {
                    return source.getHostAddress();
                }
            }
        }
        return "";
    }

    private String prefix2netmask(int prefix) {
        // convert prefix to netmask
        if (true) {
            int mask = 0xFFFFFFFF << (32 - prefix);
            //Log.d(TAG, "mask = " + mask + " prefix = " + prefix);
            return ((mask>>>24) & 0xff) + "." + ((mask>>>16) & 0xff) + "." + ((mask>>>8) & 0xff) + "." + ((mask) & 0xff);
    	   } else {
    	       return NetworkUtils.intToInetAddress(NetworkUtils.prefixLengthToNetmaskInt(prefix)).getHostName();
    	   }
    }

    public String getNetmask() {
        IpConfiguration config = mEthernetManager.getConfiguration();
        if (config.getIpAssignment() == IpAssignment.STATIC) {
            return prefix2netmask(config.getStaticIpConfiguration().ipAddress.getPrefixLength());
        } else {
            for (LinkAddress l : mLinkProperties.getLinkAddresses()) {
                InetAddress source = l.getAddress();
                if (source instanceof Inet4Address) {
                    return prefix2netmask(l.getPrefixLength());
                }
            }
        }
        return "";
    }
	
    public String getGateway() {
        IpConfiguration config = mEthernetManager.getConfiguration();
        if (config.getIpAssignment() == IpAssignment.STATIC) {
            return config.getStaticIpConfiguration().gateway.getHostAddress();
        } else {
            for (RouteInfo route : mLinkProperties.getRoutes()) {
                if (route.hasGateway()) {
                    InetAddress gateway = route.getGateway();
                    if (route.isIPv4Default()) {
                        return gateway.getHostAddress();
                    }
                }
            }
        }
        return "";		
    }

    /*
     * return dns format: "8.8.8.8,4.4.4.4"
     */
    public String getDns() {
        String dns = "";
        IpConfiguration config = mEthernetManager.getConfiguration();
        if (config.getIpAssignment() == IpAssignment.STATIC) {
            for (InetAddress nameserver : config.getStaticIpConfiguration().dnsServers) {
                dns += nameserver.getHostAddress() + ",";
            }			
        } else {		
            for (InetAddress nameserver : mLinkProperties.getDnsServers()) {
                dns += nameserver.getHostAddress() + ",";
            }
        }
        return dns;		
    }


    /**
     * Set interface information and notify listeners if availability is changed.
     */
    private void setInterfaceInfo(String iface, String hwAddr) {
        boolean oldAvailable = isTrackingInterface();
        mIface = iface;
        mHwAddr = hwAddr;
        boolean available = isTrackingInterface();

        mNetworkInfo.setExtraInfo(mHwAddr);
        mNetworkInfo.setIsAvailable(available);

        if (oldAvailable != available) {
            int n = mListeners.beginBroadcast();
            for (int i = 0; i < n; i++) {
                try {
                    mListeners.getBroadcastItem(i).onAvailabilityChanged(available);
                } catch (RemoteException e) {
                    // Do nothing here.
                }
            }
            mListeners.finishBroadcast();
        }
    }

    private void postAndWaitForRunnable(Runnable r) throws InterruptedException {
        CountDownLatch latch = new CountDownLatch(1);
        mHandler.post(() -> {
            try {
                r.run();
            } finally {
                latch.countDown();
            }
        });
        latch.await();
    }


    void dump(FileDescriptor fd, IndentingPrintWriter pw, String[] args) {
        try {
            postAndWaitForRunnable(() -> {
                pw.println("Network Requested: " + mNetworkRequested);
                if (isTrackingInterface()) {
                    pw.println("Tracking interface: " + mIface);
                    pw.increaseIndent();
                    pw.println("MAC address: " + mHwAddr);
                    pw.println("Link state: " + (mLinkUp ? "up" : "down"));
                    pw.decreaseIndent();
                } else {
                    pw.println("Not tracking any interface");
                }
                
                pw.println();
                pw.println("mEthernetCurrentState: " + dumpEthCurrentState(mEthernetCurrentState));

                pw.println();
                pw.println("NetworkInfo: " + mNetworkInfo);
                pw.println("LinkProperties: " + mLinkProperties);
                pw.println("NetworkAgent: " + mNetworkAgent);
                if (mIpManager != null) {
                    pw.println("IpManager:");
                    pw.increaseIndent();
                    mIpManager.dump(fd, pw, args);
                    pw.decreaseIndent();
                }
            });
        } catch (InterruptedException e) {
            throw new IllegalStateException("dump() interrupted");
        }
    }
}
