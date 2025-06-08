#include "NewClient.h"

#include <unistd.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

NewClient::NewClient(std::string IP, std::string MAC, unsigned int OS_port, std::string filename) : 
Device(IP, MAC, OS_port)
{
    routerOSAddress.sin_family = AF_INET;
    routerOSAddress.sin_port = htons(routerOSPort);
    routerOSAddress.sin_addr.s_addr = INADDR_ANY;

    fileName = filename;

    std::cout << "-------- CLIENT ---------\n";

}

void NewClient::start() {
    Device::start();
    sendDHCPDiscover();

    std::unique_lock<std::mutex> lock1(state_mtx);
    state_cv.wait(lock1, [this]() { return state == State::DHCP_ACK; });
    lock1.unlock();

    sendARPRequest();

    std::unique_lock<std::mutex> lock2(state_mtx);
    state_cv.wait(lock2, [this]() { return state == State::ARP_RESPONSE; });
    lock2.unlock();

}


void NewClient::sendPing(std::string destIP)
{
    sendICMPMessage("PING", destIP);
}

void NewClient::sendICMPMessage(std::string op, std::string destIP)
{
    sendIPMessage(op, destIP, "ICMP");
}

void NewClient::sendDHCPDiscover()
{
    std::string message = "DISCOVER;" + ownMAC;
    sendIPMessage(message, "255.255.255.255", "DHCP");
}

void NewClient::sendDHCPRequest(std::string receivedIP)
{
    std::string message = "REQUEST;" + ownMAC + ";" + receivedIP;
    sendIPMessage(message, "255.255.255.255", "DHCP");
}

void NewClient::sendHTTPRequest(std::string destIP) 
{
    std::string message = "GET";
    sendUDPMessage(message, 80, "HTTP", destIP);
}

void NewClient::sendHTTPResponse(std::string destIP, std::string html_page) 
{
    std::string message = "POST|" + html_page;
    sendUDPMessage(message, 80, "HTTP", destIP);
}

void NewClient::sendDNSRequest(std::string domain_name)
{
    std::string message = "REQUEST;" + domain_name;
    sendUDPMessage(message, 53, "DNS", DNSServerIP);
}

void NewClient::sendDNSRegister(std::string domain_name)
{
    std::string message = "REGISTER;" + domain_name + ";" + ownIP;
    sendUDPMessage(message, 53, "DNS", DNSServerIP);
}

void NewClient::sendUDPMessage(std::string msg, int destPort,
                               std::string upp_type, std::string destIP)
{
    std::string message; 
    if (upp_type == "DNS") {
        message = "53;";
    } else if (upp_type == "HTTP") {
        message = "80;";
    }
    message += std::to_string(destPort) + ";" + upp_type + "|" + msg;
    sendIPMessage(message, destIP, "UDP");
}

void NewClient::sendIPMessage(std::string msg, std::string destIP, std::string upp_type)
{
    std::string sendedIP = ownIP == "" ? "0.0.0.0" : ownIP;
    std::string message = sendedIP + ";" + destIP + ";" + upp_type + "|" + msg;

    std::string destMAC = routerMAC == "" ? "ff:ff:ff:ff:ff:ff" : routerMAC;
    sendEthernetMessage(message, destMAC, "IP");
}

void NewClient::sendARPRequest()
{
    std::string message = "REQ;" + ownIP + ";" + routerIP + ";" + ownMAC;
    sendEthernetMessage(message, "ff:ff:ff:ff:ff:ff", "ARP");
    sleep(1);
}

void NewClient::sendEthernetMessage(std::string msg, std::string destMAC, std::string upp_type)
{
    if (destMAC == "") {
        std::cout << "Can't send message, empty destination MAC...\n";
        return;
    }
    std::string message = destMAC + ";" + ownMAC + ";" + upp_type + "|" + msg;
    sendMessage(message, routerOSAddress);
}


void NewClient::handleMsg(std::string msg, sockaddr_in senderOSAddress)
{
    handleEthernetHeader(msg);
}

void NewClient::handleEthernetHeader(std::string recv_msg)
{
    int end_ethernet_header = recv_msg.find_first_of("|");
    std::string ethernet_header = recv_msg.substr(0, end_ethernet_header);
    std::string data = recv_msg.substr(end_ethernet_header + 1, std::string::npos);

    std::vector<std::string> parsed_header = parseHeader(ethernet_header);

    if (parsed_header[2] == "ARP") {
        handleARPResponse(data);
    } else if (parsed_header[2] == "IP") {
        handleIPHeader(data);
    }
}

void NewClient::handleARPResponse(std::string recv_msg)
{
    std::vector<std::string> parsed_header = parseHeader(recv_msg);

    if (parsed_header[0] == "RESP" && parsed_header[1] == routerIP && parsed_header[2] == ownIP) {
        routerMAC = parsed_header[3];
        state = ARP_RESPONSE;
        std::cout << "ARP RESPONCE RECEIVED, CAN SEND TO ROUTER\n";
    }
}

