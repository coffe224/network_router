#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <iostream>
#include <cstring>
#include <unistd.h>

#include <string>
#include <vector>
#include <sstream>
#include <thread>
#include <array>

#include "Client.h"

Client::Client(std::string client_ip, std::string client_mac, unsigned int client_port) {
    // Create socket
    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == -1) {
        std::cout << "No socket for you\n";
        throw;
    }
    std::cout << "Socket created\n";

    
    // Bind client address to socket 
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(client_port);
    clientAddress.sin_addr.s_addr = INADDR_ANY;
    
    int state = bind(clientSocket, (sockaddr*)&clientAddress, sizeof(clientAddress));
    if (state != 0) {
        std::cout << "No bind for you\n";
        throw;
    }

    ownIP = client_ip;
    ownMAC = client_mac;

    // Get router address
    routerAddress.sin_family = AF_INET;
    routerAddress.sin_port = htons(routerPort);
    routerAddress.sin_addr.s_addr = INADDR_ANY;
    
    std::cout << "DONE!\n";
}

void Client::start() 
{
    receiverThread = std::thread(&Client::recvMessage, this);
}

std::vector<std::string> Client::parseHeader(std::string header)
{
    std::vector<std::string> parsed_header;

    std::stringstream ss(header);
    std::string token;

    while (std::getline(ss, token, ';')) {
        parsed_header.push_back(token);
    }

    return parsed_header;
}

void Client::sendICMPMessage(std::string op, std::string destIP)
{
    sendIPMessage(op, destIP, "ICMP");
}

void Client::sendIPMessage(std::string msg, std::string destIP, std::string upp_type)
{
    std::string message = ownIP + ";" + destIP + ";" + upp_type + "|" + msg;

    sendEthernetMessage(message, routerMAC, "IP");
}

void Client::sendARPRequest()
{
    sleep(1);
    while (!startedReceiving) {}
    std::string message = "REQ;" + ownIP + ";" + routerIP + ";" + ownMAC;
    sendEthernetMessage(message, "ff:ff:ff:ff:ff", "ARP");
}

void Client::sendEthernetMessage(std::string msg, std::string destMAC, std::string upp_type)
{
    if (!canSendPing && upp_type != "ARP") {
        std::cout << "Can't send: no router MAC...\n";
    } else {
        std::string message = ownMAC + ";" + destMAC + ";" + upp_type + "|" + msg;
        sendMessage(message, routerAddress);
    }
}


void Client::sendMessage(std::string msg, sockaddr_in destAddress)
{
    // std::cout << "\nSending message!\n";
    // std::cout << msg << "\n\n";
    sendto(clientSocket, msg.data(), msg.length(), 0, (sockaddr*)&destAddress, sizeof(destAddress));
    // std::cout << "Message sended\n\n";
}

void Client::recvMessage()
{
    while (1) {
        char buffer[1024] = {0};
        
        startedReceiving = true;
        recvfrom(clientSocket, buffer, sizeof(buffer), 0, NULL, 0);
        // std::cout << "\nReceived message!\n";
        // std::cout << buffer << "\n\n";
        // std::cout << "Message received, start handling\n\n";
        
        std::string msg(buffer);
        handleEthernetHeader(msg);
    }
}

void Client::handleEthernetHeader(std::string recv_msg)
{
    int end_ethernet_header = recv_msg.find_first_of("|");
    std::string ethernet_header = recv_msg.substr(0, end_ethernet_header);
    std::string data = recv_msg.substr(end_ethernet_header + 1, std::string::npos);

    // std::cout << "ETHERNET HEADER: " << ethernet_header << "\n";
    // std::cout << "DATA: " << data << "\n";


    std::vector<std::string> parsed_header = parseHeader(ethernet_header);

    if (parsed_header[1] == ownMAC || parsed_header[1] == "ff:ff:ff:ff:ff") {
        if (parsed_header[2] == "ARP") {
            handleARPResponse(data);
        } else if (parsed_header[2] == "IP") {
            handleIPHeader(data);
        }
    }
}

void Client::handleARPResponse(std::string recv_msg)
{
    std::vector<std::string> parsed_header = parseHeader(recv_msg);

    if (parsed_header[0] == "RESP" && parsed_header[1] == routerIP && parsed_header[2] == ownIP) {
        routerMAC = parsed_header[3];
        canSendPing = true;
    }
}

void Client::handleIPHeader(std::string recv_msg)
{
    int end_ip_header = recv_msg.find_first_of("|");
    std::string ip_header = recv_msg.substr(0, end_ip_header);
    std::string data = recv_msg.substr(end_ip_header + 1, std::string::npos);

    std::vector<std::string> parsed_header = parseHeader(ip_header);
    std::string recvIP = parsed_header[0];

    if (parsed_header[1] == ownIP) {
        if (parsed_header[2] == "ICMP") {
            handleICMPHeader(data, recvIP);
        }
    }


}

void Client::handleICMPHeader(std::string recv_msg, std::string recvIP)
{
    if (recv_msg == "PING") {
        std::cout << "RECEIVED PING FROM IP " << recvIP << "  SENDING PONG" << "\n\n";
        sendICMPMessage("PONG", recvIP);
    } else if (recv_msg == "PONG") {
        std::cout << "RECEIVED PONG FROM IP " << recvIP << "\n\n";
    }
}



int main(int argc, char *argv[]) {
    int number = std::stoi(argv[1]);
    std::cout << "NUMBER: " << number << "\n\n";

    std::vector<std::string> clients_ips = {
        "192.213.234.12",
        "192.213.234.13",
        "192.213.234.14"
    };

    std::vector<std::string> clients_macs = {
        "aa:bb:cc:dd:aa:12",
        "aa:bb:cc:dd:aa:13",
        "aa:bb:cc:dd:aa:14"
    };

    std::vector<unsigned int> clients_ports = {
        6001,
        6002,
        6003
    };

    Client client(clients_ips[number], clients_macs[number], clients_ports[number]);
    client.start();

    client.sendARPRequest();

    while (1) {
        std::cout << "SEND PING TO IP " << clients_ips[(number + 1) % 3] << "\n";
        client.sendICMPMessage("PING", clients_ips[(number + 1) % 3]);
        sleep(5);

        std::cout << "SEND PING TO IP " << clients_ips[(number + 2) % 3] << "\n";
        client.sendICMPMessage("PING", clients_ips[(number + 2) % 3]);

        sleep(5);
    }

    return 0;
}