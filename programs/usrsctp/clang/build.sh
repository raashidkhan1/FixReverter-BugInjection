cd usrsctp
cmake -Dsctp_werror=0 -Dsctp_build_programs=0 -Dsctp_debug=0 -Dsctp_invariants=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo .
bear make -j$(nproc)

cd fuzzer
$CC $CFLAGS -DFUZZING_STAGE=0 -I . -I ../usrsctplib/ -c fuzzer_connect.c -o $OUT/fuzzer_connect.o
$CXX $CXXFLAGS -o $OUT/fuzzer_connect $OUT/fuzzer_connect.o $LIB_FUZZING_ENGINE ../usrsctplib/libusrsctp.a $LDFLAGS
rm -f $OUT/fuzzer_connect.o
