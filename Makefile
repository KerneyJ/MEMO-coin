CC=gcc
CPP=g++

CFLAGS=
CPP_FLAGS=-lssl -lcrypto -lzmq -lyaml-cpp -luuid

BIN_DIR=bin
OBJ_DIR=obj
SRC_DIR=src

BLAKE_DIR=./external/blake3
BLAKE_SRCS=blake3.c blake3_dispatch.c blake3_portable.c blake3_sse2_x86-64_unix.S \
			blake3_sse41_x86-64_unix.S blake3_avx2_x86-64_unix.S blake3_avx512_x86-64_unix.S
BLAKE_OBJS=$(patsubst %.c,%.o,$(patsubst %.S,%.o,$(BLAKE_SRCS)))

BASE58_DIR=./external/base58
BASE58_FILES=$(wildcard $(BASE58_DIR)/*.c)
BASE58_SRCS=$(BASE58_FILES:$(BASE58_DIR)/%=%)
BASE58_OBJS=$(BASE58_SRCS:.c=.o)

C_SRCS=
CPP_SRCS=keys.cpp wallet.cpp transaction.cpp utils.cpp server.cpp thread_pool.cpp \
		tx_pool.cpp blockchain.cpp config.cpp validator.cpp pow.cpp pom.cpp metronome.cpp
C_OBJS=$(C_SRCS:.c=.o)
CPP_OBJS=$(CPP_SRCS:.cpp=.o)

TARGETS=dsc.cpp

ifeq ($(DEBUG),true)
    CFLAGS := -D DEBUG
    CPP_FLAGS := $(CPP_FLAGS) -D DEBUG
endif

all: dir $(BLAKE_SRCS) $(BASE58_OBJS) $(TARGETS)

$(BLAKE_SRCS):
	$(CC) -c $(BLAKE_DIR)/$@ -o $(OBJ_DIR)/$(patsubst %.c,%.o,$(patsubst %.S,%.o,$@)) $(CFLAGS) 

$(BASE58_OBJS): %.o: $(BASE58_DIR)/%.c
	$(CC) -c $< -o $(OBJ_DIR)/$@ $(CFLAGS) 

$(TARGETS): $(C_OBJS) $(CPP_OBJS) $(BLAKE_SRCS)
	$(CPP) $(SRC_DIR)/$@ -o $(BIN_DIR)/$(patsubst %.cpp,%,$@) \
		$(addprefix $(OBJ_DIR)/,$(C_OBJS)) $(addprefix $(OBJ_DIR)/,$(CPP_OBJS)) $(addprefix $(OBJ_DIR)/,$(BLAKE_OBJS)) $(addprefix $(OBJ_DIR)/,$(BASE58_OBJS)) $(CPP_FLAGS) 

$(CPP_OBJS): %.o: $(SRC_DIR)/%.cpp 
	$(CPP) -c $< -o $(OBJ_DIR)/$@ $(CPP_FLAGS) 

$(C_OBJS): %.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o $(OBJ_DIR)/$@ $(CFLAGS)

test:
	g++ -lzmq -o bin/test_server zmqtest/server.cpp
	g++ -lzmq -o bin/test_client zmqtest/client.cpp

dir:
	echo $(BLAKE_SRCS)
	mkdir -p bin
	mkdir -p obj

clean:
	rm -f $(BIN_DIR)/*
	rm -f $(OBJ_DIR)/*
