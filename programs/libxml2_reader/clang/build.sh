cd libxml2
./autogen.sh
./configure --without-python --with-threads=no --with-zlib=no --with-lzma=no
make -j$(nproc) clean
bear make -j$(nproc) all

$CXX $CXXFLAGS -std=c++11 -Iinclude/ \
    $SRC/libxml2_xml_reader_for_file_fuzzer.cc \
    -o $OUT/libxml2_xml_reader_for_file_fuzzer \
    $LIB_FUZZING_ENGINE .libs/libxml2.a $LDFLAGS
