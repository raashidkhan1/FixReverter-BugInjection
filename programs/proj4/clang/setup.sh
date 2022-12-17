apt-get update && \
    apt-get install -y \
    make \
    automake \
    autoconf \
    libtool \
    sqlite3 \
    wget \
    libsqlite3-dev

git clone https://github.com/OSGeo/PROJ
cd PROJ
git checkout d00501750b210a73f9fb107ac97a683d4e3d8e7a

mkdir $OUT/seeds
cp nad/* $OUT/seeds

wget -qO $OUT/fuzz-target.dict \
    https://raw.githubusercontent.com/google/fuzzing/master/dictionaries/proj4.dict
