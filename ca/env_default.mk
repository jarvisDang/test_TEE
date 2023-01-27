-include $(TA_DEV_KIT_DIR)/host_include/conf.mk

ifeq ($(CFG_ENC_FS),y)
CFLAGS += -DCFG_ENC_FS
endif
ifeq ($(CFG_REE_FS),y)
CFLAGS += -DCFG_REE_FS
endif
ifeq ($(CFG_RPMB_FS),y)
CFLAGS += -DCFG_RPMB_FS
endif

CFLAGS += -DUSER_SPACE
CFLAGS += -DTA_DIR=\"/usr/lib/optee_armtz\"
CFLAGS += -pthread
CFLAGS += -g3

