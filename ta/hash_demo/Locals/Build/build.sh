#!/bin/bash -eu

cd $(dirname $(readlink -f $0))
cd ../..

if [ ! -d Locals ]; then
  exit 1
fi

export TLSDK_DIR_SRC=${COMP_PATH_TlSdk}
export TLSDK_DIR=${COMP_PATH_TlSdk}
export TASDK_DIR_SRC=${COMP_PATH_TlSdk}
export TASDK_DIR=${COMP_PATH_TlSdk}
mkdir -p Out/Public
cp -f \
	Locals/Code/trustonic_include/* \
	Out/Public/
echo "Running make..."

export OUTPUT_NAME=hash_demo
export TA_UUID=643bfeed9962ca0a4745c6460f082f84
export GP_TA_CONFIG_FILE=Locals/Code/properties_multi_session.xml

make -f Locals/Code/makefile.trustonic.mk "$@"

export OUTPUT_NAME=hash_demo
export TA_UUID=643bfeed9962ca0a4745c6460f082f84
#export GP_TA_CONFIG_FILE=Locals/Code/properties_multi_instance.xml
#
#make -f Locals/Code/makefile.trustonic.mk "$@"
