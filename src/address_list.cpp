#include <queue>

#include "address_list.hpp"

void AddressList::add_address(std::string address) {
    addresses.push(address);
}

std::string AddressList::get_address() {
    std::string address = addresses.front();

    addresses.pop();
    addresses.push(address);

    return address;
}