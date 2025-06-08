#include "../Device/Device.h"


class NewClient : public Device {
private:
    std::string routerIP = "173.173.173.1";
    std::string routerMAC;
    sockaddr_in routerOSAddress;
    const unsigned int routerOSPort = 6000;

    std::string DNSServerIP;

    std::string fileName;

    enum State {
        START = 1,
        DHCP_ACK,
        ARP_RESPONSE
    };

    State state = START;
    std::mutex state_mtx;
    std::condition_variable state_cv;


    // SEND FUNCTIONS:    
    // internet layer
    void sendICMPMessage(std::string op, std::string destIP);
    void sendIPMessage(std::string msg, std::string destIP, std::string upp_type);
    void sendARPRequest();
    void sendDHCPDiscover();
    void sendDHCPRequest(std::string receivedIP);
    void sendUDPMessage(std::string msg, int destPort, std::string upp_type, std::string destIP);
    void sendEthernetMessage(std::string msg, std::string destMAC, std::string upp_type);

    void sendHTTPRequest(std::string destIP);
    void sendHTTPResponse(std::string destIP, std::string html_page);

    void handleHTTPHeader(std::string recv_msg, std::string recvIP);
    void handleUDPHeader(std::string recv_msg, std::string recvIP);


    // Handle received message functions
    void handleEthernetHeader(std::string recv_msg);
    void handleARPResponse(std::string recv_msg);
    void handleIPHeader(std::string recv_msg);
    void handleICMPHeader(std::string recv_msg, std::string recvIP);
    void handleDHCPHeader(std::string recv_msg);
    void handleUDPHeader(std::string recv_msg);
    void handleDNSHeader(std::string recv_msg);

protected:
    void handleMsg(std::string msg, sockaddr_in senderOSAddress);

public:
    NewClient(std::string IP, std::string MAC, unsigned int OS_port, std::string filename);
    NewClient(std::string MAC, unsigned int OS_port, std::string filename) : NewClient("", MAC, OS_port, filename) {}


    void sendPing(std::string destIP);
    void sendDNSRequest(std::string domain_name);
    void sendDNSRegister(std::string domain_name);

    void start();
};