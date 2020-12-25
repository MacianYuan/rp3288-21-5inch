/*$_FOR_ROCKCHIP_RBOX_$*/
//$_rbox_$_modify_$_chenzhi_20120309: add for pppoe
/*  
 *  Copyright(C), 2009-2010, Fuzhou Rockchip Co. ,Ltd.  All Rights Reserved.
 *
 *  File:   PppoeService.java
 *  Desc:   
 *  Usage:        
 *  Note:
 *  Author: cz@rock-chips.com
 *  Version:
 *          v1.0
 *  Log:
    ----Thu Sep 8 2011            v1.0
 */

package com.android.server.pppoe;

import android.app.AlarmManager;
import android.app.PendingIntent;

import android.content.BroadcastReceiver;

import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.net.IPppoeManager;
import android.net.PppoeManager;
import android.net.DhcpInfo;
import android.net.NetworkInfo;
import android.net.NetworkInfo.DetailedState;
import android.net.LinkProperties;
import android.net.RouteInfo;
import android.net.LinkAddress;
import android.os.Binder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.Process;
import android.os.RemoteException;
import android.os.UEventObserver;
import android.provider.Settings;
import android.util.Log;
import android.text.TextUtils;
import android.os.INetworkManagementService;
import android.os.ServiceManager;
import android.os.UserHandle;

import java.util.ArrayList;
import java.util.BitSet;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Pattern;
import java.io.FileDescriptor;
import java.io.PrintWriter;
import android.net.NetworkUtils;

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

//import android.net.ethernet.DhcpInfo;
import android.os.SystemProperties;

public class PppoeServiceImpl extends IPppoeManager.Stub {
    private static final String TAG = "PppoeServiceImpl";
    private static final boolean DEBUG = true;
    // if true:
    //     for cmcc platform setting, should braodcast different EXTRA_PPPOE_STATE value to cmcc settings
    // if false:
    //     for box setting2 and android origin setting, braodcast normal EXTRA_PPPOE_STATE value
    private static final boolean FOR_CMCC_PLATFORM = false;
    private static void LOG(String msg) {
        if ( DEBUG ) {
            Log.d(TAG, msg);
        }
    }

    private Context mContext;
    private PppoeObserver mObserver;
    private NetworkInfo mNetworkInfo;
    int mPppoeState = PppoeManager.PPPOE_STATE_DISCONNECT;
    private String mIface="eth0"; //default eth0
    private String mWifiIface = "wlan0";
    private String mEthIface = "eth0";
    private String mUser;
    private String mPassword;
    private boolean mSameUserPwd;
    private String mPppIface = "ppp0";
    private int mWiFiEthernetSwitchMode;  // 0: no switch;
                                          // 1: ethernet pppoe switch to wifi pppoe; 
                                          // 2: wifi pppoe switch to ethernet pppoe;
    private static final int PPPOE_RETRY_COUNT = 1;
    /*-------------------------------------------------------*/

    static {
        /* Native functions are defined in libpppoe-jni.so */
        System.loadLibrary("pppoe-jni");
        registerNatives();
    }

    PppoeServiceImpl(Context context) {
        LOG("PppoeServiceImpl() : Entered.");
       
        mContext = context;
        mObserver = new PppoeObserver(mContext);
        mSameUserPwd = false;
        mWiFiEthernetSwitchMode = 0;
        mIface = Settings.Secure.getString(mContext.getContentResolver(), Settings.Secure.PPPOE_PHY_IFACE);
        if (mIface == null) {
            mIface = mEthIface;
            LOG("set default mIface = " + mIface); 
        }
        LOG("mIface = " + mIface);
        Settings.Secure.putString(mContext.getContentResolver(), Settings.Secure.PPPOE_PHY_IFACE, mIface);
        
        // if pppoe zero config is on, default enable ethernet pppoe, set default count, password
        String zeroconfig = SystemProperties.get("persist.sys.pppoe.enable");
        if ( zeroconfig.equals("1")) {
        	LOG("PPPOE zero config is enable");
        	String user = Settings.Secure.getString(mContext.getContentResolver(),Settings.Secure.PPPOE_USERNAME);
        	LOG("get current pppoe user = " + user);
        	if (user == null) {
        		String defusr = SystemProperties.get("persist.sys.pppoe.user");
        		String defpwd = SystemProperties.get("persist.sys.pppoe.pwd");
        		LOG("set default pppoe user = " + defusr + ", pwd = " + defpwd);
        		Settings.Secure.putString(mContext.getContentResolver(), Settings.Secure.PPPOE_USERNAME,defusr);
        		Settings.Secure.putString(mContext.getContentResolver(), Settings.Secure.PPPOE_PSWD,defpwd);
        		Settings.Secure.putInt(mContext.getContentResolver(),Settings.Secure.PPPOE_ON,1);
        	}
        }
    }

