/*  
 *  Copyright(C), 2009-2010, Fuzhou Rockchip Co. ,Ltd.  All Rights Reserved.
 *
 *  File:   PppoeManager.java
 *  Desc:   
 *  Usage:
 *  Note:
 *  Author: cz@rock-chips.com
 *  Version:
 *          v1.0
 *  Log:
    ----Thu Sep 8 2011            v1.0
 */
package android.net;

import android.content.Context;
import android.annotation.SdkConstant;
import android.annotation.SdkConstant.SdkConstantType;
import android.net.DhcpInfo;
import android.net.LinkProperties;
import android.os.Binder;
import android.os.IBinder;
import android.os.Handler;
import android.os.Message;
import android.os.Looper;
import android.os.HandlerThread;
import android.os.RemoteException;
import android.util.Log;

import java.util.List;

//$_rbox_$_modify_$_by huangjc,add for chinamobile
import java.net.InetAddress;
import java.net.Inet4Address;
import android.os.SystemProperties;
import android.net.NetworkUtils;
//$_rbox_$_modify_$_end

public class PppoeManager {
    private static final String TAG = "PppoeManager";
    public static final boolean DEBUG = true;
    private static void LOG(String msg) {
        if ( DEBUG ) {
            Log.d(TAG, msg);
        }
    }
    
    /**
     *  Broadcast intent action 
     *      indicating that PPPOE has been enabled, disabled, enabling, disabling, or unknown. 
     *  One extra provides current state as an int.
     *  Another extra provides the previous state, if available.
     * 
     * @see #EXTRA_PPPOE_STATE
     * @see #EXTRA_PREVIOUS_PPPOE_STATE
     */
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String PPPOE_STATE_CHANGED_ACTION 
        = "android.net.pppoe.PPPOE_STATE_CHANGED";
    /**
     *  Retrieve it with {@link android.content.Intent#getIntExtra(String,int)}.
     * 
     * @see #PPPOE_STATE_DISABLED
     * @see #PPPOE_STATE_DISABLING
     * @see #PPPOE_STATE_INIT_ENABLED
     * @see #PPPOE_STATE_ENABLING
     * @see #PPPOE_STATE_UNKNOWN
     */
    public static final String EXTRA_PPPOE_STATE = "pppoe_state";
    /**
     * The previous PPPOE state.
     * 
     * @see #EXTRA_PPPOE_STATE
     */
    public static final String EXTRA_PREVIOUS_PPPOE_STATE = "previous_pppoe_state";
    /**
     * The  PPPOE error message.
     * 
     * @see #EXTRA_PPPOE_STATE
     */
    public static final String EXTRA_PPPOE_ERRMSG = "pppoe_errmsg";
	//$_rbox_$_modify_$_by cx,add for chinamobile
    public static final int PPPOE_STATE_UNKNOWN = 0;
    public static final int PPPOE_STATE_CONNECT = 1;
    public static final int PPPOE_STATE_DISCONNECT = 2;
    public static final int PPPOE_STATE_CONNECTING = 3;
    public static final int PPPOE_STATE_DISCONNECTING = 4;

    public static final int EVENT_CONNECT_SUCESSED = 0;
    public static final int EVENT_CONNECT_FAILED = 1;

    /**
     * PPPOE ERROR CODE
     */
    public static final String PPPOE_ERRMSG_AUTH_FAIL = "PPPOE Error: Authentication Failed";
    public static final String PPPOE_ERRMSG_NO_CARRIER= "PPPOE Error: No Carrier";
    public static final String PPPOE_ERRMSG_UNKNOWN   = "PPPOE Error: Unkown";

    /**
     *the pppoe interface is configured by dhcp
     */
    public static final String PPPOE_CONNECT_MODE_DHCP = "dhcp";
    /**
        *the pppoe interface is configured manually
    */
    public static final String PPPOE_CONNECT_MODE_MANUAL = "manual";
//$_rbox_$_modify_$_end
    private IPppoeManager mService;
    private Context mContext;
    private PppoeHandler mPppoeHandler;

    public PppoeManager(Context context, IPppoeManager service) {
        mService = service;
        mContext = context;
        /*if ( null == mPppoeHandler ) {
            LOG("PppoeManager() : start 'pppoe handle thread'.");
            HandlerThread handleThread = new HandlerThread("Pppoe Handler Thread");
            handleThread.start();
            mPppoeHandler = new PppoeHandler(handleThread.getLooper());
        }*/
    }

