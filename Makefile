NAME := tcp_shell
SDK_DIR := /home/orayn/lede-sdk-17.01.7
TOOLCHAIN_DIR := $(SDK_DIR)/staging_dir/toolchain-mips_24kc_gcc-5.4.0_musl-1.1.16
TARGET_CC := $(TOOLCHAIN_DIR)/bin/mips-openwrt-linux-gcc

# Don’t add -I for system headers unless absolutely needed
CFLAGS := -Os -Wall
LDFLAGS := 

all: $(NAME)

$(NAME): $(NAME).c
	$(TARGET_CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(NAME)
