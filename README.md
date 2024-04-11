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
* Make Blockchain Persistent
* Make Blockchain Distriuted(multiple blockchain nodes to exist, simple flooding mechanism)
* Make Transaction Pool Distributed
* Make Metronome Distributed
* Try not to add complexity to the wallet or the validator(push any more complexity to the blockchain, transaction pool, or metronome)

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

# install alpaca:
* https://github.com/p-ranav/alpaca/tree/master#building-installing-and-testing





