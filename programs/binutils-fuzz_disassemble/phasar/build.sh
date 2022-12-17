apt-get update && apt-get install -y make autoconf autogen texinfo flex bison

git clone --recursive git://sourceware.org/git/binutils-gdb.git binutils-gdb
cd binutils-gdb
git checkout 7c96e6120f1b9b5025629bbe995ca55d1be8f36f

./configure --disable-gdb --disable-gdbserver --disable-gdbsupport \
	    --disable-libdecnumber --disable-readline --disable-sim \
	    --enable-targets=all --disable-werror
make -j$(nproc)

mkdir fuzz
cp ../fuzz_*.c fuzz/
cd fuzz

$CC $CFLAGS -I ../include -I ../bfd -I ../opcodes -c fuzz_disassemble.c -o fuzz_disassemble.o
$CXX $CXXFLAGS fuzz_disassemble.o -o $SRC/fuzzer $LIB_FUZZING_ENGINE ../opcodes/libopcodes.a ../bfd/libbfd.a ../libiberty/libiberty.a ../zlib/libz.a $LDFLAGS

cd $SRC
extract-bc fuzzer
llvm-dis fuzzer.bc
