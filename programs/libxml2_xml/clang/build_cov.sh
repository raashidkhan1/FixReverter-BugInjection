export CFLAGS="$CFLAGS -fsanitize=address, -fsanitize=array-bounds,bool,builtin,enum,float-divide-by-zero,function,integer-divide-by-zero,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,unreachable,vla-bound,vptr -DFRCOV"
export CXXFLAGS="$CXXFLAGS -fsanitize=address, -fsanitize=array-bounds,bool,builtin,enum,float-divide-by-zero,function,integer-divide-by-zero,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,unreachable,vla-bound,vptr -DFRCOV"

cd libxml2
./autogen.sh
CCLD="$CXX $CXXFLAGS" ./configure --without-python --with-threads=no \
    --with-zlib=no --with-lzma=no
bear make -j $(nproc)

$CXX $CXXFLAGS -std=c++11 $SRC/target.cc -I include .libs/libxml2.a \
    $FUZZER_LIB -o $OUT/xml $LDFLAGS