    private class PppoeHandler extends Handler {
        private static final int COMMAND_START_PPPOE = 1;
        private static final int COMMAND_STOP_PPPOE = 2;

        private Handler mTarget;
        
        public PppoeHandler(Looper looper) {
            super(looper);
        }

        public void handleMessage(Message msg) {       
            
            int event;
            LOG("PppoeHandler::handleMessage() : Entered : msg = " + msg);

            switch (msg.what) {
                case COMMAND_START_PPPOE:
                    try {
                        mService.startPppoe();
                    } catch (RemoteException e) {
                        Log.e(TAG, "startPppoe failed");
                    }
                    break;
                case COMMAND_STOP_PPPOE:
                    try {
                        mService.stopPppoe();
                    } catch (RemoteException e) {
                        Log.e(TAG, "stopPppoe failed");
                    }
                    break;
                default:
                    break;
            }
        }
    }

    public int getPppoeState() {
        try {
            return mService.getPppoeState();
        } catch (RemoteException e) {
            Log.e(TAG, "stopPppoe failed");
            return -1;
        }
    }
    /**
     * {@hide}
     */
    public boolean machineStopPppoe(){
        try {
        return   mService.machineStopPppoe();
        } catch (RemoteException e) {
            Log.e(TAG, "stopPppoe failed");
            return false;
        }
    }
    public boolean startPppoe() {
        //return mPppoeHandler.sendEmptyMessage(PppoeHandler.COMMAND_START_PPPOE);
        try {
            return mService.startPppoe();
        } catch (RemoteException e) {
            return false;
        }
    }

    public boolean stopPppoe() {
        //return mPppoeHandler.sendEmptyMessage(PppoeHandler.COMMAND_STOP_PPPOE);
        try {
            return mService.stopPppoe();
        } catch (RemoteException e) {
            return false;
        }      
    }

    public boolean setupPppoe(String user, String iface, String dns1, String dns2, String password) {
        try {
            return mService.setupPppoe(user, iface, dns1, dns2, password);
        } catch (RemoteException e) {
            return false;
        }
    }

    public String getPppoePhyIface() {
        try {
            return mService.getPppoePhyIface();
        } catch (RemoteException e) {
            return null;
        }
    }
 
    public boolean setPppoePhyIface(String iface) {
        try {
            return mService.setPppoePhyIface(iface);
        } catch (RemoteException e) {
            return false;
        }
    }

    public String getPppoeUserName() {
        try {
            return mService.getPppoeUserName();
        } catch (RemoteException e) {
            return null;
        }
    }

    public String getPppoePassword() {
        try {
            return mService.getPppoePassword();
        } catch (RemoteException e) {
            return null;
        }
    }

    public void setPppoeUserName(String user) {
        try {
            mService.setPppoeUserName(user);
        } catch (RemoteException e) {
        }
    }

    public void setPppoePassword(String pwd) {
        try {
            mService.setPppoePassword(pwd);
        } catch (RemoteException e) {
        }
    }

    public int getPppoeWiFiEthernetSwitchMode() {
        try {
            return mService.getPppoeWiFiEthernetSwitchMode();
        } catch (RemoteException e) {
            return 0;
        }
    }

    public void setPppoeWiFiEthernetSwitchMode(int mode) {
        try {
            mService.setPppoeWiFiEthernetSwitchMode(mode);
        } catch (RemoteException e) {
        }
    }

    public int isPppoeEnable() {
        try {
            return mService.isPppoeEnable();
        } catch (RemoteException e) {
            return 0;
        }
    }

    public void setPppoeEnable(int enable) {
        try {
            mService.setPppoeEnable(enable);
        } catch (RemoteException e) {
        }
    }

    public boolean disablePppoe(String iface) {
        try {
            return mService.disablePppoe(iface);
        } catch (RemoteException e) {
            return false;
        }
    }
 
  //$_rbox_$_modify_$_by cx,add for chinamobile
	private String mMode = PPPOE_CONNECT_MODE_DHCP;
	/**
	**/
	public synchronized void connect(String name, String pwd, String iface) {
		LOG("connect");
		//try{throw new Exception();}catch(Exception e){e.printStackTrace();}
		setupPppoe(name,iface,null,null,pwd);
		startPppoe();
	}

