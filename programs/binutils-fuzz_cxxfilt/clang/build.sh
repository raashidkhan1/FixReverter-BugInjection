cd binutils-gdb
cd binutils
sed -i 's/vfprintf (stderr/\/\//' elfcomm.c
sed -i 's/fprintf (stderr/\/\//' elfcomm.c
cd ../

./configure --disable-gdb --disable-gdbserver --disable-gdbsupport \
	    --disable-libdecnumber --disable-readline --disable-sim \
	    --enable-targets=all --disable-werror

sed -i "s/g ->  \. addend/g -> addend/g" /src/binutils-gdb/bfd/elf32-spu.c
sed -i "s/short FIXREVERTER\[FIXREVERTER_SIZE\];/extern short FIXREVERTER\[\];/" $SRC/binutils-gdb/binutils/fuzz_cxxfilt.c

make -j$(nproc)

cd binutils

# Modify main functions so we dont have them anymore
sed 's/main (int argc/old_main (int argc, char **argv);\nint old_main (int argc/' cxxfilt.c > cxxfilt.h

# Compile object file
$CC $CFLAGS -DHAVE_CONFIG_H -I. -I../bfd -I./../bfd -I./../include -I./../zlib -DLOCALEDIR="\"/usr/local/share/locale\"" -Dbin_dummy_emulation=bin_vanilla_emulation -W -Wall -MT fuzz_cxxfilt.o -MD -MP -c -o fuzz_cxxfilt.o fuzz_cxxfilt.c

# Link the files
## cxxfilt
$CXX $CXXFLAGS $LIB_FUZZING_ENGINE -W -Wall -Wstrict-prototypes -Wmissing-prototypes -Wshadow -I./../zlib -o fuzz_cxxfilt fuzz_cxxfilt.o bucomm.o version.o filemode.o ../bfd/.libs/libbfd.a -L/src/binutils-gdb/zlib -lpthread -ldl -lz ../libiberty/libiberty.a
mv fuzz_cxxfilt $OUT/fuzz_cxxfilt
