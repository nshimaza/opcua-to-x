FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive
RUN cd && \
    mkdir build && \
    apt-get update && \
    apt-get install -y build-essential cmake git pkg-config python vim && \
    apt-get install -y libmbedtls-dev check libsubunit-dev && \
    git clone https://github.com/libuv/libuv.git && \
    cd libuv && \
    git checkout v1.34.2 && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make && \
    make install && \
    cd ../.. && \
    git clone --recursive https://github.com/open62541/open62541.git && \
    cd open62541 && \
    git checkout v.1.0.1 && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Debug -DUA_NAMESPACE_ZERO=FULL -DUA_LOGLEVEL=100 -DUA_MULTITHREADING=100 .. && \
    make && \
    make install && \
    cd && \
    git clone https://github.com/OPCFoundation/UA-Nodeset.git && \
    git clone https://github.com/benhoyt/inih.git