	/**
	**/
	public synchronized void disconnect(String iface) {
		LOG("disconnect");
		//try{throw new Exception();}catch(Exception e){e.printStackTrace();}
		stopPppoe();
	}

	/**
	**/
	public synchronized void setPppoeMode(String mode, DhcpInfo info) {
		LOG("setPppoeMode = " + mode);
	}

	/**
	**/
	public synchronized String getPppoeMode() {
		LOG("getPppoeMode");
		return PPPOE_CONNECT_MODE_DHCP;
	}

	public boolean isPppoeDeviceup() {
		LOG("isPppoeDeviceup");
		return true;
	}

	public DhcpInfo getDhcpInfo() {
		LOG("getDhcpInfo");
		DhcpInfo info = new DhcpInfo();
		/*String nullIpInfo = "0.0.0.0";
		int curState=getPppoeState();
		String tempIpInfo;
		tempIpInfo = SystemProperties.get("net.ppp0.local-ip");
		Log.d(TAG,"get Ip from properties = " + tempIpInfo);
		if ((tempIpInfo != null) && (!tempIpInfo.equals("")) && curState==PPPOE_STATE_CONNECT){ 
			info.ipAddress = convertToInt(tempIpInfo);
    	} else {  
    		info.ipAddress = convertToInt(nullIpInfo);
    	}
				
		tempIpInfo = SystemProperties.get("net.ppp0.mask");
		Log.d(TAG,"get netmask from properties = " + tempIpInfo);
		if ((tempIpInfo != null) && (!tempIpInfo.equals("")) && curState==PPPOE_STATE_CONNECT ){
       		info.netmask = convertToInt(tempIpInfo);
    	} else {           		
    		info.netmask = convertToInt(nullIpInfo);
    	}
					
		tempIpInfo = SystemProperties.get("net.ppp0.remote-ip");
		Log.d(TAG,"get gateway from properties = " + tempIpInfo);
		if ((tempIpInfo != null) && (!tempIpInfo.equals("")) && curState==PPPOE_STATE_CONNECT){
        	info.gateway = convertToInt(tempIpInfo);
    	} else {
    		info.gateway = convertToInt(nullIpInfo);    		
    	}

		tempIpInfo = SystemProperties.get("net.ppp0.dns1");
		Log.d(TAG,"get dns1 from properties = " + tempIpInfo);
		if ((tempIpInfo != null) && (!tempIpInfo.equals("")) && curState==PPPOE_STATE_CONNECT){
       		info.dns1 = convertToInt(tempIpInfo);
    	} else {
    		info.dns1 = convertToInt(nullIpInfo);      		
    	}

		tempIpInfo = SystemProperties.get("net.ppp0.dns2");
		Log.d(TAG,"get dns2 from properties = " + tempIpInfo);
		if ((tempIpInfo != null) && (!tempIpInfo.equals("")) && curState==PPPOE_STATE_CONNECT){
			info.dns2 = convertToInt(tempIpInfo);
    	} else {
    		info.dns2 = convertToInt(nullIpInfo);     		
    	}
                LOG("info = " + info);*/
		return info;
	}

    public LinkProperties getLinkProperties() {
        try {
            return mService.getLinkProperties();
        } catch (RemoteException e) {
            //e.printStackTrace();
        }
        return null;
     }

    public String dumpCurrentState(int state) {
        try {
            return mService.dumpCurrentState(state);
        } catch (RemoteException e) {
            //e.printStackTrace();
        }
        return null;
    }

    public String getPppIfaceName() {
        try {
            return mService.getPppIfaceName();
        } catch (RemoteException e) {
            //e.printStackTrace();
        }
        return null;
    }

	/*private int convertToInt(String addr) {
        if (addr != null) {
            try {
                InetAddress inetAddress = NetworkUtils.numericToInetAddress(addr);
                if (inetAddress instanceof Inet4Address) {
                    return NetworkUtils.inetAddressToInt(inetAddress);
                }
            } catch (IllegalArgumentException e) {
            	return 0;
            }
        }
        return 0;
    }*/

	private String intToString(int addr) {
		String nullIpInfo = "0.0.0.0";
		try {
			InetAddress inetAddr = NetworkUtils.intToInetAddress(addr);
			if (inetAddr != null) {
				return inetAddr.getHostAddress();
			}
		} catch (Exception e) {
			LOG("intToString error:" + e);
			return nullIpInfo;
		}
        return nullIpInfo;     
    }
	//$_rbox_$_modify_$_end
}

