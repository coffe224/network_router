#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <string>
#include <vector>

class ARPTable {
    private:

    struct ARPRow{
        std::string IP;
        std::string MAC;
        sockaddr_in address;

        ARPRow(std::string IP, std::string MAC, sockaddr_in address) : IP{IP}, MAC{MAC}, address{address} {};
    };

    std::vector<ARPRow> arp_table;

    public:

    int add(std::string IP, std::string MAC, sockaddr_in address);
    std::string findMAC(std::string IP);
    sockaddr_in findAdress(std::string MAC);
};