    public void start() {
        Log.i(TAG, "Starting Pppoe service");
    }

    public int getPppoeState() {
        LOG("getPppoeState = " + mPppoeState);
        return mPppoeState;
    }

    public String dumpCurrentState(int state) {
        if (state == PppoeManager.PPPOE_STATE_CONNECT) {
            return "PPPOE_STATE_CONNECT";
        } else if (state == PppoeManager.PPPOE_STATE_CONNECTING) {
            return "PPPOE_STATE_CONNECTING";
        } else if (state == PppoeManager.PPPOE_STATE_DISCONNECT) {
            return "PPPOE_STATE_DISCONNECT";
        } else if (state == PppoeManager.PPPOE_STATE_DISCONNECTING) {
            return "PPPOE_STATE_DISCONNECTING";
        } else {
            return "PPPOE_STATE_UNKNOWN";
        }
    }
    
    private void setPppoeStateAndSendBroadcast(int newState) {
        String failedCodes = "unknown";
        int preState = mPppoeState;
        mPppoeState = newState;
        if (mPppoeState == -1)
           mPppoeState = PppoeManager.PPPOE_STATE_DISCONNECT;

        LOG("setPppoeStateAndSendBroadcast newState = " + dumpCurrentState(newState));

        Intent intent = new Intent(PppoeManager.PPPOE_STATE_CHANGED_ACTION);
        intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);

if (FOR_CMCC_PLATFORM) {
	if (newState == PppoeManager.PPPOE_STATE_CONNECT){
            newState = PppoeManager.EVENT_CONNECT_SUCESSED;
        } else if (newState == PppoeManager.PPPOE_STATE_DISCONNECT) {
            newState = PppoeManager.EVENT_CONNECT_FAILED; //don't broadCast
            return ;
        }
}

        if (newState == -1) {
            LOG("newstate= -1 ,     "+newState);
if (FOR_CMCC_PLATFORM) {
            newState = PppoeManager.EVENT_CONNECT_FAILED;
} else {
            //newState = PppoeManager.PPPOE_STATE_DISCONNECT;
}
            String errors = SystemProperties.get("net.pppoe.error.codes");
            if (errors.contains("authentication failed")) {
                failedCodes = PppoeManager.PPPOE_ERRMSG_AUTH_FAIL;
            } else if (!isCarrierOn()) {
                failedCodes = PppoeManager.PPPOE_ERRMSG_NO_CARRIER;
            } else {
                failedCodes = PppoeManager.PPPOE_ERRMSG_UNKNOWN;
            }
if (FOR_CMCC_PLATFORM) {
            failedCodes = null;
}
            intent.putExtra(PppoeManager.EXTRA_PPPOE_ERRMSG,failedCodes);
        }

