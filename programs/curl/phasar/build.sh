apt-get install -y libtool autoconf pkg-config

git clone https://github.com/curl/curl.git /src/curl
cd curl
git checkout 376d5bb323c03c0fc4af266c03abac8f067fbd0e
cd ..
git clone https://github.com/curl/curl-fuzzer.git /src/curl_fuzzer
git -C /src/curl_fuzzer checkout -f 9a48d437484b5ad5f2a97c0cab0d8bcbb5d058de

# Update zlib link.
rm $SRC/curl_fuzzer/scripts/download_zlib.sh
chmod +x $SRC/download_zlib.sh
cp $SRC/download_zlib.sh $SRC/curl_fuzzer/scripts/download_zlib.sh

$SRC/curl_fuzzer/scripts/ossfuzzdeps.sh
cd $SRC/curl_fuzzer

./ossfuzz.sh
cp /out/curl_fuzzer_http $SRC/fuzzer

cd $SRC
extract-bc fuzzer
llvm-dis fuzzer.bc
