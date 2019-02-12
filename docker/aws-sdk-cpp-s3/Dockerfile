FROM gcc:latest

WORKDIR /gtest_build

RUN apt-get update && \
    apt-get install -y cmake

RUN echo " --- Download, build and install aws-c-common --- " \
    && git clone https://github.com/awslabs/aws-c-common \
    && cd aws-c-common \
    # This is the commit we've tested that works with C++ SDK.
    && git checkout a9380a43ab3762 \
    && mkdir build \
    && cd build \
    # without CMAKE_INSTALL_PREFIX, it will be installed to /usr/local/lib by default.
    && cmake .. -DCMAKE_INSTALL_PREFIX=~/aws_deps -DBUILD_SHARED_LIBS=ON \
    && make -j 8 \
    && make install

RUN echo " --- Download, build and install aws-checksums --- " \
    && git clone https://github.com/awslabs/aws-checksums \
    && cd aws-checksums \
    && git checkout 8db7ac89d232b \
    && mkdir build && cd build \
    && cmake .. -DCMAKE_INSTALL_PREFIX=~/aws_deps -DBUILD_SHARED_LIBS=ON \
    && make -j 8 \
    && make install

RUN echo " --- Download, build and install aws-checksums --- " \
    && git clone https://github.com/awslabs/aws-c-event-stream \
    && cd aws-c-event-stream \
    && git checkout 40dc42f47f3 \
    && mkdir build && cd build \
    && cmake .. -DCMAKE_INSTALL_PREFIX=~/aws_deps -DCMAKE_PREFIX_PATH=~/aws_deps -DBUILD_SHARED_LIBS=ON \
    && make -j 8 \
    && make install

RUN echo " --- Download, build and install aws-sdk-cpp --- " && \
    git clone https://github.com/aws/aws-sdk-cpp.git &&\
    cd aws-sdk-cpp &&\
    git checkout tags/1.7.41 &&\
    cmake -DBUILD_ONLY='s3;core;kinesis' -DBUILD_DEPS=ON -DCMAKE_PREFIX_PATH=~/aws_deps -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTING=OFF . &&\
    make &&\
    make install &&\
    cd .. && rm -R aws-sdk-cpp