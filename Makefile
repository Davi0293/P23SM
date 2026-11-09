# ps2_xmb_combo.sprx - needs the open-source PS3 toolchain + PSL1GHT
# Output: build/ps2_xmb_combo.sprx
#
# Recipes use ">" instead of TAB (.RECIPEPREFIX) so copy/paste or editors
# that convert tabs to spaces cannot cause "missing separator".

.RECIPEPREFIX := >

PS3DEV  ?= /usr/local/ps3dev
PSL1GHT ?= $(PS3DEV)

NAME    := ps2_xmb_combo
BUILD   := build
SRC     := src/main.c

CC      := powerpc64-ps3-elf-gcc
# -O2, not -Os: -Os triggers an ICE in this toolchain (rs6000_savres_routine_name)
CFLAGS  := -O2 -mcpu=cell -ffunction-sections -fdata-sections -Wall \
           -I$(PSL1GHT)/ppu/include -I$(PS3DEV)/ppu/include
LDFLAGS := -nostartfiles -Wl,--gc-sections -L$(PSL1GHT)/ppu/lib \
           -Wl,-e,module_start
LIBS    := -lio -lsysutil -lsysfs -llv2

SPRXLINKER := $(PS3DEV)/bin/sprxlinker
MAKE_SELF  := $(PS3DEV)/bin/make_self

all: $(BUILD)/$(NAME).sprx

$(BUILD):
> mkdir -p $(BUILD)

$(BUILD)/$(NAME).elf: $(SRC) | $(BUILD)
> $(CC) $(CFLAGS) $(SRC) $(LDFLAGS) $(LIBS) -o $@

# Fix up the import stubs, then wrap as SELF/SPRX
$(BUILD)/$(NAME).sprx: $(BUILD)/$(NAME).elf
> $(SPRXLINKER) $<
> $(MAKE_SELF) $< $@

clean:
> rm -rf $(BUILD)

.PHONY
: all clean
