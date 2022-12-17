apt-get update && \
    apt-get install -y \
    make \
    automake \
    libtool \
    wget
git clone https://github.com/mm2/Little-CMS.git
cd Little-CMS
git checkout f9d75ccef0b54c9f4167d95088d4727985133c52
./autogen.sh
./configure
make -j $(nproc)

$CXX $CXXFLAGS $SRC/cms_transform_fuzzer.cc -I include/ src/.libs/liblcms2.a \
    $FUZZER_LIB -o $SRC/fuzzer $LDFLAGS

cd $SRC
extract-bc fuzzer
llvm-dis fuzzer.bc
