apt-get update && apt-get install -y make cmake flex bison
git clone https://github.com/the-tcpdump-group/libpcap.git libpcap
git clone https://github.com/the-tcpdump-group/tcpdump.git tcpdump

pushd libpcap
git checkout d615abec7e0237299250c409dca23effb8dd36cc
git apply ../patch.diff
popd

pushd tcpdump
git checkout e3a00d340c8707b178b0cce017e009cfaafdd22d
popd

cp libpcap/testprogs/fuzz/fuzz_*.options $OUT/
cp -r seeds $OUT/
