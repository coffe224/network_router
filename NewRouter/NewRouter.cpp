#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <iostream>
#include <cstring>

#include <string>
#include <vector>
#include <sstream>

#include "NewRouter.h"


void NewRouter::handleMsg(std::string msg, sockaddr_in senderOSAddress)
{
    handleEthernetHeader(msg, senderOSAddress);
}

void NewRouter::start() {
    Device::start();
}

NewRouter::NewRouter(std::string IP, std::string MAC, unsigned int OS_port) : Device(IP, MAC, OS_port)
{
    DHSPServerOSAddress.sin_family = AF_INET;
    DHSPServerOSAddress.sin_port = htons(DHCPServerOSPort);
    DHSPServerOSAddress.sin_addr.s_addr = INADDR_ANY;

    DNSServerOSAddress.sin_family = AF_INET;
    DNSServerOSAddress.sin_port = htons(DNSServerOSPort);
    DNSServerOSAddress.sin_addr.s_addr = INADDR_ANY;

    std::cout << "-------- ROUTER ---------\n";

}

void NewRouter::handleEthernetHeader(std::string recv_msg, sockaddr_in clientAddr) 
{
    int end_ethernet_header = recv_msg.find_first_of("|");
    std::string ethernet_header = recv_msg.substr(0, end_ethernet_header);
    std::string data = recv_msg.substr(end_ethernet_header + 1, std::string::npos);

    bool flag = true;
    for (auto address : broadcastAddresses) {
        if (address.sin_port == clientAddr.sin_port) {
            flag = false;
        }
    }
    if (flag) {
        broadcastAddresses.push_back(clientAddr);
        std::cout << "Added to broadcast " << clientAddr.sin_port << "\n";
    }


    std::vector<std::string> parsed_header = parseHeader(ethernet_header);

    if (parsed_header[2] == "ARP") {
        handleARPRequest(data, clientAddr);
    } else if (parsed_header[2] == "IP") {
        handleIPHeader(data);
    }
}

void NewRouter::handleARPRequest(std::string recv_msg, sockaddr_in clientAddr) 
{
    std::vector<std::string> parsed_header = parseHeader(recv_msg);

    if (parsed_header[0] == "REQ") {
        arpTable.add(parsed_header[1], parsed_header[3], clientAddr);
        std::cout << parsed_header[1] << "   " << parsed_header[3] << "\n";
        sendARPResponse(parsed_header[1], parsed_header[3]);
    }
}

void NewRouter::handleIPHeader(std::string recv_msg) 
{
    int end_ip_header = recv_msg.find_first_of("|");
    std::string ip_header = recv_msg.substr(0, end_ip_header);

    std::vector<std::string> parsed_header = parseHeader(ip_header);
    
    std::string senderIP = parsed_header[0];
    std::string destIP = parsed_header[1];
    std::string op = parsed_header[2];

    if (destIP == "255.255.255.255") {
        if (senderIP == "0.0.0.0") {
            sendEthernetMessage(recv_msg, DHCPServerMAC, "IP");
        } else if (senderIP == DHCPServerIP) {
            sendBroadcast(recv_msg, "IP");
        }
        return;
    }

    if (destIP == DNSServerIP) {
        sendEthernetMessage(recv_msg, DNSServerMac, "IP");
        return;
    }
    std::string destMAC = arpTable.findMAC(destIP);
    if (!destMAC.empty()) {
        sendEthernetMessage(recv_msg, destMAC, "IP");
    } else {
        std::cout << "IP is not registered\n";
    }
}

void NewRouter::sendARPResponse(std::string destIP, std::string destMAC)
{
    std::cout << "Sending ARP response\n";
    std::string message = "RESP;" + ownIP + ";" + destIP + ";" + ownMAC;
    sendEthernetMessage(message, destMAC, "ARP");
}

void NewRouter::sendEthernetMessage(std::string msg, std::string destMAC, std::string upp_type)
{
    std::string message = destMAC + ";" + ownMAC + ";" + upp_type + "|" + msg;
    
    sockaddr_in destAddress; 
    if (destMAC == DHCPServerMAC) {
        destAddress = DHSPServerOSAddress;
    } else if (destMAC == DNSServerMac) {
        destAddress = DNSServerOSAddress;
    } else {
        destAddress = arpTable.findAdress(destMAC);
    }
    sendMessage(message, destAddress);    
}

void NewRouter::sendBroadcast(std::string msg, std::string upp_type)
{
    std::string message = "ff:ff:ff:ff:ff:ff;" + ownMAC + ";" + upp_type + "|" + msg;
    for (sockaddr_in address : broadcastAddresses) {
        sendMessage(message, address);
    }
}


int main() {
    std::string routerIP = "173.173.173.1";
    std::string routerMAC = "00:00:00:00:aa:01";

    NewRouter router(routerIP, routerMAC, 6000);
    router.start();
    std::this_thread::sleep_for(std::chrono::seconds(600));

    return 0;
}