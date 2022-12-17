# Fix injections for generated code wtih sysinfo.y.
sed -i '1,3d' $SRC/binutils-gdb/binutils/sysinfo.y
sed -i '22i #ifdef FRCOV\n#include <stdio.h>\nshort FIXREVERTER[7503];\n#endif\n' $SRC/binutils-gdb/binutils/sysinfo.y

export CFLAGS="$CFLAGS -fsanitize=address, -fsanitize=array-bounds,bool,builtin,enum,float-divide-by-zero,function,integer-divide-by-zero,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,unreachable,vla-bound,vptr -DFRCOV"
export CXXFLAGS="$CXXFLAGS -fsanitize=address, -fsanitize=array-bounds,bool,builtin,enum,float-divide-by-zero,function,integer-divide-by-zero,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,unreachable,vla-bound,vptr -DFRCOV"

cd binutils-gdb
cd binutils
sed -i 's/vfprintf (stderr/\/\//' elfcomm.c
sed -i 's/fprintf (stderr/\/\//' elfcomm.c
cd ../

./configure --disable-gdb --disable-gdbserver --disable-gdbsupport \
	    --disable-libdecnumber --disable-readline --disable-sim \
	    --enable-targets=all --disable-werror
make -j$(nproc)

#sed -i "s/g ->  \. addend/g -> addend/g" /src/binutils-gdb/bfd/elf32-spu.c
sed -i "s/short FIXREVERTER\[FIXREVERTER_SIZE\];/extern short FIXREVERTER\[\];/" /src/fuzz_disassemble.c

mkdir -p fuzz
cp ../fuzz_*.c fuzz/
cd fuzz

$CC $CFLAGS -I ../include -I ../bfd -I ../opcodes -c fuzz_disassemble.c -o fuzz_disassemble.o
$CXX $CXXFLAGS fuzz_disassemble.o -o $OUT/fuzz_disassemble $LIB_FUZZING_ENGINE ../opcodes/libopcodes.a ../bfd/libbfd.a ../libiberty/libiberty.a ../zlib/libz.a $LDFLAGS
