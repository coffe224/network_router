#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <string>
#include <vector>

#include "ARPTable.h"

class Router {
    private:

    sockaddr_in routerAddress;
    unsigned int routerSocket;

    ARPTable arpTable;

    std::vector<std::string> parseHeader(std::string header);

    public:

    const std::string ownIP = "192.213.234.11";
    std::string ownMAC = "aa:bb:cc:dd:ee:11";
    
    // RECEIVE AND HANDLE FUNCTIONS:
    void recvMessage();
    void handleEthernetHeader(std::string recv_msg, sockaddr_in clientAddr);
    void handleARPRequest(std::string recv_msg, sockaddr_in clientAddr);
    void handleIPHeader(std::string recv_msg);

    // SEND FUNCTIONS:    
    // internet layer
    void sendARPResponse(std::string destIP, std::string destMAC);

    // link layer
    void sendEthernetMessage(std::string msg, std::string destMAC, std::string upp_type);

    // типа отправили
    void sendMessage(std::string msg, sockaddr_in destAddress);


    // NAT

    // Добавить порты
    // проверку локальный порт или нет
    // упрощённый вариант ? IP -> port
    // NAT table
    void resolve();


    Router(unsigned int a);
};