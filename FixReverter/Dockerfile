FROM gcr.io/oss-fuzz-base/base-builder@sha256:d4b003943394400d2d60616a2f35b8f6fc572c45aa0430f907ae773a027ec2bc

RUN apt-get update && apt-get install -y wget nano bear

ENV LDFLAGS "-lpthread -ldl"
ENV FUZZER_LIB /usr/lib/FRFuzzingEngine.a
ENV LIB_FUZZING_ENGINE /usr/lib/FRFuzzingEngine.a

ARG FIXREVERTER=/fixreverter
ARG REPODIR=${FIXREVERTER}/FixReverter
ARG LLVMDIR=${FIXREVERTER}/llvm-project

RUN mkdir -p ${FIXREVERTER}

RUN mkdir -p ${REPODIR}
ADD clangTools ${REPODIR}/clangTools
COPY docker/libFuzzingEngine.a /usr/lib/
COPY docker/FRFuzzingEngine.a /usr/lib/

WORKDIR ${FIXREVERTER}
RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.66.0/source/boost_1_66_0.tar.bz2
RUN tar --bzip2 -xf boost_1_66_0.tar.bz2
WORKDIR ${FIXREVERTER}/boost_1_66_0
RUN ./bootstrap.sh --with-libraries=system,filesystem,container
RUN ./b2 install


WORKDIR ${FIXREVERTER}
RUN git clone https://github.com/llvm/llvm-project.git --branch llvmorg-12.0.0 ${LLVMDIR}
RUN mkdir -p ${LLVMDIR}/build

WORKDIR ${LLVMDIR}/clang-tools-extra/
RUN ln -s ${REPODIR}/clangTools/astPatternMatcher
RUN ln -s ${REPODIR}/clangTools/mainFuncLocator
RUN ln -s ${REPODIR}/clangTools/mutantLocator
RUN ln -s ${REPODIR}/clangTools/rewriter
RUN ln -s ${REPODIR}/clangTools/utils

RUN echo "add_subdirectory(astPatternMatcher)" >> ${LLVMDIR}/clang-tools-extra/CMakeLists.txt
RUN echo "add_subdirectory(mutantLocator)" >> ${LLVMDIR}/clang-tools-extra/CMakeLists.txt
RUN echo "add_subdirectory(rewriter)" >> ${LLVMDIR}/clang-tools-extra/CMakeLists.txt
RUN echo "add_subdirectory(mainFuncLocator)" >> ${LLVMDIR}/clang-tools-extra/CMakeLists.txt
RUN echo "add_subdirectory(utils)" >> ${LLVMDIR}/clang-tools-extra/CMakeLists.txt

WORKDIR ${LLVMDIR}/build
RUN cmake ../llvm -DCMAKE_C_FLAGS="-pthread" -DCMAKE_CXX_FLAGS="-pthread" -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra"
RUN make -j30

ADD drivers ${REPODIR}/drivers

RUN ldconfig

WORKDIR $SRC

