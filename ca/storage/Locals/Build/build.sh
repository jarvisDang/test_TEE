#!/bin/bash -e
# Source setup.sh
setup_script=""



if [ "$MODE" == "Release" ]; then
  echo -e "Mode\t\t: Release"
  OPTIM=release
else
  echo -e "Mode\t\t: Debug"
  OPTIM=debug
fi

# go to project root
cd $(dirname $(readlink -f $0))/../..

if [[ ${TARGET_OS} == "Linux" ]]; then
    OUT_DIR=Out/Bin/$APP_ABI/$MODE
    if [ -f Locals/Code/CMakeLists.txt ]; then
        ### ---------------- Linux Build Command ----------------
        CORES=`cat /proc/cpuinfo | grep -c processor`
        BASE=$PWD
        BuildDir=Out/_build/$MODE/$APP_ABI
        mkdir -p $BuildDir
        mkdir -p $OUT_DIR/LinuxBinaries

cat > $PWD/$BuildDir/toolchain.cmake <<EOF
# CMake system name must be something like "Linux".
# This is important for cross-compiling.
set( CMAKE_SYSTEM_NAME "${SYSTEM_NAME}" )
set( CMAKE_SYSTEM_PROCESSOR ${TARGET_ARCH} )
set( CMAKE_C_COMPILER ${CROSS_GCC_PATH_GCC} )
set( CMAKE_CXX_COMPILER ${CROSS_GCC_PATH_GPP} )
set( CMAKE_ASM_COMPILER ${CROSS_GCC_PATH_GCC} )
set( CMAKE_AR ${CROSS_GCC_PATH_AR} CACHE FILEPATH "Archiver" )
set( CMAKE_C_FLAGS_RELEASE "${ARCH_FLAGS} -O2 -pipe -g -feliminate-unused-debug-types -DNDEBUG" CACHE STRING "CFLAGS for release" )
set( CMAKE_CXX_FLAGS_RELEASE "${ARCH_FLAGS} -O2 -pipe -g -feliminate-unused-debug-types -fvisibility-inlines-hidden -DNDEBUG" CACHE STRING "CXXFLAGS for release" )
set( CMAKE_ASM_FLAGS_RELEASE "${ARCH_FLAGS} -O2 -pipe -g -feliminate-unused-debug-types -DNDEBUG" CACHE STRING "ASM FLAGS for release" )
set( CMAKE_C_FLAGS_DEBUG "${ARCH_FLAGS} -O2 -pipe -g -feliminate-unused-debug-types  " CACHE STRING "CFLAGS for debug" )
set( CMAKE_CXX_FLAGS_DEBUG "${ARCH_FLAGS} -O2 -pipe -g -feliminate-unused-debug-types -fvisibility-inlines-hidden " CACHE STRING "CXXFLAGS for debug" )
set( CMAKE_ASM_FLAGS_DEBUG "${ARCH_FLAGS} -O2 -pipe -g -feliminate-unused-debug-types " CACHE STRING "ASM FLAGS for debug" )
set( CMAKE_FIND_ROOT_PATH "${CROSS_GCC_PATH_DIR} ")
set( CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY )
set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH )
set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
set(COMP_PATH_TlAsyncExample ${COMP_PATH_TlAsyncExample})
set(COMP_PATH_TlFloat ${COMP_PATH_TlFloat})
set(COMP_PATH_DrTplayDrmSkeleton ${COMP_PATH_DrTplayDrmSkeleton})
set(COMP_PATH_TACryptoCatalog ${COMP_PATH_TACryptoCatalog})
set(COMP_PATH_TASampleGP ${COMP_PATH_TASampleGP})
set(COMP_PATH_TlSdk ${COMP_PATH_TlSdk})
EOF
        cd ${BuildDir}

        cmake \
            $BASE/Locals/Code/ \
            -DCMAKE_INSTALL_PREFIX=$BASE/$OUT_DIR/LinuxBinaries \
            -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake \
            -DCOMMON_DEP_INCLUDE_DIRS=${COMP_PATH_MobiCoreClientLib_module}/Public/ \
            -DCOMMON_DEP_LIBRARIES=${COMP_PATH_MobiCoreClientLib_module}/Bin/$APP_ABI/Debug/ \
            -DCMAKE_BUILD_TYPE=$MODE \
            -Wno-dev

        make  -j $CORES

        make install
        binaries=$(find $BASE/$OUT_DIR/LinuxBinaries -type f)
        cp $binaries $BASE/$OUT_DIR
        rm -rf $BASE/$OUT_DIR/LinuxBinaries
        rm -rf *.cmake CMake* *.txt

        cd $BASE
    else
        echo "CMakeLists.txt not present"
    fi
elif [[ ${TARGET_OS} == "Android" ]]; then
    ### ---------------- Android Build Command ----------------
    # run NDK build
    ${NDK_BUILD} \
        -C Locals/Code \
        -B \
        NDK_DEBUG=1 \
        NDK_PROJECT_PATH=. \
        NDK_APPLICATION_MK=Application.mk \
        NDK_MODULE_PATH=${t_sdk_root} \
        NDK_APP_OUT=$PWD/Out/_build \
        APP_BUILD_SCRIPT=Android.mk \
        APP_OPTIM=$OPTIM

    for abi in $APP_ABI; do
        mkdir -p Out/Bin/$abi/$MODE
        cp -r $PWD/Out/_build/local/$abi/* Out/Bin/$abi/$MODE
    done
else
    echo "ERROR: TARGET_OS [$TARGET_OS] not supported"
    exit 1
fi

echo
echo "Output directory of build is $PWD/Out/Bin"
