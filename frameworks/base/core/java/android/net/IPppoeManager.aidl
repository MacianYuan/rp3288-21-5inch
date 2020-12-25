
package android.net;

import android.net.LinkProperties;

interface IPppoeManager
{
    int getPppoeState();
    boolean setupPppoe(String user, String iface, String dns1, String dns2, String password);
    boolean startPppoe();
    boolean stopPppoe();
    String getPppoePhyIface();
    boolean setPppoePhyIface(String iface);
    String getPppoeUserName();
    String getPppoePassword();
    void setPppoeUserName(String user);
    void setPppoePassword(String pwd);
    int getPppoeWiFiEthernetSwitchMode();
    void setPppoeWiFiEthernetSwitchMode(int mode);
    int isPppoeEnable();
    void setPppoeEnable(int enable);
    boolean machineStopPppoe();
    boolean machineStartPppoe();
    boolean disablePppoe(String iface);
    LinkProperties getLinkProperties();
    String dumpCurrentState(int state);
    String getPppIfaceName();
}

