cd libxml2
./autogen.sh
CCLD="$CXX $CXXFLAGS" ./configure --without-python --with-threads=no \
    --with-zlib=no --with-lzma=no
bear make -j $(nproc)

$CXX $CXXFLAGS -std=c++11 $SRC/target.cc -I include .libs/libxml2.a \
    $FUZZER_LIB -o $OUT/xml $LDFLAGS
