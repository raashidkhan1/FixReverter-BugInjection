# Roll back macro expansion to avoid error.
sed -i s/xmlStrlen__internal_alias/xmlStrlen/g $SRC/libxml2/xpath.c

export CFLAGS="$CFLAGS -fsanitize=address, -fsanitize=array-bounds,bool,builtin,enum,float-divide-by-zero,function,integer-divide-by-zero,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,unreachable,vla-bound,vptr -DFRCOV"
export CXXFLAGS="$CXXFLAGS -fsanitize=address, -fsanitize=array-bounds,bool,builtin,enum,float-divide-by-zero,function,integer-divide-by-zero,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,unreachable,vla-bound,vptr -DFRCOV"
cd libxml2
./autogen.sh
./configure --without-python --with-threads=no --with-zlib=no --with-lzma=no
make -j$(nproc) clean
make -j$(nproc) all

$CXX $CXXFLAGS -std=c++11 -Iinclude/ \
    $SRC/libxml2_xml_reader_for_file_fuzzer.cc \
    -o $OUT/libxml2_xml_reader_for_file_fuzzer \
    $LIB_FUZZING_ENGINE .libs/libxml2.a $LDFLAGS
