FROM gcr.io/oss-fuzz-base/base-builder@sha256:d4b003943394400d2d60616a2f35b8f6fc572c45aa0430f907ae773a027ec2bc

ARG PHASAR=/phasar
ARG PLUGIN=/phasar-plugin
ARG PHASAR_INSTALL_DIR=/usr/local/phasar
ARG FR_INCLUDE=/fixreverter/include
ARG LLVM_INSTALL_DIR=/usr/local/llvm-10
COPY include/libFuzzingEngine.a /usr/lib/libFuzzingEngine.a
ADD include/llvm-10 ${LLVM_INSTALL_DIR}

RUN mkdir -p ${PHASAR}
RUN mkdir -p ${PLUGIN}

ENV CC="/usr/local/llvm-10/bin/clang" \
CXX="/usr/local/llvm-10/bin/clang++" \
CFLAGS="" \
CXXFLAGS="-stdlib=libc++"

RUN apt-get update && \
apt-get install wget sudo -y && \
apt-get install git make cmake -y && \
apt-get install zlib1g-dev sqlite3 libsqlite3-dev bear python3 doxygen graphviz python3-pip libxml2 libxml2-dev libncurses5-dev libncursesw5-dev swig build-essential g++ cmake libz3-dev libedit-dev python3-sphinx libomp-dev libcurl4-openssl-dev ninja-build -y

WORKDIR /
RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.66.0/source/boost_1_66_0.tar.bz2
RUN tar --bzip2 -xf boost_1_66_0.tar.bz2
WORKDIR /boost_1_66_0
RUN ./bootstrap.sh --with-libraries=graph,program_options,log,system,filesystem,container --with-toolset=clang
RUN ./b2 install toolset=clang cxxflags="-stdlib=libc++" linkflags="-stdlib=libc++"

RUN git clone https://github.com/secure-software-engineering/phasar.git ${PHASAR}
WORKDIR ${PHASAR}
RUN git checkout febddffe9e2ca98b4587e3ed4298dd02c1adda0e
RUN git submodule init && git submodule update

RUN mkdir -p ${PHASAR}/build
WORKDIR ${PHASAR}/build
RUN export PATH=${LLVM_INSTALL_DIR}:$PATH
RUN cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
RUN cmake --build .
RUN cmake -DCMAKE_INSTALL_PREFIX=${PHASAR_INSTALL_DIR} -P cmake_install.cmake
RUN ldconfig
RUN ${PHASAR}/utils/setEnvironmentVariables.sh ${LLVM_INSTALL_DIR} ${PHASAR_INSTALL_DIR}

#RUN mkdir -p ${PHASAR}/external/json/build
#WORKDIR ${PHASAR}/external/json/build
#RUN cmake .. && make -j20 && make install
#
#RUN mkdir -p ${PHASAR}/external/json-schema-validator/build
#WORKDIR ${PHASAR}/external/json-schema-validator/build
#RUN cmake .. && make -j20 && make install
#RUN ldconfig

COPY AstPatternParser ${PLUGIN}/AstPatternParser
COPY CodeMatcher ${PLUGIN}/CodeMatcher
COPY DataHolders ${PLUGIN}/DataHolders
COPY FlowFunctions ${PLUGIN}/FlowFunctions
COPY OutputWriter ${PLUGIN}/OutputWriter
COPY Utils ${PLUGIN}/Utils
ADD CMakeLists.txt DependenceAnalyzer.cpp DependenceAnalyzer.h ${PLUGIN}/

RUN mkdir -p ${PLUGIN}/build
WORKDIR ${PLUGIN}/build
RUN cmake ..
RUN make

ARG FUNC_PRINTER=/funcPrinter
COPY funcPrinter ${FUNC_PRINTER}/
RUN mkdir -p ${FUNC_PRINTER}/build
WORKDIR ${FUNC_PRINTER}/build
RUN cmake ..
RUN make

ENV CFLAGS="-g -O0 -fno-omit-frame-pointer -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION" \
    CXXFLAGS="-g -O0 -fno-omit-frame-pointer -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION -stdlib=libc++" \
    LLVM_COMPILER=clang \
    CC=wllvm \
    CXX=wllvm++ \
    LLVM_COMPILER_PATH=${LLVM_INSTALL_DIR}/bin \
    LDFLAGS="-lpthread -ldl" \
    FUZZER_LIB=/usr/lib/libFuzzingEngine.a \
    FIXREVERTER_DA_APM=$SRC/apm.json \
    FIXREVERTER_DA_OUT=$SRC/out/

RUN pip3 install wllvm

WORKDIR $SRC
