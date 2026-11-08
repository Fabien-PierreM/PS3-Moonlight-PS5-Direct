PROJECT := ..
PS3DEV ?= $(PROJECT)/ps3dev
CHIaki := $(PROJECT)/third_party/chiaki-ng
BUILD := $(PROJECT)/build/direct
CHIAKI_BUILD := $(PROJECT)/build/chiaki-ps3
CC := $(PS3DEV)/ppu/bin/ppu-gcc
INCLUDES := -I$(CHIaki)/ps3/include -I$(CHIaki)/lib/include -I$(BUILD)/generated -I$(PS3DEV)/ppu/include -I$(PS3DEV)/portlibs/ppu/include
CFLAGS := -mcpu=cell -mstrict-align -O2 -Wall -Wextra -Werror=implicit-function-declaration -include chiaki/config.h $(INCLUDES)
LIBS := -L$(CHIAKI_BUILD) -Wl,--start-group -lchiaki-ps3-session -lchiaki-ps3-crypto -lchiaki-ps3-protobuf -lchiaki-ps3-fec -lchiaki-ps3-platform -Wl,--end-group -L$(PS3DEV)/portlibs/ppu/lib -lpthread -lcurl -lmbedtls -lmbedx509 -lmbedcrypto -lnetctl -lnet -lio -lsysmodule -lsysutil -llv2 -lrt -lm -lz
all: $(BUILD)/chiaki-ps3-direct.elf
$(BUILD):
	mkdir -p $@
$(BUILD)/direct_main.o: $(PROJECT)/src/direct_main.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/chiaki-ps3-direct.elf: $(BUILD)/direct_main.o $(CHIAKI_BUILD)/libchiaki-ps3-session.a $(CHIAKI_BUILD)/libchiaki-ps3-crypto.a $(CHIAKI_BUILD)/libchiaki-ps3-protobuf.a $(CHIAKI_BUILD)/libchiaki-ps3-fec.a $(CHIAKI_BUILD)/libchiaki-ps3-platform.a
	$(CC) $(CFLAGS) $< $(LIBS) -o $@
clean:
	rm -rf $(BUILD)
.PHONY: all clean
