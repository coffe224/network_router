#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string>
#include <vector>
#include <atomic>
#include <thread>


class Client {
    private:

    std::thread receiverThread;
    std::atomic<bool> canSendPing = false;
    std::atomic<bool> startedReceiving = false;

    unsigned int clientSocket;
    sockaddr_in clientAddress;
    
    sockaddr_in routerAddress;
    

    std::vector<std::string> parseHeader(std::string header);

    public:
    
    const std::string routerIP = "192.213.234.11";
    const unsigned int routerPort = 6000;
    std::string routerMAC;
    
    std::string ownIP;
    std::string ownMAC;

    Client(std::string client_ip, std::string client_mac, unsigned int client_port);
    void start();


    // SEND FUNCTIONS:    
    // internet layer
    void sendICMPMessage(std::string op, std::string destIP);
    void sendIPMessage(std::string msg, std::string destIP, std::string upp_type);
    void sendARPRequest();

    // link layer
    void sendEthernetMessage(std::string msg, std::string destMAC, std::string upp_type);
    
    // типа отправили
    void sendMessage(std::string msg, sockaddr_in destAddress);
    

    // RECEIVE AND HANDLE FUNCTIONS:
    // типа слушаем
    void recvMessage();
    void handleEthernetHeader(std::string recv_msg);
    void handleARPResponse(std::string recv_msg);
    void handleIPHeader(std::string recv_msg);
    void handleICMPHeader(std::string recv_msg, std::string recvIP);
};