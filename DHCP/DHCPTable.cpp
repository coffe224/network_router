#include "DHCPTable.h"
#include <iostream>

std::string DHCPTable::getNextAvailableIP() 
{
    return baseIP + std::to_string(counter);
}

void DHCPTable::add(std::string IP, std::string MAC) 
{
    table.insert({IP, MAC});
    std::cout << "Added to table: " << IP << " " << MAC;
    counter++;
}

bool DHCPTable::contains(std::string IP, std::string MAC) 
{ 
    auto tmp = table.find(IP);
    if (tmp == table.end()) {
        return false;
    }
    if ((*tmp).second != MAC) {
        return false;
    }
    return true;
}