void NewClient::handleIPHeader(std::string recv_msg)
{
    int end_ip_header = recv_msg.find_first_of("|");
    std::string ip_header = recv_msg.substr(0, end_ip_header);
    std::string data = recv_msg.substr(end_ip_header + 1, std::string::npos);

    std::vector<std::string> parsed_header = parseHeader(ip_header);
    std::string recvIP = parsed_header[0];

    std::string senderIP = parsed_header[0];
    std::string destIP = parsed_header[1];
    std::string op = parsed_header[2];

    if (destIP == "255.255.255.255" && op == "DHCP") {
        handleDHCPHeader(data);
        return;
    } else if (destIP == ownIP) {
        if (op == "ICMP") {
            handleICMPHeader(data, recvIP);
        } else if (op == "UDP") {
            handleUDPHeader(data, recvIP);
        }
    }
}

void NewClient::handleICMPHeader(std::string recv_msg, std::string recvIP)
{
    if (recv_msg == "PING") {
        std::cout << "RECEIVED PING FROM IP " << recvIP << "  SENDING PONG" << "\n\n";
        sendICMPMessage("PONG", recvIP);
    } else if (recv_msg == "PONG") {
        std::cout << "RECEIVED PONG FROM IP " << recvIP << "\n\n";
    }
}


void NewClient::handleDHCPHeader(std::string recv_msg)
{
    std::vector<std::string> parsed_header = parseHeader(recv_msg);

    std::string operation = parsed_header[0];
    std::string receivedMAC = parsed_header[1];
    std::string receivedIP = parsed_header[2];

    if (receivedMAC != ownMAC) {
        std::cout << "Not for me I guess\n";
        return;
    }

    if (operation == "OFFER") {
        sendDHCPRequest(receivedIP);
        std::cout << "RECEIVED OFFER\n";
    } else if (operation == "ACKNOWLEDGE") {
        ownIP = receivedIP;
        DNSServerIP = parsed_header[3];
        std::cout << "RECEIVED IP: " + receivedIP + "\n";
        state = DHCP_ACK;
        state_cv.notify_all();
    }
}

void NewClient::handleUDPHeader(std::string recv_msg, std::string recvIP) 
{
    int end_udp_header = recv_msg.find_first_of("|");
    std::string udp_header = recv_msg.substr(0, end_udp_header);
    std::string data = recv_msg.substr(end_udp_header + 1, std::string::npos);
    std::vector<std::string> parsed_header = parseHeader(udp_header);

    std::string senderPort = parsed_header[0];
    std::string receivedPort = parsed_header[1];
    std::string operation = parsed_header[2];

    if (operation == "DNS" && senderPort == "53" && receivedPort == "53") {
        handleDNSHeader(data);
    } else if (operation == "HTTP" && senderPort == "80" && receivedPort == "80") {
        handleHTTPHeader(data, recvIP);
    }
}


void NewClient::handleDNSHeader(std::string recv_msg)
{
    std::vector<std::string> parsed_header = parseHeader(recv_msg);

    std::string operation = parsed_header[0];
    std::string domain_name = parsed_header[1];
    std::string receivedIP = parsed_header[2];

    if (operation == "RESPONSE") {
        std::cout << "RECEIVED IP FROM DNS SERVER\n";
        std::cout << "DOMAIN NAME: " << domain_name << " IP: " << receivedIP << "\n";
        sendHTTPRequest(receivedIP);
    }
}

void NewClient::handleHTTPHeader(std::string recv_msg, std::string recvIP) 
{
    int end_http_header = recv_msg.find_first_of("|");
    std::string http_header = recv_msg.substr(0, end_http_header);
    std::string data = recv_msg.substr(end_http_header + 1, std::string::npos);


    if (http_header == "GET") {
        std::cout << "RECEIVED GET REQUEST. SENDING HTML PAGE....\n";
        std::ifstream file(fileName);
    
        std::stringstream buffer;
        buffer << file.rdbuf();

        sendHTTPResponse(recvIP, buffer.str());
    } else if (http_header == "POST") {
        std::cout << "RECEIVED HTML PAGE:\n";
        std::cout << data;
    }
}


int main(int argc, char *argv[]) {
    int number = std::stoi(argv[1]);
    std::cout << "NUMBER: " << number << "\n\n";

    std::vector<std::string> clients_macs = {
        "aa:bb:cc:dd:aa:12",
        "aa:bb:cc:dd:aa:13"
    };

    std::vector<unsigned int> clients_ports = {
        6002,
        6003
    };

    std::vector<std::string> clients_htmls = {
        "files/file1.html",
        "files/file2.html"
    };

    NewClient client(clients_macs[number], clients_ports[number], clients_htmls[number]);

    std::string command;
    std::string arg;
    while (1) {
        std::cin >> command >> arg;

        if (command == "start") {
            client.start();
        } else if (command == "ping") {
            std::cout << "SEND PING TO IP " << arg << "\n";
            client.sendPing(arg);
        } else if (command == "register") {
            std::cout << "REGISTERING DOMAIN NAME " << arg << "\n";
            client.sendDNSRegister(arg);
        } else if (command == "request") {
            std::cout << "REQUESTING IP BY DOMAIN NAME " << arg << "\n";
            client.sendDNSRequest(arg);
        }
    }

    return 0;
}