## How to setup SDK and compile TAs


#### 1、从gitlab上下载通用代码

`git clone git@gitee.com:baron_zz/test_ca_ta.git`

#### 2、配置optee SDK

在sdk目录下，可以看到已经有了一个最新版本的的optee SDK. 如果不说您想要的，您可以换成你想要的.

```c
test@test-21:~/workspace/code/application/test_ca_ta$ ls sdk/
optee_sdk_3.12
test@test-21:~/workspace/code/application/test_ca_ta$ 
```

**配置optee toolchains**

默认拉下来的代码，不包含编译工具链，需要您自行下载下面两个工具链:<br>
- gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu.tar.xz<br>
- gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf.tar.xz<br> 

然后修改mk_optee.mk中的工具链路径:
```c
(vim mk_optee.mk)
optee_dir="/home/test/workspace/code/optee"
export PATH="$optee_dir/toolchains/aarch64/bin:$PATH"
```

#### 2、配置trustonic SDK
默认拉下来的代码，不包含trustonic SDK. 如有将trustonic SDK拷贝到sdk目录下，如我使用的't-base-MTK-A64-Android-510a-V001':
```c
test@test-21:~/workspace/code/application/test_ca_ta$ ls sdk/
optee_sdk_3.12  t-base-MTK-A64-Android-510a-V001
test@test-21:~/workspace/code/application/test_ca_ta$ 
```

**配置trustonic toolchains**

默认从官方拿到的sdk，不包含编译工具链，需要您自行下载下面两个工具链:<br>
gcc-arm-9.2-2019.12-x86_64-aarch64-none-elf.tar.xz<br>
gcc-arm-9.2-2019.12-x86_64-arm-none-eabi.tar.xz<br>

然后修改setup.sh中的工具链路径:
```c
(vim t-base-MTK-A64-Android-510a-V001/setup.sh)

export CROSS_GCC64_PATH=/home/test/workspace/code/tt_sdk/toolchains/gcc-arm-9.2-2019.12-x86_64-aarch64-none-elf
export CROSS_GCC64_PATH_BIN=${CROSS_GCC64_PATH}/bin/aarch64-none-elf-

export CROSS_GCC_PATH=/home/test/workspace/code/tt_sdk/toolchains/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi
export CROSS_GCC_PATH_BIN=${CROSS_GCC_PATH}/bin/arm-none-eabi-
```

**修改sdk变量**
为了让optee和trustonic sdk编译TA更好的兼容，我们是trustonic sdk中添加了编译选项`LOCAL_CFLAGS`宏

```c
(vim ./t-base-MTK-A64-Android-510a-V001/t-base-dev-kit/t-sdk/TlSdk/trustlet.mk)

$(OUTPUT_OBJ_PATH)/%.o : %.c $(OBJDIRS)
        $(CC) $(C99) $(CC_OPTS) $(CC_OPTS2) $(INC) $(LOCAL_CFLAGS) -c -o $@ $<
```

#### 3、编译TA/CA

默认拉下的代码中，包含了一个最小的代码`zhouhehe`，编译需要:<br>
- ./mk_trustonic.mk 即可编译trustonic ca/ta<br>
- ./mk_optee.mk 即可编译trustonic ca/ta<br>

#### 4、clone一对TA/CA

敲击`./clone zhouhehe new_name` 即可clone一对CA/TA，然后再次敲击`./mk_trustonic.mk`和`./mk_optee.mk`即可完成CA和TA的编译

注意:<br>
- 编译后的CA在out/CA路径下<br>
- 编译后的trustonic TA在ta/new_name/Out/Bin/ARM_V8A_AARCH64/GNU/Debug/路径下<br>
- 编译后的optee TA在out/ta路径下<br>





# test_TEE
