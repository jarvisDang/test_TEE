
include $(ROOT_BUILD_DIR)/ca/env_default.mk

OUT_DIR := $(O)

#CROSS_COMPILE   ?= #arm-linux-gnueabihf-
CC := $(CROSS_COMPILE)gcc
AR := $(CROSS_COMPILE)ar

.PHONY: all TARGET clean
all: TARGET

TARGET_NAME := zhouhehe_demo

SRCS := Locals/Code/src/zhouhehe_main.c

INCLUDES_DIR := Locals/Code/include
INCLUDES_DIR += $(ROOT_BUILD_DIR)/sdk/optee_sdk_3.12/export-ta_arm64/host_include
INCLUDES_DIR += $(ROOT_BUILD_DIR)/sdk/optee_sdk_3.12/export-ca_arm64/include

CFLAGS += $(addprefix -I, $(INCLUDES_DIR))

LFLAGS := -L$(ROOT_BUILD_DIR)/sdk/optee_sdk_3.12/export-ca_arm64/lib/share
LFLAGS += -lteec

OBJ_DIR	:= $(OUT_DIR)
OBJS := $(patsubst %.c,$(OBJ_DIR)/%.o, $(SRCS))

CA_BIN_FILE := $(OUT_DIR)/$(TARGET_NAME)

TARGET: $(CA_BIN_FILE)


$(CA_BIN_FILE): $(OBJS)
	@echo "  LINK    $@"
	$(q)$(CC) -o $@ $+ $(LFLAGS)
	@echo ""

$(OBJ_DIR)/%.o:%.c
	$(q)mkdir -p $(dir $@)
	@echo "  CC      $<"
	$(q)$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f $(OBJS) $(CA_BIN_FILE)

