apt-get update && apt-get install -y make cmake pkg-config
git clone https://github.com/weinrank/usrsctp usrsctp
cd usrsctp
git checkout e08eacffd438cb0760c926fbe60ccda011f6ce70

cmake -DCMAKE_C_FLAGS="-g -O0" -Dsctp_build_programs=0 -Dsctp_debug=0 -Dsctp_invariants=1 .
make -j$(nproc)
cd fuzzer

$CC $CFLAGS -DFUZZING_STAGE=0 -I . -I ../usrsctplib/ -c fuzzer_connect.c -o $OUT/fuzzer_connect.o
$CXX $CXXFLAGS -o $SRC/fuzzer $OUT/fuzzer_connect.o $LIB_FUZZING_ENGINE ../usrsctplib/libusrsctp.a $LDFLAGS
rm -f $OUT/fuzzer_connect.o

cd $SRC
extract-bc fuzzer
llvm-dis fuzzer.bc
