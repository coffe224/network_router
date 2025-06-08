#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <string>
#include <vector>
#include <iostream>

#include "ARPTable.h"

int ARPTable::add(std::string IP, std::string MAC, sockaddr_in address)
{
    for (auto& row : arp_table) {
        if (IP == row.IP && MAC == row.MAC) {
            row.address = address;
            std::cout << "Entry updated in ARP table\n";
            return 0;
        }
        if (IP == row.IP) {
            std::cout << "TWO SAME IPS - BAN\n";
            throw;
        }
        if (MAC == row.MAC) {
            std::cout << "TWO SAME MACS - BAN\n";
            throw;
        }
    }
    arp_table.push_back({IP, MAC, address});
    std::cout << "Entry added to ARP table\n";
    return 0;
}

std::string ARPTable::findMAC(std::string IP) 
{ 
    for (auto& row : arp_table) {
        if (IP == row.IP) {
            return row.MAC;
        }
    }
    std::cout << "MAC for " << IP << " is not found\n";
    return std::string(); 
}

sockaddr_in ARPTable::findAdress(std::string MAC) 
{ 
    for (auto& row : arp_table) {
        if (MAC == row.MAC) {
            return row.address;
        }
    }
    std::cout << "Address is not found\n";
    return sockaddr_in(); 
}
