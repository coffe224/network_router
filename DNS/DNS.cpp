#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <cstdint>

#include "DNS.h"

DNSServer::DNSServer(std::string IP, std::string MAC, unsigned int OS_port) : 
Device(IP, MAC, OS_port)
{
    routerOSAddress.sin_family = AF_INET;
    routerOSAddress.sin_port = htons(routerOSPort);
    routerOSAddress.sin_addr.s_addr = INADDR_ANY;

    std::cout << "-------- DNS SERVER ---------\n";
}

void DNSServer::sendDNSResponse(std::string domain_name, std::string registeredIP, std::string destIP)
{
    std::string message = "RESPONSE;" + domain_name + ";" + registeredIP;
    sendUDPMessage(message, 53, "DNS", destIP);
}

void DNSServer::sendUDPMessage(std::string msg, int destPort,
                               std::string upp_type, std::string destIP)
{
    std::string message; 
    if (upp_type == "DNS") {
        message = "53;";
    }
    message += std::to_string(destPort) + ";" + upp_type + "|" + msg;
    sendIPMessage(message, destIP, "UDP");
}

void DNSServer::sendIPMessage(std::string msg, std::string destIP, std::string upp_type)
{
    std::string message = ownIP + ";" + destIP + ";" + upp_type + "|" + msg;
    sendEthernetMessage(message, routerMAC, "IP");
}

void DNSServer::sendEthernetMessage(std::string msg, std::string destMAC, std::string upp_type)
{
    std::string message = destMAC + ";" + ownMAC + ";" + upp_type + "|" + msg;
    sendMessage(message, routerOSAddress);
}


void DNSServer::handleMsg(std::string msg, sockaddr_in senderOSAddress)
{
    handleEthernetHeader(msg);
}

void DNSServer::handleEthernetHeader(std::string recv_msg)
{
    int end_ethernet_header = recv_msg.find_first_of("|");
    std::string ethernet_header = recv_msg.substr(0, end_ethernet_header);
    std::string data = recv_msg.substr(end_ethernet_header + 1, std::string::npos);

    std::vector<std::string> parsed_header = parseHeader(ethernet_header);
    if (parsed_header[2] == "IP") {
        handleIPHeader(data);
    }
}

void DNSServer::handleIPHeader(std::string recv_msg)
{
    int end_ip_header = recv_msg.find_first_of("|");
    std::string ip_header = recv_msg.substr(0, end_ip_header);
    std::string data = recv_msg.substr(end_ip_header + 1, std::string::npos);

    std::vector<std::string> parsed_header = parseHeader(ip_header);

    std::string senderIP = parsed_header[0];
    std::string recvIP = parsed_header[1];
    std::string op = parsed_header[2];


    if (op == "UDP") {
        handleUDPHeader(data, senderIP);
    }
}

void DNSServer::handleUDPHeader(std::string recv_msg, std::string destIP) 
{
    int end_udp_header = recv_msg.find_first_of("|");
    std::string udp_header = recv_msg.substr(0, end_udp_header);
    std::string data = recv_msg.substr(end_udp_header + 1, std::string::npos);
    std::vector<std::string> parsed_header = parseHeader(udp_header);

    std::string senderPort = parsed_header[0];
    std::string receivedPort = parsed_header[1];
    std::string operation = parsed_header[2];


    if (operation == "DNS" && senderPort == "53" && receivedPort == "53") {
        handleDNSHeader(data, destIP);
    }
}

void DNSServer::handleDNSHeader(std::string recv_msg, std::string destIP)
{
    std::vector<std::string> parsed_header = parseHeader(recv_msg);

    std::string operation = parsed_header[0];
    std::string domain_name = parsed_header[1];
    
    if (operation == "REGISTER") {
        std::string receivedIP = parsed_header[2];
        std::cout << "REGISTRATION FOR DOMAIN NAME: " << domain_name << "\n";
        addRecord(domain_name, receivedIP);
    } else if (operation == "REQUEST") {
        std::cout << "REQUEST: " << domain_name << "\n";
        if (containsRecord(domain_name)) {
            std::cout << "Record found\n";
            sendDNSResponse(domain_name, getRecord(domain_name), destIP);
        } else {
            std::cout << "Record not found\n";
            sendDNSResponse(domain_name, "0.0.0.0", destIP);
        }
    }
}


void DNSServer::addRecord(std::string domain_name, std::string ip) 
{
    dnsRecords[domain_name] = ip;
    std::cout << "Added record: " << domain_name << "  " << ip << "\n";
}

bool DNSServer::containsRecord(std::string domain_name)
{
    return (dnsRecords.find(domain_name) != dnsRecords.end());
}

std::string DNSServer::getRecord(std::string domain_name)
{
    return (*(dnsRecords.find(domain_name))).second;
}


void DNSServer::start() 
{
    Device::start();
}


int main() 
{
    std::string DNSServerIP = "173.160.25.0";
    std::string DNSServerMAC = "ab:ab:ab:ab:ab:ab";
    int DNSServerPort = 6006;

    DNSServer dns_server(DNSServerIP, DNSServerMAC, DNSServerPort);
    dns_server.start();
    std::this_thread::sleep_for(std::chrono::seconds(600));
}