CC=gcc
CPP=g++

CFLAGS=
CPP_FLAGS=-pthread

BIN_DIR=bin
OBJ_DIR=obj
SRC_DIR=src

BLAKE_SRCS=blake3.c blake3_dispatch.c blake3_portable.c blake3_sse2_x86-64_unix.S \
			blake3_sse41_x86-64_unix.S blake3_avx2_x86-64_unix.S blake3_avx512_x86-64_unix.S
BLAKE_OBJS=$(patsubst %.c,%.o,$(patsubst %.S,%.o,$(BLAKE_SRCS)))

C_SRCS=
CPP_SRCS=
C_OBJS=$(C_SRCS:.c=.o)
CPP_OBJS=$(CPP_SRCS:.cpp=.o)

TARGETS=pow.cpp wallet.cpp

ifeq ($(DEBUG),true)
    CFLAGS := -D DEBUG
    CPP_FLAGS := $(CPP_FLAGS) -D DEBUG
endif

all: dir $(BLAKE_SRCS) $(TARGETS)

$(BLAKE_SRCS):
	$(CC) $(CFLAGS) -c $(SRC_DIR)/blake3/$@ -o $(OBJ_DIR)/$(patsubst %.c,%.o,$(patsubst %.S,%.o,$@))

$(TARGETS): $(C_OBJS) $(CPP_OBJS) $(BLAKE_SRCS)
	$(CPP) $(CPP_FLAGS) $(SRC_DIR)/$@ -o $(BIN_DIR)/$(patsubst %.cpp,%,$@) \
		$(addprefix $(OBJ_DIR)/,$(C_OBJS)) $(addprefix $(OBJ_DIR)/,$(CPP_OBJS)) $(addprefix $(OBJ_DIR)/,$(BLAKE_OBJS))

$(CPP_OBJS): %.o: $(SRC_DIR)/%.cpp 
	$(CPP) $(CPP_FLAGS) -c $< -o $(OBJ_DIR)/$@

$(C_OBJS): %.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o $(OBJ_DIR)/$@ $(CFLAGS)

dir:
	echo $(BLAKE_SRCS)
	mkdir -p bin
	mkdir -p obj

clean:
	rm -f $(BIN_DIR)/*
	rm -f $(OBJ_DIR)/*
