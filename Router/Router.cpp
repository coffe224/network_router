#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <iostream>
#include <cstring>

#include <string>
#include <vector>
#include <sstream>

#include "Router.h"

Router::Router(unsigned int port)
{
    routerSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (routerSocket == -1) {
        std::cout << "No socket for you\n";
        throw;
    }
    std::cout << "Socket created\n";

    routerAddress.sin_family = AF_INET;
    routerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    routerAddress.sin_port = htons(port);

    int state = bind(routerSocket, (sockaddr*)&routerAddress, sizeof(routerAddress));
    if (state != 0) {
        std::cout << "No bind for you\n";
        throw;
    }
    
    std::cout << "Done binding\n";
}

std::vector<std::string> Router::parseHeader(std::string header)
{
    std::vector<std::string> parsed_header;

    std::stringstream ss(header);
    std::string token;

    while (std::getline(ss, token, ';')) {
        parsed_header.push_back(token);
    }

    return parsed_header;
}

void Router::recvMessage()
{
    std::cout << "\nListening to input...\n";

    char buffer[1024] = {0};
    sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    recvfrom(routerSocket, buffer, sizeof(buffer), 0, (sockaddr*)&clientAddr, &clientLen);

    std::cout << buffer << "\n";

    std::string msg(buffer);
    handleEthernetHeader(msg, clientAddr);
}

void Router::handleEthernetHeader(std::string recv_msg, sockaddr_in clientAddr) 
{
    int end_ethernet_header = recv_msg.find_first_of("|");
    std::string ethernet_header = recv_msg.substr(0, end_ethernet_header);
    std::string data = recv_msg.substr(end_ethernet_header + 1, std::string::npos);

    std::vector<std::string> parsed_header = parseHeader(ethernet_header);

    if (parsed_header[1] == ownMAC || parsed_header[1] == "ff:ff:ff:ff:ff") {
        if (parsed_header[2] == "ARP") {
            handleARPRequest(data, clientAddr);
        } else if (parsed_header[2] == "IP") {
            handleIPHeader(data);
        }
    }
}

void Router::handleARPRequest(std::string recv_msg, sockaddr_in clientAddr) 
{
    std::vector<std::string> parsed_header = parseHeader(recv_msg);

    if (parsed_header[0] == "REQ") {
        arpTable.add(parsed_header[1], parsed_header[3], clientAddr);
        std::cout << parsed_header[1] << "   " << parsed_header[3] << "\n";
        sendARPResponse(parsed_header[1], parsed_header[3]);
    }
}

void Router::handleIPHeader(std::string recv_msg) 
{
    int end_ip_header = recv_msg.find_first_of("|");
    std::string ip_header = recv_msg.substr(0, end_ip_header);

    std::vector<std::string> parsed_header = parseHeader(ip_header);
    
    std::string destIP = parsed_header[1];
    std::string destMAC = arpTable.findMAC(destIP);
    if (!destMAC.empty()) {
        sendEthernetMessage(recv_msg, destMAC, "IP");
    } else {
        std::cout << "IP is not registered\n";
    }
    
}

void Router::sendARPResponse(std::string destIP, std::string destMAC)
{
    std::cout << "Sending ARP response\n";
    std::string message = "RESP;" + ownIP + ";" + destIP + ";" + ownMAC;
    sendEthernetMessage(message, destMAC, "ARP");
}

void Router::sendEthernetMessage(std::string msg, std::string destMAC, std::string upp_type)
{
    std::string message = ownMAC + ";" + destMAC + ";" + upp_type + "|" + msg;

    sockaddr_in destAddress = arpTable.findAdress(destMAC);
    sendMessage(message, destAddress);    
}


void Router::sendMessage(std::string msg, sockaddr_in destAddress)
{
    std::cout << "\nSending message!\n";
    std::cout << msg << "\n";
    sendto(routerSocket, msg.data(), msg.length(), 0, (sockaddr*)&destAddress, sizeof(destAddress));
}

int main() {

    Router router(6000);
    while (1) {
        router.recvMessage();
    }

    return 0;
}