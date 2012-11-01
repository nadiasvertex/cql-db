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
./configure --prefix=$BINDIR --with-pgm --disable-shared --enable-static
make && make install
cd ..

cp -f zmqcpp/zmq.hpp $BINDIR/include
cp -fr pegtl-0.31/include/* $BINDIR/include

# Cleanup
hg clean -X toolchain/tools/*



