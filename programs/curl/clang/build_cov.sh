cd curl_fuzzer
export BUILD_ROOT=$PWD
SCRIPTDIR=${BUILD_ROOT}/scripts

. ${SCRIPTDIR}/fuzz_targets

export CFLAGS="$CFLAGS -fsanitize=address, -fsanitize=array-bounds,bool,builtin,enum,float-divide-by-zero,function,integer-divide-by-zero,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,unreachable,vla-bound,vptr -DFRCOV"
export CXXFLAGS="$CXXFLAGS -fsanitize=address, -fsanitize=array-bounds,bool,builtin,enum,float-divide-by-zero,function,integer-divide-by-zero,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,unreachable,vla-bound,vptr -DFRCOV"

echo "CC: $CC"
echo "CXX: $CXX"
echo "LIB_FUZZING_ENGINE: $FUZZER_LIB"
echo "CFLAGS: $CFLAGS"
echo "CXXFLAGS: $CXXFLAGS"
echo "ARCHITECTURE: $ARCHITECTURE"
echo "FUZZ_TARGETS: $FUZZ_TARGETS"

export MAKEFLAGS+="-j$(nproc)"

# Make an install directory
export INSTALLDIR=/src/curl_install

# Compile curl
$SRC/install_curl.sh /src/curl ${INSTALLDIR}

# Build the fuzzers.
${SCRIPTDIR}/compile_fuzzer.sh ${INSTALLDIR}
