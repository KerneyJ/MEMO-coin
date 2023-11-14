#include <iostream>
#include <string>
#include <array>
#include <iomanip> 
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "defs.hpp"
#include "wallet.hpp"
#include "keys.hpp"

#include <fstream>

using SHA256Hash = std::array<unsigned char, 32>;
// // Overload << for SHA256Hash
// std::ostream& operator<<(std::ostream& os, const SHA256Hash& hash) {
//     for (auto byte : hash) {
//         os << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
//     }
//     return os;
// }
// Overload >> for SHA256Hash
std::istream& operator>>(std::istream& is, SHA256Hash& hash) {
    // Assuming the input is in hexadecimal format
    for (auto& byte : hash) {
        int tmp;
        if (is >> std::hex >> tmp) {
            byte = static_cast<unsigned char>(tmp);
        } else {
            is.setstate(std::ios_base::failbit);
            return is;
        }
    }
    return is;
}

Wallet create_wallet() {
    Ed25519Key pub_key, priv_key;
    gen_keys_ed25519(pub_key, priv_key);
    
    return { priv_key, pub_key };
}

// load wallet from file
Wallet load_wallet(std::string filepath) {
    return {};
}

// Function to write a wallet to a file
void store_wallet(const std::string& filepath, const Wallet& wallet) {
    // Open the file for writing in binary mode
    std::ofstream file(filepath, std::ios::binary);

    if (file.is_open()) {
        // Write the private key to the file
        file.write(reinterpret_cast<const char*>(wallet.priv_key.data()), wallet.priv_key.size());

        // Write the public key to the file
        file.write(reinterpret_cast<const char*>(wallet.pub_key.data()), wallet.pub_key.size());

        // Close the file
        file.close();
        std::cout << "Wallet stored successfully." << std::endl;
    } else {
        std::cerr << "Error opening file: " << filepath << std::endl;
    }
}

// signature is generated in create_transaction, it should sign all of the other fields in the transaction struct
// the id of the transaction should be unique for each transaction generated by a wallet 
Transaction create_transaction(SHA256Hash src, SHA256Hash dest, uint32_t amount) {
    Transaction new_tr;
    new_tr.src = src;
    new_tr.dest = dest;
    new_tr.amount = amount;
    // new_tr.id = create_tx_id(new_tr.src, new_tr.dest, new_tr.amount);    


    return {};
}

// Function to display wallet information
void display_wallet(const Wallet& wallet) {
    // Display the wallet information
    std::cout << "Public Key: ";
    for (auto byte : wallet.pub_key) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    std::cout << std::endl;

    // Display the private key (for demonstration purposes)
    std::cout << "Private Key: ";
    for (auto byte : wallet.priv_key) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    std::cout << std::endl;
}

int main(int argc, char** argv) {
    struct Wallet wallet;

    while (true) {
        std::cout << "\n\n---------------------------------\n\n";
        std::cout << "\n\nWelcome to da crypto wallet 💰💰💰\n\n";
        std::cout << "What would you like to do?\n";
        std::cout << "Quit                  [0]\n";
        std::cout << "create wallet         [1]\n";
        std::cout << "load wallet           [2]\n";
        std::cout << "Display wallet        [3]\n";
        std::cout << "create transaction    [4]\n";
        std::cout << "Query balance         [5]\n";
        std::cout << "Your selection:        ";
        
        int menu_selection;
        std::cin >> menu_selection;
        
        switch (menu_selection) {
            case 0: {
                std::cout << "Goodbye! ✌️\n";
                return 0;
            }

            case 1: {
                Wallet wallet = create_wallet();
                std::cout << "\nWallet created successfully!\n";
                display_wallet(wallet);
                std::cout << "\nInput file path to save file to path: ";
                
                // Use std::getline to get a file path that may contain spaces
                std::string filePath;
                std::getline(std::cin, filePath);

                std::cout << "You entered: " << filePath << std::endl;
                break;
            }

            case 2: {
                // TODO
                std::cout << "`load_wallet` not implemented.";
                // std::cout << "Please enter the filepath to your wallet: ";
                break;
            }

            case 3: {
                std::cout << "Displaying your wallet...";
                display_wallet(wallet); 
                break;
            }

            case 4: {
                // TODO


                std::cout << "`create transaction` not implemented.";
                break;

                SHA256Hash dest;
                uint32_t amount;
                std::cout << "Creating transaction...";
                std::cout <<"enter address of payment recipient: ";
                std::cin >> dest;
                std::cout <<"Enter the amount to be paid: ";
                std::cin >> amount;
                
                // create the transaction
                // TODO
                Transaction mew_transaction;
                
                //Display the transaction;
                std::cout << "Submit tramsaction?";

            }

            case 5: {
                std::cout << "query_balance() not implemented";
                // int balance = query_balance();
                // std::cout << "Your balance is: " << balance << std::endl;
                break;
            }
            

            default: {
                std::cout << "Invalid choice. Please try again \n";
            }
        }
    }

    return 0;
}