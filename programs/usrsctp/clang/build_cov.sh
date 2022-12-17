# Fix an injection messed up by Clang AST which mistakenly recognizes a single statement as a compound statement.
sed -i '1458d' $SRC/usrsctp/usrsctplib/netinet/sctp_timer.c
sed -i '1463d' $SRC/usrsctp/usrsctplib/netinet/sctp_timer.c
sed -i '1463i \\t\t\t\tsctp_ucount_incr(stcb->asoc.sent_queue_retran_cnt);' $SRC/usrsctp/usrsctplib/netinet/sctp_timer.c

export CFLAGS="$CFLAGS -fsanitize=address, -fsanitize=array-bounds,bool,builtin,enum,float-divide-by-zero,function,integer-divide-by-zero,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,unreachable,vla-bound,vptr -DFRCOV"
export CXXFLAGS="$CXXFLAGS -fsanitize=address, -fsanitize=array-bounds,bool,builtin,enum,float-divide-by-zero,function,integer-divide-by-zero,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,unreachable,vla-bound,vptr -DFRCOV"

cd usrsctp
cmake -Dsctp_werror=0 -Dsctp_build_programs=0 -Dsctp_debug=0 -Dsctp_invariants=1 -DCMAKE_C_FLAGS=-DFRCOV -DCMAKE_CXX_FLAGS=-DFRCOV -DCMAKE_BUILD_TYPE=RelWithDebInfo .
VERBOSE=1 make -j$(nproc)

cd fuzzer
$CC $CFLAGS -DFUZZING_STAGE=0 -I . -I ../usrsctplib/ -c fuzzer_connect.c -o $OUT/fuzzer_connect.o
$CXX $CXXFLAGS -o $OUT/fuzzer_connect $OUT/fuzzer_connect.o $LIB_FUZZING_ENGINE ../usrsctplib/libusrsctp.a $LDFLAGS
rm -f $OUT/fuzzer_connect.o
