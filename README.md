# How to setup
Download, build, and install libzmq
```
git clone git@github.com:zeromq/libzmq.git
cd libzmq
./autogen.sh 
./configure && make
sudo make install
sudo ldconfig
cd ..
```

Download, build, and install C++ libzmq API and headers
```
git clone git@github.com:zeromq/cppzmq.git
cd cppzmq
mkdir build
cd build
cmake ..
sudo make install
```

Download, build, and install Alpaca
```
git clone --recurse-submodules https://github.com/p-ranav/alpaca
cd alpaca
cmake -DALPACA_BUILD_TESTS=on \
      -DALPACA_BUILD_BENCHMARKS=on \
      -DALPACA_BUILD_SAMPLES=on \
      -DCMAKE_BUILD_TYPE=Releasei
make
./test/tests
make install
```

Download, build, and install YAML C++
```
git clone git@github.com:jbeder/yaml-cpp.git
mkdir build
cd build
cmake ..
make install
```

# How to Run
```
# All in seperate shells
./bin/dsc blockchain [optional: path to stored blockchain]
./bin/dsc metronome
./bin/dsc pool
./bin/dsc validator
```

# Stuff To Do
* Make Blockchain Persistent
* Make Blockchain Distriuted(multiple blockchain nodes to exist, simple flooding mechanism)
  * modify config file to point new blockchain nodes to existing blockchain nodes
  * pick random blockchain node for load balancing
* Make Transaction Pool Distributed
* Make Metronome Distributed
* Try not to add complexity to the wallet or the validator(push any more complexity to the blockchain, transaction pool, or metronome)
* Convert proof of memory to proof of storage

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





