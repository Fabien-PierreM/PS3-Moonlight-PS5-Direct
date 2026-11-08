# First cross-compiled libchiaki building block for the direct PS3 port.
# This intentionally contains only the upstream platform primitives. Session
# sources are added after this target is portable and tested with PSL1GHT.

CHIAKI_ROOT := ..
PROJECT_ROOT := ../../..
-include sources.mk
PS3DEV ?= $(abspath $(PROJECT_ROOT)/ps3dev)

CC := $(PS3DEV)/ppu/bin/ppu-gcc
AR := $(PS3DEV)/ppu/bin/ppu-ar
RANLIB := $(PS3DEV)/ppu/bin/ppu-ranlib

BUILD_DIR ?= $(abspath $(PROJECT_ROOT)/build/chiaki-ps3)
PLATFORM_LIBRARY := $(BUILD_DIR)/libchiaki-ps3-platform.a
FEC_LIBRARY := $(BUILD_DIR)/libchiaki-ps3-fec.a
PROTOBUF_LIBRARY := $(BUILD_DIR)/libchiaki-ps3-protobuf.a
CRYPTO_LIBRARY := $(BUILD_DIR)/libchiaki-ps3-crypto.a
SESSION_LIBRARY := $(BUILD_DIR)/libchiaki-ps3-session.a
PROTO_DIR := $(BUILD_DIR)/generated
TAKION_PB := $(PROTO_DIR)/takion.pb
TAKION_PB_C := $(PROTO_DIR)/takion.pb.c
TAKION_PB_H := $(PROTO_DIR)/takion.pb.h

PLATFORM_SOURCES := \
	lib/src/sock.c \
	lib/src/log.c \
	$(CURDIR)/src/thread.c \
	$(CURDIR)/src/time.c
PLATFORM_OBJECTS := $(BUILD_DIR)/platform/sock.o $(BUILD_DIR)/platform/log.o \
	$(BUILD_DIR)/platform/thread.o $(BUILD_DIR)/platform/time.o

GF_SOURCES := \
	third-party/gf-complete/src/gf.c \
	third-party/gf-complete/src/gf_wgen.c \
	third-party/gf-complete/src/gf_w4.c \
	third-party/gf-complete/src/gf_w8.c \
	third-party/gf-complete/src/gf_w16.c \
	third-party/gf-complete/src/gf_w32.c \
	third-party/gf-complete/src/gf_w64.c \
	third-party/gf-complete/src/gf_w128.c \
	third-party/gf-complete/src/gf_rand.c \
	third-party/gf-complete/src/gf_general.c \
	third-party/gf-complete/src/gf_cpu.c
JERASURE_SOURCES := \
	third-party/jerasure/src/galois.c \
	third-party/jerasure/src/jerasure.c \
	third-party/jerasure/src/reed_sol.c \
	third-party/jerasure/src/cauchy.c \
	third-party/jerasure/src/liberation.c
GF_OBJECTS := $(patsubst third-party/gf-complete/src/%.c,$(BUILD_DIR)/gf-complete/%.o,$(GF_SOURCES))
JERASURE_OBJECTS := $(patsubst third-party/jerasure/src/%.c,$(BUILD_DIR)/jerasure/%.o,$(JERASURE_SOURCES))
FEC_OBJECT := $(BUILD_DIR)/fec.o

NANOPB_SOURCES := \
	third-party/nanopb/pb_common.c \
	third-party/nanopb/pb_encode.c \
	third-party/nanopb/pb_decode.c
NANOPB_OBJECTS := $(patsubst third-party/nanopb/%.c,$(BUILD_DIR)/nanopb/%.o,$(NANOPB_SOURCES))
PROTOBUF_OBJECTS := $(NANOPB_OBJECTS) $(BUILD_DIR)/generated/takion.pb.o

CRYPTO_SOURCES := \
	lib/src/rpcrypt.c \
	lib/src/gkcrypt.c \
	$(CURDIR)/src/random.c \
	$(CURDIR)/src/ecdh.c
CRYPTO_OBJECTS := \
	$(BUILD_DIR)/crypto/rpcrypt.o \
	$(BUILD_DIR)/crypto/gkcrypt.o \
	$(BUILD_DIR)/crypto/random.o \
	$(BUILD_DIR)/crypto/ecdh.o

SESSION_SOURCES := \
	lib/src/common.c lib/src/base64.c lib/src/http.c lib/src/ctrl.c \
	lib/src/takion.c lib/src/senkusha.c lib/src/streamconnection.c \
	lib/src/aia.c lib/src/audio.c lib/src/audioreceiver.c lib/src/regist.c \
	lib/src/remote/rudp.c lib/src/remote/rudpsendbuffer.c \
	lib/src/launchspec.c lib/src/stoppipe.c lib/src/reorderqueue.c \
	lib/src/feedback.c lib/src/feedbacksender.c lib/src/controller.c \
	lib/src/takionsendbuffer.c lib/src/videoreceiver.c \
	lib/src/frameprocessor.c lib/src/packetstats.c lib/src/congestioncontrol.c \
	lib/src/bitstream.c lib/src/session.c
