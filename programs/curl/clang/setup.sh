git clone https://github.com/curl/curl.git /src/curl
git -C /src/curl checkout -f 376d5bb323c03c0fc4af266c03abac8f067fbd0e
# Disable style check for injected codes.
git -C /src/curl apply /src/checksrc.patch

git clone https://github.com/curl/curl-fuzzer.git /src/curl_fuzzer
git -C /src/curl_fuzzer checkout -f 9a48d437484b5ad5f2a97c0cab0d8bcbb5d058de
# Disable data check for injected fuzzing target, as it relies on env variable FIXREVERTER to run.
git -C /src/curl_fuzzer apply /src/check_data.patch

# Update zlib link.
rm $SRC/curl_fuzzer/scripts/download_zlib.sh
chmod +x $SRC/download_zlib.sh
cp $SRC/download_zlib.sh $SRC/curl_fuzzer/scripts/download_zlib.sh

$SRC/curl_fuzzer/scripts/ossfuzzdeps.sh

export BUILD_ROOT=/src/curl_fuzzer
SCRIPTDIR=${BUILD_ROOT}/scripts

# Make an install directory
export INSTALLDIR=/src/curl_install

ZLIBDIR=/src/zlib
OPENSSLDIR=/src/openssl
NGHTTPDIR=/src/nghttp2

# Install zlib
${SCRIPTDIR}/handle_x.sh zlib ${ZLIBDIR} ${INSTALLDIR} || exit 1

# For the memory sanitizer build, turn off OpenSSL as it causes bugs we can't
# affect (see 16697, 17624)
if [[ ${SANITIZER} != "memory" ]]
then
    # Install openssl
    export OPENSSLFLAGS="-fno-sanitize=alignment"
    ${SCRIPTDIR}/handle_x.sh openssl ${OPENSSLDIR} ${INSTALLDIR} || exit 1
fi

# Install nghttp2
${SCRIPTDIR}/handle_x.sh nghttp2 ${NGHTTPDIR} ${INSTALLDIR} || exit 1
