#include <string>
#include <unordered_map>

class DHCPTable {
private:
    std::string baseIP = "192.168.1.";
    int counter = 0;

    std::unordered_map<std::string, std::string> table;

public:
    std::string getNextAvailableIP();

    void add(std::string IP, std::string MAC);
    bool contains(std::string IP, std::string MAC);
};