SESSION_OBJECTS := $(patsubst lib/src/%.c,$(BUILD_DIR)/session/%.o,$(SESSION_SOURCES)) $(BUILD_DIR)/session/netdb.o $(BUILD_DIR)/session/holepunch_stub.o $(BUILD_DIR)/session/socketpair.o
$(SESSION_OBJECTS): $(TAKION_PB_C) $(TAKION_PB_H)

CPPFLAGS := -include chiaki/config.h -I$(CURDIR)/include -I$(BUILD_DIR)/generated -I$(PS3DEV)/ppu/include -I$(PS3DEV)/portlibs/ppu/include -I$(CHIAKI_ROOT)/lib/include \
	-I$(CHIAKI_ROOT)/third-party/gf-complete/include -I$(CHIAKI_ROOT)/third-party/jerasure/include \
	-I$(CHIAKI_ROOT)/third-party/nanopb -I$(PROTO_DIR)
CFLAGS := -mcpu=cell -mstrict-align -O2 -Wall -Wextra -Werror=implicit-function-declaration $(CPPFLAGS)

.PHONY: all platform fec protobuf crypto session clean

all: platform fec protobuf crypto session
platform: $(PLATFORM_LIBRARY)
fec: $(FEC_LIBRARY)
protobuf: $(PROTOBUF_LIBRARY)
crypto: $(CRYPTO_LIBRARY)
session: $(SESSION_LIBRARY)

$(PLATFORM_LIBRARY): $(PLATFORM_OBJECTS)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^
	$(RANLIB) $@

$(FEC_LIBRARY): $(FEC_OBJECT) $(GF_OBJECTS) $(JERASURE_OBJECTS)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^
	$(RANLIB) $@

$(PROTOBUF_LIBRARY): $(PROTOBUF_OBJECTS)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^
	$(RANLIB) $@

$(CRYPTO_LIBRARY): $(CRYPTO_OBJECTS)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^
	$(RANLIB) $@

$(SESSION_LIBRARY): $(SESSION_OBJECTS) $(PROTOBUF_LIBRARY)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^
	$(RANLIB) $@

$(BUILD_DIR)/platform/sock.o: $(CHIAKI_ROOT)/lib/src/sock.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/platform/log.o: $(CHIAKI_ROOT)/lib/src/log.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/platform/thread.o: $(CURDIR)/src/thread.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/platform/time.o: $(CURDIR)/src/time.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/gf-complete/%.o: $(CHIAKI_ROOT)/third-party/gf-complete/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/jerasure/%.o: $(CHIAKI_ROOT)/third-party/jerasure/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(FEC_OBJECT): $(CHIAKI_ROOT)/lib/src/fec.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(TAKION_PB): $(CHIAKI_ROOT)/lib/protobuf/takion.proto
	@mkdir -p $(dir $@)
	protoc -o$@ $< -I$(CHIAKI_ROOT)/lib/protobuf

$(TAKION_PB_C) $(TAKION_PB_H): $(TAKION_PB)
	python3 $(CHIAKI_ROOT)/third-party/nanopb/generator/nanopb_generator.py -D $(PROTO_DIR) $<

$(BUILD_DIR)/nanopb/%.o: $(CHIAKI_ROOT)/third-party/nanopb/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/generated/takion.pb.o: $(TAKION_PB_C) $(TAKION_PB_H)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $(TAKION_PB_C) -o $@

$(BUILD_DIR)/crypto/rpcrypt.o: $(CHIAKI_ROOT)/lib/src/rpcrypt.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/crypto/gkcrypt.o: $(CHIAKI_ROOT)/lib/src/gkcrypt.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/crypto/random.o: $(CURDIR)/src/random.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/crypto/ecdh.o: $(CURDIR)/src/ecdh.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/session/%.o: $(CHIAKI_ROOT)/lib/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/session/netdb.o: $(CURDIR)/src/netdb.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/session/holepunch_stub.o: $(CURDIR)/src/holepunch_stub.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/session/socketpair.o: $(CURDIR)/src/socketpair.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)/platform $(BUILD_DIR)/gf-complete $(BUILD_DIR)/jerasure $(BUILD_DIR)/nanopb $(BUILD_DIR)/crypto $(BUILD_DIR)/session $(PROTO_DIR) $(PLATFORM_LIBRARY) $(FEC_LIBRARY) $(PROTOBUF_LIBRARY) $(CRYPTO_LIBRARY) $(SESSION_LIBRARY)
