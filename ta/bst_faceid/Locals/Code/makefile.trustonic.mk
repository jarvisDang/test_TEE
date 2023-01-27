################################################################################
#
# SDK Sample GP Trusted Application
#
################################################################################


GP_ENTRYPOINTS := Y

#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------
TA_KEYFILE := Locals/Build/pairVendorTltSig.pem

TA_INSTANCES := 16

TA_OPTS += -DTATAG='"$(OUTPUT_NAME): "'

#-------------------------------------------------------------------------------

HEAP_SIZE_INIT := 16384
HEAP_SIZE_MAX := 65536

#TA_OPTS += -Werror

#------------------------------------------------------------------------------
# Encrypt the service after signature
#ENCRYPT_SERVICE_KEY := Locals/Build/ServiceEncryptionKey128.xml

#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------

### Add include path here
INCLUDE_DIRS += \
    Locals/Code/include \
    Out/Bin/Public \
    Out/Public \

LOCAL_CFLAGS += -DUSE_TRUSTONIC


HAL_LIBS += Locals/Code/libs/${PLATFORM}/libbstMatch.a

### Add source code files for C compiler here
SRC_C += \
    Locals/Code/ta_entry.c

### Add source code files for assembler here
SRC_S += # nothing

#-------------------------------------------------------------------------------
# use generic make file
TRUSTED_APP_DIR ?= Locals/Code
TASDK_DIR_SRC ?= $(TASDK_DIR)
include $(TASDK_DIR)/trusted_application.mk
