apt-get update && apt-get install -y make python wget

git clone https://github.com/facebook/zstd
cd zstd
git checkout 9ad7ea44ec9644c618c2e82be5960d868e48745d
cd $SRC/ && tar -xf seeds.tar
cp $SRC/compile_commands.json $SRC/zstd/tests/fuzz/
cp -r $SRC/seeds $OUT
