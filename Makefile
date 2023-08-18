# If you would like to choose a different path to the SDK, you can pass it as an
# argument.
ifndef SEL4CP_SDK
	SEL4CP_SDK := ../sel4cp-sdk-1.2.6
endif

SHELL=/bin/bash
# In case the default compiler triple doesn't work for you or your package manager
# only has aarch64-none-elf or something, you can specifiy the toolchain.
ifndef TOOLCHAIN
	# Get whether the common toolchain triples exist
	TOOLCHAIN_AARCH64_LINUX_GNU := $(shell command -v aarch64-linux-gnu-gcc 2> /dev/null)
	TOOLCHAIN_AARCH64_UNKNOWN_LINUX_GNU := $(shell command -v aarch64-unknown-linux-gnu-gcc 2> /dev/null)
	# Then check if they are defined and select the appropriate one
	ifdef TOOLCHAIN_AARCH64_LINUX_GNU
		TOOLCHAIN := aarch64-linux-gnu
	else ifdef TOOLCHAIN_AARCH64_UNKNOWN_LINUX_GNU
		TOOLCHAIN := aarch64-unknown-linux-gnu
	else
		$(error "Could not find an AArch64 cross-compiler")
	endif
endif

BOARD := qemu_arm_virt
SEL4CP_CONFIG := debug
BUILD_DIR := build

CPU := cortex-a53

CC := $(TOOLCHAIN)-gcc
LD := $(TOOLCHAIN)-ld
AS := $(TOOLCHAIN)-as
SEL4CP_TOOL ?= $(SEL4CP_SDK)/bin/sel4cp

PRINTF_OBJS := printf.o util.o
HELLO_WORLD_OBJS := $(PRINTF_OBJS) hello_world.o

BOARD_DIR := $(SEL4CP_SDK)/board/$(BOARD)/$(SEL4CP_CONFIG)

IMAGES := hello_world.elf
# Note that these warnings being disabled is to avoid compilation errors while in the middle of completing each exercise part
CFLAGS := -mcpu=$(CPU) -mstrict-align -nostdlib -ffreestanding -g3 -Wall -Wno-array-bounds -Wno-unused-variable -Wno-unused-function -Werror -I$(BOARD_DIR)/include -Iinclude -DBOARD_$(BOARD)
LDFLAGS := -L$(BOARD_DIR)/lib
LIBS := -lsel4cp -Tsel4cp.ld

IMAGE_FILE_HW = $(BUILD_DIR)/hello_world.img
IMAGE_FILE = $(BUILD_DIR)/loader.img
REPORT_FILE = $(BUILD_DIR)/report.txt

all: directories $(IMAGE_FILE)

directories:
	$(info $(shell mkdir -p $(BUILD_DIR)))

run: $(IMAGE_FILE)
	qemu-system-aarch64 \
   	-machine virt,virtualization=on \
   	-cpu cortex-a53 \
   	-m size=2048M \
   	-nographic \
   	-serial mon:stdio \
   	-device loader,file=$(IMAGE_FILE),addr=0x70000000,cpu-num=0

system: directories $(BUILD_DIR)/hello_world.elf $(IMAGE_FILE_HW)

$(BUILD_DIR)/%.o: %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@


$(BUILD_DIR)/hello_world.elf: $(addprefix $(BUILD_DIR)/, $(HELLO_WORLD_OBJS))
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

$(IMAGE_FILE_HW): $(addprefix $(BUILD_DIR)/, $(IMAGES)) kgdb_dev.system
	$(SEL4CP_TOOL) kgdb_dev.system --search-path $(BUILD_DIR) --board $(BOARD) --config $(SEL4CP_CONFIG) -o $(IMAGE_FILE) -r $(REPORT_FILE)
