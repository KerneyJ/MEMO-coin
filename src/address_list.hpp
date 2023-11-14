#include <queue>
#include <string>

#pragma once

// For making round robin requests to a set of addresses
class AddressList {
    private:
        std::queue<std::string> addresses;
    public:
        void add_address(std::string);
        std::string get_address();
};