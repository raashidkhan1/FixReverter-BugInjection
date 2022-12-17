apt-get update && apt-get install -y make cmake flex bison libnl-3-dev libnl-genl-3-dev
git clone https://github.com/the-tcpdump-group/libpcap.git libpcap
git clone https://github.com/the-tcpdump-group/tcpdump.git tcpdump

pushd tcpdump
git checkout e3a00d340c8707b178b0cce017e009cfaafdd22d
popd

cd libpcap
git checkout d615abec7e0237299250c409dca23effb8dd36cc
git apply ../patch.diff
mkdir build
cd build
cmake ..
make

$CC $CFLAGS -I.. -c ../testprogs/fuzz/fuzz_both.c -o fuzz_both.o
$CXX $CXXFLAGS fuzz_both.o -o $SRC/fuzzer libpcap.a $LIB_FUZZING_ENGINE $LDFLAGS -lnl-3 -lnl-genl-3 -libverbs

cd $SRC
extract-bc fuzzer
llvm-dis fuzzer.bc