        intent.putExtra(PppoeManager.EXTRA_PPPOE_STATE, newState);
        intent.putExtra(PppoeManager.EXTRA_PREVIOUS_PPPOE_STATE, preState);
        LOG("setPppoeStateAndSendBroadcast() : preState = " + dumpCurrentState(preState) +", curState = " + dumpCurrentState(newState));
        mContext.sendBroadcastAsUser(intent, UserHandle.ALL);
    }

    private boolean isCarrierOn() {
        return true;
    }

    Handler handler = new Handler();
    Runnable runnable = new Runnable() {
        @Override
        public void run() {
          // TODO Auto-generated method stub
              startPppoe();
        }
    };

    private int tryCount=PPPOE_RETRY_COUNT;

    public boolean startPppoe() {
        LOG("startPppoe");
        if(mSameUserPwd) {
            LOG("We have already connect to account " + mUser + ", skip connect angain.");
            return true;
        }

        setPppoeEnable(1);
        if (!isCarrierOn()) {
            //setPppoeStateAndSendBroadcast(-1);
            return true;
        }

        setPppoeStateAndSendBroadcast(PppoeManager.PPPOE_STATE_CONNECTING);

        IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
        INetworkManagementService NMService = INetworkManagementService.Stub.asInterface(b);
        try {

            /*int resetMask = NetworkUtils.RESET_ALL_ADDRESSES;
            if (!TextUtils.isEmpty(mIface)) {
                 Log.d(TAG,"resetConnections on interface "+mIface);
                 NetworkUtils.resetConnections(mIface,resetMask);
            }*/

            NMService.clearInterfaceAddresses(mIface);
        } catch (Exception e) {
            Log.e(TAG, "Failed to clear addresses" + e);
        }


        if ( 0 != startPppoeNative() ) {
            if(tryCount > 0) {
                LOG("fail to start pppoe! we will try "+tryCount+" times");
                tryCount--;
                stopPppoeNative();
                handler.postDelayed(runnable, 2000);
            }else {
                LOG("startPppoe() : fail to start pppoe!");
                machineStopPppoe();
                setPppoeStateAndSendBroadcast(-1);
            }
            return false;
        } else {
            tryCount=PPPOE_RETRY_COUNT;
            setPppoeStateAndSendBroadcast(PppoeManager.PPPOE_STATE_CONNECT);
            return true;
        }
    }
    
    public boolean stopPppoe() {
    	LOG("stopPppoe");
	tryCount = 0;
    	if (mPppoeState != PppoeManager.PPPOE_STATE_CONNECT &&
    	    mPppoeState != PppoeManager.PPPOE_STATE_CONNECTING) {
            LOG("Already stoped");
            stopPppoeNative();
            return true;
    	}

        setPppoeStateAndSendBroadcast(PppoeManager.PPPOE_STATE_DISCONNECTING);
        if ( 0 !=   stopPppoeNative() ) {
            LOG("stopPppoe() : fail to stop pppoe!");
            setPppoeStateAndSendBroadcast(PppoeManager.PPPOE_STATE_DISCONNECT);
            setPppoeEnable(0);
            return false;
        } else {
            setPppoeStateAndSendBroadcast(PppoeManager.PPPOE_STATE_DISCONNECT);
            setPppoeEnable(0);
            return true;
        }
    }

    Runnable machineRunnable = new Runnable() {
        @Override
        public void run() {
          // TODO Auto-generated method stub
              machineStartPppoe();
        }
    };

    public boolean machineStartPppoe() {
        LOG("machineStartPppoe");
        if(mSameUserPwd) {
            LOG("We have already connect to account " + mUser + ", skip connect angain.");
            return true;
        }
        if (!isCarrierOn()) {
            return true;
        }

        IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
        INetworkManagementService NMService = INetworkManagementService.Stub.asInterface(b);
        try {
            NMService.clearInterfaceAddresses(mIface);
        } catch (Exception e) {
            Log.e(TAG, "Failed to clear addresses" + e);
        }

        setPppoeStateAndSendBroadcast(PppoeManager.PPPOE_STATE_CONNECTING);
        if ( 0 != startPppoeNative() ) {

            if(tryCount > 0) {
                LOG("fail to start pppoe! we will try "+tryCount+" times");
                tryCount--;
                stopPppoeNative();
                handler.postDelayed(machineRunnable, 2000);
            }else {
                LOG("startPppoe() : fail to start pppoe!");
                machineStopPppoe();
                setPppoeStateAndSendBroadcast(-1);
            }
            return false;
        } else {
            tryCount=PPPOE_RETRY_COUNT;
            new Handler().postDelayed((new Runnable() {
                 @Override
                 public void run() {
                       setPppoeStateAndSendBroadcast(PppoeManager.PPPOE_STATE_CONNECT);
                 } }),1000);

            return true;
        }
    }


    public boolean machineStopPppoe() {
        LOG("machineStopPppoe");
        tryCount = 0;
        if (mPppoeState != PppoeManager.PPPOE_STATE_CONNECT &&
            mPppoeState != PppoeManager.PPPOE_STATE_CONNECTING) {
            stopPppoeNative();
            LOG("Already stoped");
            return true;
        }
        setPppoeStateAndSendBroadcast(PppoeManager.PPPOE_STATE_DISCONNECTING);
        if ( 0 != stopPppoeNative() ) {
            LOG("stopPppoe() : fail to stop pppoe!");
            setPppoeStateAndSendBroadcast(PppoeManager.PPPOE_STATE_DISCONNECT);
            return false;
        } else {
            setPppoeStateAndSendBroadcast(PppoeManager.PPPOE_STATE_DISCONNECT);
            return true;
        }
    } 
    public boolean setupPppoe(String user, String iface, String dns1, String dns2, String password) {
        int ret;
        
        LOG("setupPppoe: " + "user=" + user + ", iface=" + iface + ", dns1=" + dns1 + ", dns2=" + dns2);

        if (user==null || iface==null || password==null) return false;
        if (dns1==null) dns1="";
        if (dns2==null) dns2="";

        mUser     = getPppoeUserName();
        mPassword = getPppoePassword();
        /*if(mPppoeState == PppoeManager.PPPOE_STATE_CONNECT &&
           mUser.equals(user) && mPassword.equals(password) && mIface.equals(iface)) {
            LOG("We have already connect to account " + user + ", skip connect angain.");
            mSameUserPwd = true;
            return true;
        }*/
        mSameUserPwd = false;
        tryCount=PPPOE_RETRY_COUNT;
        
        LOG("mPppoeState = " + mPppoeState);
        if(mPppoeState == PppoeManager.PPPOE_STATE_CONNECT ||
           mPppoeState == PppoeManager.PPPOE_STATE_CONNECTING) {
            LOG("pppoe is connecting or connected ot account " + mUser +", we disconnet it first.");
            if (iface.equals("eth0") && mIface.equals("wlan0")) 
                mWiFiEthernetSwitchMode = 2;
            else if (iface.equals("wlan0") && mIface.equals("eth0"))
                mWiFiEthernetSwitchMode = 1;
            else 
                mWiFiEthernetSwitchMode = 0;
            machineStopPppoe();
        }

        mIface = iface;
        setPppoeUserName(user);
        setPppoePassword(password);
        setPppoePhyIface(iface);
        
        if (!isCarrierOn()) {
            return true;
        }

        if (0 == setupPppoeNative(user, iface, dns1, dns2, password)) {
            return true;
        } else {
            return false;
        }
    }

    public String getPppoePhyIface() {
        //LOG("getPppoePhyIface = " + mIface);
        return mIface;	
    }

    public boolean setPppoePhyIface(String iface) {
        if (iface.equals("ethernet") || iface.equals("eth0")) {
            mIface = mEthIface;
        } else {
            mIface = mWifiIface;
        }
        Settings.Secure.putString(mContext.getContentResolver(), Settings.Secure.PPPOE_PHY_IFACE, mIface);
        LOG("setPppoePhyIface = " + mIface);
        return true;
    }

    public void setPppoeEnable(int enable) {
        Settings.Secure.putInt(mContext.getContentResolver(), Settings.Secure.PPPOE_ON, enable); 
    }

    public void setPppoeUserName(String user) {
        Settings.Secure.putString(mContext.getContentResolver(), Settings.Secure.PPPOE_USERNAME, user);
    }

    public void setPppoePassword(String pwd) {
        Settings.Secure.putString(mContext.getContentResolver(), Settings.Secure.PPPOE_PSWD, pwd);
    }

    public String getPppoeUserName() {
        return Settings.Secure.getString(mContext.getContentResolver(),Settings.Secure.PPPOE_USERNAME);
    }

    public String getPppoePassword() {
        return Settings.Secure.getString(mContext.getContentResolver(),Settings.Secure.PPPOE_PSWD);
    }

    public int getPppoeWiFiEthernetSwitchMode() {
        return mWiFiEthernetSwitchMode;
    }

    public void setPppoeWiFiEthernetSwitchMode(int mode) {
        mWiFiEthernetSwitchMode = mode;
    }

    public int isPppoeEnable() {
        int on = Settings.Secure.getInt(mContext.getContentResolver(), Settings.Secure.PPPOE_ON, 0);
        LOG("isPppoeEnable = " + on);
        return on;
    }

    public boolean disablePppoe(String iface) {
        LOG("disablePppoe: iface = " + iface + ", phyIface = " + mIface);
        if (iface.equals(mIface)) {
            setPppoeEnable(0);
            return true;
        }
        return false;
    }

    private void getDns(String[] dnses) {
      int tryCount = 10;
      boolean needSleep = false;
      int sleepTime = 200; // ms
      for(int n = 0; n < tryCount; n++) {
        if (needSleep) {
            try {
                Thread.sleep(sleepTime);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        try {
            File file = new File ("/data/misc/ppp/resolv.conf");
            FileInputStream fis = new FileInputStream(file);
            InputStreamReader input = new InputStreamReader(fis);
            BufferedReader br =  new BufferedReader(input, 128);
            String str;
            int i = 0;

            Log.d(TAG, "getDns");
            while((str=br.readLine()) != null) {
                String dns = str.substring(11);
                Log.d(TAG, "dns"+i+":="+dns);
//              mLinkProperties.addDnsServer(NetworkUtils.numericToInetAddress(dns));
                dnses[i] = dns;
                i++;
            }
            br.close();
        } catch (FileNotFoundException e) {
            Log.e(TAG, "resolv.conf not found, wait and retry");
            needSleep = true;
            continue;
        } catch (IOException e) {
            Log.e(TAG, "handle resolv.conf failed");
        }
        if (n >= tryCount)
            Log.e(TAG, "wait resolv.conf timeout");
        break;
      }
    }

    private LinkAddress makeLinkAddress() {
        int prefix = 24;
        String netmask = SystemProperties.get("net."+mPppIface+".mask");
        String ipaddr = SystemProperties.get("net."+mPppIface+".local-ip");
        if (TextUtils.isEmpty(ipaddr)) {
            Log.e(TAG, "makeLinkAddress: with empty ipAddress");
            return null;
        }		
        try {
            Inet4Address mask = (Inet4Address)NetworkUtils.numericToInetAddress(netmask);
            prefix = NetworkUtils.netmaskToPrefixLength(mask);
        } catch (Exception e) {
            Log.e(TAG, "makeLinkAddress: netmask to prefix exception: " + e);
            Log.d(TAG, "set default prefix 24");
        }
        return new LinkAddress(NetworkUtils.numericToInetAddress(ipaddr), prefix);
    }

    public LinkProperties getLinkProperties() {
        LinkProperties p = new LinkProperties();
        p.addLinkAddress(makeLinkAddress());

        String gateway = SystemProperties.get("net."+mPppIface+".gw");
        p.addRoute(new RouteInfo(NetworkUtils.numericToInetAddress(gateway)));

        String[] dnses = new String[2];
        getDns(dnses);
        if (TextUtils.isEmpty(dnses[0]) == false) {
            p.addDnsServer(NetworkUtils.numericToInetAddress(dnses[0]));
        } else {
            Log.d(TAG, "makeLinkProperties with empty dns1!");
        }
        if (TextUtils.isEmpty(dnses[1]) == false) {
            p.addDnsServer(NetworkUtils.numericToInetAddress(dnses[1]));
        } else {
            Log.d(TAG, "makeLinkProperties with empty dns2!");
        }
        return p;
    }

    public String getPppIfaceName() {
        return mPppIface;
    }

    private void doPppoeDisconnectCheck() {
        String errors;
        errors = SystemProperties.get("net.pppoe.error.codes");
        if (mPppoeState == PppoeManager.PPPOE_STATE_CONNECTING &&
            errors.contains("authentication failed")) {
            LOG("pppoe failed: " + errors + ", stop pppoe process.");
            new Thread(new Runnable() {
               public void run() {
                  stopPppoeNative();
               }
            }).start();

            if (errors.contains("You are already log in")) {
                LOG("we need to retry one more"); 
                tryCount = 1;
            }
        }
    }
    	
    private class PppoeObserver extends UEventObserver {
        private static final String PPPOE_UEVENT_MATCH = "SUBSYSTEM=net";
        
        private Context mContext;
        
        public PppoeObserver(Context context) {
            mContext = context;
            LOG("PppoeObserver() : to start observing, to catch uevent with '" + PPPOE_UEVENT_MATCH + "'.");
            startObserving(PPPOE_UEVENT_MATCH);
            init();
        }

        private synchronized final void init() {
        }

        @Override
        public void onUEvent(PppoeObserver.UEvent event) {
            LOG("onUEvent() : get uevent : '" + event + "'.");

            String netInterface = event.get("INTERFACE");
            String action = event.get("ACTION");            
            if ( null != netInterface && netInterface.equals(mPppIface) ) {
                if ( action.equals("add") ) {
//                    setEthernetEnabled(true);
                }
                else if ( action.equals("remove") ) {
                        doPppoeDisconnectCheck();
//                    setEthernetEnabled(false);
                }
           }
        }
    }
    
    public native static int setupPppoeNative(String user, String iface,String dns1, String dns2, String password);
    public native static int startPppoeNative();
    public native static int stopPppoeNative();
    public native static int isPppoeUpNative();
    public native static int registerNatives();	
}

