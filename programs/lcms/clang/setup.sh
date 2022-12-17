apt-get update && \
    apt-get install -y \
    make \
    automake \
    libtool \
    wget

git clone https://github.com/mm2/Little-CMS.git

pushd Little-CMS
git checkout f9d75ccef0b54c9f4167d95088d4727985133c52
popd

wget -qO $OUT/fuzz-target.dict \
    https://raw.githubusercontent.com/google/fuzzing/master/dictionaries/icc.dict

cp -r seeds $OUT/
