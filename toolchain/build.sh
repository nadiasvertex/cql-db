#!/bin/bash
BINDIR=$PWD/tools
mkdir -p $BINDIR
# Build protobuf
cd protobuf-2.4.1
./configure --prefix=$BINDIR
make && make install
cd ..

# Build 0MQ
cd zeromq-3.2.1
./configure --prefix=$BINDIR
make && make install
cd ..

# Cleanup
hg clean -X toolchain/tools/*



