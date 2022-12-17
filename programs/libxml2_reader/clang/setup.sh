apt-get update && apt-get install -y make autoconf automake libtool pkg-config python-dev python3-dev
git clone https://gitlab.gnome.org/GNOME/libxml2.git
cd libxml2
git checkout 99a864a1f7a9cb59865f803770d7d62fb47cad69

./autogen.sh
./configure --without-python --with-threads=no --with-zlib=no --with-lzma=no
make -j$(nproc) clean
bear make -j$(nproc) all

cp -r $SRC/seeds $OUT/
