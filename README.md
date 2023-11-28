# How to setup

```
git clone git://github.com/zeromq/libzmq.git
cd libzmq
./autogen.sh 
./configure && make
sudo make install
sudo ldconfig
cd ..

git clone --recurse-submodules https://github.com/p-ranav/alpaca
cd alpaca
cmake -DALPACA_BUILD_TESTS=on \
      -DALPACA_BUILD_BENCHMARKS=on \
      -DALPACA_BUILD_SAMPLES=on \
      -DCMAKE_BUILD_TYPE=Release
make
./test/tests
make install

git clone git@github.com:jbeder/yaml-cpp.git
mkdir build
cd build
cmake ..
make install
```

# Stuff To Do

## OOP Architecture
### Wallet
* has a Pub key
* has aPriv key
* has a wallet id
* has a Blockchain Node => address of some node
* Stored on disk
* exists in its own process(cli/benchmark thing)

### Tx Pool
* has a list of Peers
* has a list of Txs
* has a method to add tx
* has a method to get/remove(pop) n transactions, needs to propegate pop to all peers
* exists in its own process

### Transaction
* has a Source Wallet
* has a Destination Wallet
* has a amount
* has a signature from source wallet

### Validator
* has a wallet
* has a difficulty variable
* has a Blockchain Node => address of some node
* has a Tx Pool Node => address of some node
* has a method to create blocks
* has a consensus mechanism
  * PoW => solve hash method
  * PoM => solve hash method
  * PoS => solve hash method
* has a method to request txs from the tx pool

### Block
* has a prev block
* has a curr hash
* has a difficulty
* has a tx list

### Blockchain
* has a linked list of blocks(merkle tree?)
* has a ledger
* has a method to append blocks
* has a method to validate blocks
* has a method to get latest hash

### Ledger
* has a list of wallet balances
* has a method to get balance(wallet id)
* has a method to update balances(block/txs)

### Metronome
* has a Blockchain Node => address of some node
* has a method to create an empty block

## External Utils
* openssl (key generation)
* BLAKE3
* Alpaca (serialization)
* ZMQ
* yaml-cpp

# apt-get install these
* libssl-dev
* libzmq3-dev
* libyaml-cpp-dev
* uuid-dev

# install alpaca: https://github.com/p-ranav/alpaca/tree/master#building-installing-and-testing





