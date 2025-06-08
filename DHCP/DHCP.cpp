#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <cstdint>

#include "DHCP.h"

DHCPServer::DHCPServer(std::string IP, std::string MAC, unsigned int OS_port) : 
Device(IP, MAC, OS_port)
{
    routerOSAddress.sin_family = AF_INET;
    routerOSAddress.sin_port = htons(routerOSPort);
    routerOSAddress.sin_addr.s_addr = INADDR_ANY;

    std::cout << "-------- DHCP SERVER ---------\n";

}

void DHCPServer::sendDHCPOffer(std::string clientMAC, std::string offeredIP)
{
    std::string message = "OFFER;" + clientMAC + ";" + offeredIP;
    sendIPMessage(message, "255.255.255.255", "DHCP");
}

void DHCPServer::sendDHCPAcknowledge(std::string clientMAC, std::string leasedIP)
{
    std::string message = "ACKNOWLEDGE;" + clientMAC + ";" + leasedIP + ";" + dnsServerIP;
    sendIPMessage(message, "255.255.255.255", "DHCP");
}

void DHCPServer::sendIPMessage(std::string msg, std::string destIP, std::string upp_type)
{
    std::string message = ownIP + ";" + destIP + ";" + upp_type + "|" + msg;

    sendEthernetMessage(message, routerMAC, "IP");
}

void DHCPServer::sendEthernetMessage(std::string msg, std::string destMAC, std::string upp_type)
{
    if (destMAC == "") {
        std::cout << "Can't send message, empty destination MAC...\n";
        return;
    }
    std::string message = destMAC + ";" + ownMAC + ";" + upp_type + "|" + msg;
    sendMessage(message, routerOSAddress);
}

void DHCPServer::handleMsg(std::string msg, sockaddr_in senderOSAddress)
{
    std::cout << "Received msg\n";
    handleEthernetHeader(msg);
}

void DHCPServer::handleEthernetHeader(std::string recv_msg)
{
    int end_ethernet_header = recv_msg.find_first_of("|");
    std::string ethernet_header = recv_msg.substr(0, end_ethernet_header);
    std::string data = recv_msg.substr(end_ethernet_header + 1, std::string::npos);

    std::vector<std::string> parsed_header = parseHeader(ethernet_header);
    if (parsed_header[2] == "IP") {
        handleIPHeader(data);
    }
}

void DHCPServer::handleIPHeader(std::string recv_msg)
{
    int end_ip_header = recv_msg.find_first_of("|");
    std::string ip_header = recv_msg.substr(0, end_ip_header);
    std::string data = recv_msg.substr(end_ip_header + 1, std::string::npos);

    std::vector<std::string> parsed_header = parseHeader(ip_header);

    std::string senderIP = parsed_header[0];
    std::string recvIP = parsed_header[1];
    std::string op = parsed_header[2];


    if (recvIP == "255.255.255.255" && senderIP == "0.0.0.0" && op == "DHCP") {
        handleDHCPHeader(data);
    }
}

void DHCPServer::handleDHCPHeader(std::string recv_msg)
{
    std::vector<std::string> parsed_header = parseHeader(recv_msg);

    std::string operation = parsed_header[0];
    std::string receivedMAC = parsed_header[1];

    if (operation == "DISCOVER") {
        std::cout << "RECEIVED DISCOVER\n";
        std::string offeredIP = table.getNextAvailableIP();
        table.add(offeredIP, receivedMAC);
        sendDHCPOffer(receivedMAC, offeredIP);
    } else if (operation == "REQUEST") {
        std::cout << "RECEIVED REQUEST\n";
        std::string receivedIP = parsed_header[2];
        if (table.contains(receivedIP, receivedMAC))
        sendDHCPAcknowledge(receivedMAC, receivedIP);
    }
}

void DHCPServer::start() 
{
    Device::start();
}


int main() 
{
    std::string DHCPServerIP = "173.173.12.0";
    std::string DHCPServerMAC = "16:34:16:34:aa:aa";
    int DHCPServerPort = 6005;

    DHCPServer dhcp_server(DHCPServerIP, DHCPServerMAC, DHCPServerPort);
    dhcp_server.start();
    std::this_thread::sleep_for(std::chrono::seconds(600));
}