#include "validator.hpp";
#include "wallet.hpp"

Validator::Validator() {
    difficulty = 1;
    wallet = load_wallet("./confing/wallet.cfg");
}

void Validator::run() {
    // TODO: error handling
    while(true) {
        auto problem = request_block_header();
        auto solution = create_block(problem.hash);
        submit_block(solution);

        printf("Submitted block!\n");
    }
}

//  {
//     private:
//         int difficulty;
//         std::string blockchain_address;
//         std::string tx_pool_address;
//         IConsensusModel* consensus;
//     public:
//         Block create_block(Blake3Hash hash);
//         int submit_block();
//         int request_block_header();
//         std::array<Transaction, BLOCK_SIZE> request_txs();
// };

int main() {
    Validator validator = Validator();
    validator.run();
}