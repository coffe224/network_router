#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "../Device/Device.h"

class DNSServer : public Device {
private:
    std::string routerIP = "173.173.173.1";
    std::string routerMAC = "00:00:00:00:aa:01";
    sockaddr_in routerOSAddress;
    const unsigned int routerOSPort = 6000;

    std::unordered_map<std::string, std::string> dnsRecords;

    void addRecord(std::string domain_name, std::string ip);

    bool containsRecord(std::string domain_name);

    std::string getRecord(std::string domain_name);

public:
    DNSServer(std::string IP, std::string MAC, unsigned int OS_port);
    void start();

private:
    void handleDNSHeader(std::string recv_msg, std::string destIP);
    void handleUDPHeader(std::string recv_msg, std::string destIP);
    void handleIPHeader(std::string recv_msg);
    void handleEthernetHeader(std::string recv_msg);

    void handleMsg(std::string msg, sockaddr_in senderOSAddress);


    void sendDNSResponse(std::string domain_name, std::string registeredIP, std::string destIP);

    void sendUDPMessage(std::string msg, int destPort, std::string upp_type, std::string destIP);
    void sendIPMessage(std::string msg, std::string destIP, std::string upp_type);
    void sendEthernetMessage(std::string msg, std::string destMAC, std::string upp_type);

};