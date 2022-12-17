apt-get update && apt-get install -y make autoconf autogen texinfo flex bison

git clone --recursive git://sourceware.org/git/binutils-gdb.git binutils-gdb
cd binutils-gdb
git checkout 7c96e6120f1b9b5025629bbe995ca55d1be8f36f
git apply $SRC/pre_build.patch
cp $SRC/compile_commands.json $SRC/binutils-gdb
cp -r $SRC/seeds $OUT/

sed -i "s/codes\.insert(0, b'#include <stdio\.h>\\\n')//" /fixreverter/FixReverter/drivers/inject/inject.py

