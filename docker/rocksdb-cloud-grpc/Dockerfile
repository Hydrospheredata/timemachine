FROM lerrox/aws-sdk-cpp-s3-kinesis:1.7.41

RUN apt-get update && apt-get install -y \
  build-essential autoconf git pkg-config zlib1g zlib1g-dev openssl libssl-dev libgtk-3-dev autogen libsnappy-dev \
  automake libtool curl make g++ unzip cmake libgtest-dev libcurl4-openssl-dev golang &&\
  apt-get remove libprotobuf-dev -y &&\
  apt-get clean && \
  git clone https://github.com/rockset/rocksdb-cloud.git /var/local/git/rocksdb-cloud && cd /var/local/git/rocksdb-cloud

ENV USE_AWS=1
ENV CMAKE_PREFIX_PATH=/usr/local/aws-sdk-cpp
ENV PLATFORM_LDFLAGS='-lcurl -lssl -lcrypto -lz'

RUN cd /var/local/git/rocksdb-cloud \
    && echo " --- installing rocksdb-cloud --- " \
    && make clean all \
    && make -j $MAKEJ static_lib \
    && make install