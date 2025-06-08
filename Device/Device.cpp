#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>

#include "Device.h"

Device::Device(std::string IP, std::string MAC, unsigned int OS_port)
{
    ownIP = IP;
    ownMAC = MAC;
    OSSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (OSSocket == -1) {
        std::cout << "No socket for you\n";
        throw;
    }
    std::cout << "Socket created\n";

    // Bind client address to socket 
    OSAddress.sin_family = AF_INET;
    OSAddress.sin_port = htons(OS_port);
    OSAddress.sin_addr.s_addr = INADDR_ANY;
    
    int state = bind(OSSocket, (sockaddr*)&OSAddress, sizeof(OSAddress));
    if (state != 0) {
        std::cout << "No bind for you\n";
        throw;
    }
    std::cout << "Socket binded\n";
}

void Device::sendMessage(std::string msg, sockaddr_in destAddress)
{
    if (isWorking) {
        // std::cout << "=====================\n";
        // std::cout << msg << "\n";
        // std::cout << "=====================\n";
        sendto(OSSocket, msg.data(), msg.length(), 0,
        (sockaddr*)&destAddress, sizeof(destAddress));
        // std::cout << "Message sended!\n";
    } else {
        std::cout << "Can't send message\n";
    }
}

void Device::recvMessageThread()
{
    while (true) {
        char buffer[1024] = {0};
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);

        recvfrom(OSSocket, buffer, sizeof(buffer), 0, 
                 (sockaddr*)&clientAddr, &clientLen);
        
        std::string msg(buffer);
        
        std::unique_lock<std::mutex> lock(recvCVMtx);
        msgQueue.push(Message(msg, clientAddr));
        // std::cout << "Message received\n";
        // std::cout << "=====================\n";
        // std::cout << msg << "\n";
        // std::cout << "=====================\n";
        recvCV.notify_all();
        lock.unlock();
    }
}

void Device::handleMessageThread()
{
    while (true) {
        std::unique_lock<std::mutex> lock(recvCVMtx);
        recvCV.wait(lock, [this]() { return !msgQueue.empty(); });
        Message message = msgQueue.front();
        msgQueue.pop();
        lock.unlock();
        std::string str_msg = message.message;
        if (str_msg.rfind(ownMAC, 0) == 0 || str_msg.rfind("ff:ff:ff:ff:ff:ff", 0) == 0) {
            handleMsg(message.message, message.sender);
        } else {
            std::cout << "Not for you\n";
        }
    }
}


void Device::start() {
    receiverThread = std::thread(&Device::recvMessageThread, this);
    handlerThread = std::thread(&Device::handleMessageThread, this);
    isWorking = true;
}

std::vector<std::string> Device::parseHeader(std::string header)
{
    std::vector<std::string> parsed_header;

    std::stringstream ss(header);
    std::string token;

    while (std::getline(ss, token, ';')) {
        parsed_header.push_back(token);
    }

    return parsed_header;
}

Device::~Device() {}