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

cp -r $SRC/seeds $OUT/
