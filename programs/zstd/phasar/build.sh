apt-get update && apt-get install -y make python wget
git clone https://github.com/facebook/zstd
cd zstd
git checkout 9ad7ea44ec9644c618c2e82be5960d868e48745d
cd tests/fuzz
./fuzz.py build stream_decompress --debug=0
cp stream_decompress /src/fuzzer
cd $SRC
extract-bc fuzzer
llvm-dis fuzzer.bc
