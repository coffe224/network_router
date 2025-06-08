#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <string>
#include <vector>

#include "ARPTable.h"
#include "../Device/Device.h"

class NewRouter : public Device {
private:
    std::string DHCPServerIP = "173.173.12.0";
    std::string DHCPServerMAC = "16:34:16:34:aa:aa";
    int DHCPServerOSPort = 6005;
    sockaddr_in DHSPServerOSAddress;

    std::string DNSServerIP = "173.160.25.0";
    std::string DNSServerMac = "ab:ab:ab:ab:ab:ab";
    int DNSServerOSPort = 6006;
    sockaddr_in DNSServerOSAddress;

    ARPTable arpTable;

    std::vector<sockaddr_in> broadcastAddresses;

    void handleEthernetHeader(std::string recv_msg, sockaddr_in clientAddr);
    void handleARPRequest(std::string recv_msg, sockaddr_in clientAddr);
    void handleIPHeader(std::string recv_msg);

    void sendARPResponse(std::string destIP, std::string destMAC);
    void sendEthernetMessage(std::string msg, std::string destMAC, std::string upp_type);

    void sendBroadcast(std::string msg, std::string upp_type);

protected:
    void handleMsg(std::string msg, sockaddr_in senderOSAddress);

public:
    NewRouter(std::string IP, std::string MAC, unsigned int OS_port);
    void start();



    // NAT
    // Добавить порты
    // проверку локальный порт или нет
    // упрощённый вариант ? IP -> port
    // NAT table
    // void resolve();
    // Router(unsigned int a);
};