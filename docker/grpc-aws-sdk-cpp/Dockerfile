FROM gcc:5

RUN apt-get update && \
    apt-get install -y \
    build-essential autoconf git pkg-config zlib1g zlib1g-dev openssl libssl-dev libgtk-3-dev autogen libsnappy-dev \
    automake libtool curl make g++ unzip cmake libgtest-dev libcurl4-openssl-dev golang &&\
    apt-get remove libprotobuf-dev -y && \
    apt-get clean && \
    git clone https://github.com/grpc/grpc.git /var/local/git/grpc && cd /var/local/git/grpc && \
    git submodule update --init

RUN echo " --- installing protobuf --- " && \
    cd /var/local/git/grpc/third_party/protobuf && \
    ./autogen.sh && ./configure --enable-shared && \
    make -j$(nproc) && make install && make clean && ldconfig

RUN echo " --- installing grpc --- " && \
    cd /var/local/git/grpc && \
    make -j$(nproc) && make install && make clean && ldconfig


