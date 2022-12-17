apt-get update && \
    apt-get install -y \
    make \
    wget \
    autoconf \
    automake \
    libtool \
    libglib2.0-dev

git clone https://gitlab.gnome.org/GNOME/libxml2.git

cd libxml2

git checkout -f v2.9.2
./autogen.sh

CCLD="$CXX $CXXFLAGS" ./configure --without-python --with-threads=no \
    --with-zlib=no --with-lzma=no
make -j $(nproc)

$CXX $CXXFLAGS -std=c++11 $SRC/target.cc -I include .libs/libxml2.a \
    $FUZZER_LIB -o $SRC/fuzzer $LDFLAGS

cd ..
extract-bc fuzzer
llvm-dis fuzzer.bc
