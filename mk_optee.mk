FILEPATH=$(readlink -f "$0")
DIRPATH=$(dirname "$FILEPATH")

optee_dir="/home/test/workspace/code/optee_3.14"
dest_dir=$(dirname $(readlink -f $0))/testcase/optee

echo $DIRPATH

export PATH="$optee_dir/toolchains/aarch64/bin:$PATH"

export CROSS_COMPILE_HOST=aarch64-linux-gnu-
export CROSS_COMPILE_TA=aarch64-linux-gnu-
export CROSS_COMPILE_CA=aarch64-linux-gnu-
export TA_DEV_KIT_DIR=$DIRPATH/sdk/optee_sdk_3.12/export-ta_arm64
export ROOT_BUILD_DIR=$DIRPATH

export ARCH=arm

make -C $DIRPATH -f optee.mak V=0 clean
make -C $DIRPATH -f optee.mak V=0 all

demo_name="cata_stress pass_paramaters storage aes_auth_demo aes_demo esdsa_demo hash_demo hmac_demo persist_demo rsa_demo zhouhehe rsakey bst_faceid"

for name in $demo_name
do
	echo "Copy "$name" binaries(TA & CA) to $optee_dir/out-br/-------------------"
	cp ./out/ta/$name/*.ta $optee_dir/out-br/target/lib/optee_armtz
	cp ./out/ca/$name/${name}_demo $optee_dir/out-br/target/usr/bin
	
	echo "Copy "$name" binaries(TA & CA) to TESTCASE-------------------"
	cp ./out/ta/$name/*.ta $dest_dir/
	cp ./out/ca/$name/${name}_demo $dest_dir/
done
