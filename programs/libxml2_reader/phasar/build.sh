apt-get update && apt-get install -y make autoconf automake libtool pkg-config python-dev python3-dev
git clone https://gitlab.gnome.org/GNOME/libxml2.git
cd libxml2
git checkout 99a864a1f7a9cb59865f803770d7d62fb47cad69

./autogen.sh
./configure --without-python --with-threads=no --with-zlib=no --with-lzma=no
make -j$(nproc) clean
make -j$(nproc) all

$CXX $CXXFLAGS -std=c++11 -Iinclude/ \
    $SRC/libxml2_xml_reader_for_file_fuzzer.cc \
    -o $SRC/fuzzer \
    $LIB_FUZZING_ENGINE .libs/libxml2.a \
    $LDFLAGS

cd $SRC
extract-bc fuzzer
llvm-dis fuzzer.bc
