#include <iostream>
#include <unordered_map>
#include <string>
#include <optional>
#include <utility>

class NAT_Table {
private:
    std::unordered_map<std::string, std::string> nat_map;

public:
    void add_entry(std::string private_ip) {
    }

    void remove_entry(const std::string& private_ip) {
    }

    std::string get_public_mapping(std::string private_ip) const {
    }

    std::string get_private_mapping(std::string public_ip, int NAT_port) const {
    }
};

// Example usage:
int main() {
    return 0;
}