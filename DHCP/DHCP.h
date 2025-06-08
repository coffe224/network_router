#include "../Device/Device.h"
#include "DHCPTable.h"

class DHCPServer : public Device {
private:
    std::string routerIP = "173.173.173.1";
    std::string routerMAC = "00:00:00:00:aa:01";
    sockaddr_in routerOSAddress;
    const unsigned int routerOSPort = 6000;

    std::string dnsServerIP = "173.160.25.0";

    DHCPTable table;

public:
    DHCPServer(std::string IP, std::string MAC, unsigned int OS_port);
    void start();
        
private:
    void sendIPMessage(std::string msg, std::string destIP, std::string upp_type);
    void sendEthernetMessage(std::string msg, std::string destMAC, std::string upp_type);
    
    void handleEthernetHeader(std::string recv_msg);
    void handleIPHeader(std::string recv_msg);
    void handleDHCPHeader(std::string recv_msg);
    
    void sendDHCPOffer(std::string clientMAC, std::string offeredIP);
    void sendDHCPAcknowledge(std::string clientMAC, std::string leasedIP);
    void handleMsg(std::string msg, sockaddr_in senderOSAddress);
};