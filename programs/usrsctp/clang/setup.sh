apt-get update && apt-get install -y make cmake pkg-config

git clone https://github.com/weinrank/usrsctp usrsctp
cd usrsctp
git checkout e08eacffd438cb0760c926fbe60ccda011f6ce70
cp -r fuzzer/CORPUS_CONNECT $OUT/seeds
