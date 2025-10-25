NAME := 3020-keyboard

# Paths
PROJECT_DIR := $(CURDIR)
SDK_DIR := $(PROJECT_DIR)/lede-sdk-17.01.7
TOOLCHAIN_DIR := $(SDK_DIR)/staging_dir/toolchain-mips_24kc_gcc-5.4.0_musl-1.1.16

# Toolchain
TARGET_CC := $(TOOLCHAIN_DIR)/bin/mips-openwrt-linux-gcc

# Directories
SRC_DIR := $(PROJECT_DIR)/src
INC_DIR := $(PROJECT_DIR)/include
BUILD_DIR := $(PROJECT_DIR)/build

# Flags
CFLAGS := -Os -Wall -I$(INC_DIR)
LDFLAGS := 

# Sources & objects
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Target
TARGET := $(BUILD_DIR)/$(NAME)

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS)
	$(TARGET_CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(TARGET_CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR)
