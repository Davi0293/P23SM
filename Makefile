# ps2_xmb_combo.sprx - needs the open-source PS3 toolchain + PSL1GHT
#   export PS3DEV=/usr/local/ps3dev
#   export PSL1GHT=$(PS3DEV)
#   export PATH=$PATH:$PS3DEV/bin:$PS3DEV/ppu/bin
#
# Output: build/ps2_xmb_combo.sprx

PS3DEV  ?= /usr/local/ps3dev
PSL1GHT ?= $(PS3DEV)

NAME    := ps2_xmb_combo
BUILD   := build
SRC     := src/main.c

# Current ps3toolchain names its compiler powerpc64-ps3-elf-gcc
# (ppu-lv2-gcc was the name in older/Sony-style toolchains).
CC      ?= powerpc64-ps3-elf-gcc
CFLAGS  := -Os -mcpu=cell -ffunction-sections -fdata-sections -Wall \
           -I$(PSL1GHT)/ppu/include -I$(PS3DEV)/ppu/include
LDFLAGS := -nostartfiles -Wl,--gc-sections -L$(PSL1GHT)/ppu/lib \
           -Wl,-e,module_start
LIBS    := -lio -lsysutil -llv2

SPRXLINKER := $(PS3DEV)/bin/sprxlinker
MAKE_SELF  := $(PS3DEV)/bin/make_self

all: $(BUILD)/$(NAME).sprx

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/$(NAME).elf: $(SRC) | $(BUILD)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) $(LIBS) -o $@

# Fix up the import stubs, then wrap as an unsigned-style SELF/SPRX
$(BUILD)/$(NAME).sprx: $(BUILD)/$(NAME).elf
	$(SPRXLINKER) $<
	$(MAKE_SELF) $< $@

clean:
	rm -rf $(BUILD)

.PHONY: all clean
