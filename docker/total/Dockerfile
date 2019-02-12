FROM gcc:latest

WORKDIR /gtest_build

ENV USE_AWS=1
ENV CMAKE_PREFIX_PATH=/usr/local/aws-sdk-cpp
ENV PLATFORM_LDFLAGS='-lcurl -lssl -lcrypto -lz'

RUN apt-get update \
    && apt-get install -y cmake build-essential autoconf git pkg-config zlib1g zlib1g-dev openssl libssl-dev libgtk-3-dev autogen libsnappy-dev \
    automake libtool curl make g++ unzip cmake libgtest-dev libcurl4-openssl-dev golang \
    && apt-get remove libprotobuf-dev -y \
    && echo " --- Download, build and install aws-c-common --- " \
    && git clone https://github.com/awslabs/aws-c-common \
    && cd aws-c-common \
    # This is the commit we've tested that works with C++ SDK.
    && git checkout a9380a43ab3762 \
    && mkdir build \
    && cd build \
    # without CMAKE_INSTALL_PREFIX, it will be installed to /usr/local/lib by default.
    && cmake .. -DCMAKE_INSTALL_PREFIX=~/aws_deps -DBUILD_SHARED_LIBS=ON \
    && make -j 8 \
    && make install \
    && echo " --- Download, build and install aws-checksums --- " \
    && git clone https://github.com/awslabs/aws-checksums \
    && cd aws-checksums \
    && git checkout 8db7ac89d232b \
    && mkdir build && cd build \
    && cmake .. -DCMAKE_INSTALL_PREFIX=~/aws_deps -DBUILD_SHARED_LIBS=ON \
    && make -j 8 \
    && make install \
    && echo " --- Download, build and install aws-checksums --- " \
    && git clone https://github.com/awslabs/aws-c-event-stream \
    && cd aws-c-event-stream \
    && git checkout 40dc42f47f3 \
    && mkdir build && cd build \
    && cmake .. -DCMAKE_INSTALL_PREFIX=~/aws_deps -DCMAKE_PREFIX_PATH=~/aws_deps -DBUILD_SHARED_LIBS=ON \
    && make -j 8 \
    && make install \
    && echo " --- Download, build and install aws-sdk-cpp --- " \
    && git clone https://github.com/aws/aws-sdk-cpp.git \
    && cd aws-sdk-cpp \
    && git checkout tags/1.7.41 \
    && cmake -DBUILD_DEPS=ON -DCMAKE_PREFIX_PATH=~/aws_deps -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTING=OFF . \
    && make \
    && make install \
    && cd .. && rm -R aws-sdk-cpp  \
    && git clone https://github.com/rockset/rocksdb-cloud.git /var/local/git/rocksdb-cloud \
    && cd /var/local/git/rocksdb-cloud \
    && echo " --- installing rocksdb-cloud --- "

RUN cd /var/local/git/rocksdb-cloud && make clean \
    && make static_lib \
    && make install