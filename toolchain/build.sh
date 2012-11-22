#!/bin/bash
BINDIR=$PWD/tools
mkdir -p $BINDIR

# Build APR
cd apr-1.4.6
./configure --prefix=$BINDIR --disable-shared --enable-static
make && make install
cd..

# Build log4cxx
cd apache-log4cxx-0.10.0
./configure --prefix=$BINDIR --disable-shared --enable-static --disable-doxygen --with-apr=$BINDIR
make && make install
cd..

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

# Build libjit
cd libjit-0.1.2
rm -fr $BINDIR/include/jit/
./configure --prefix=$BINDIR --disable-shared --enable-static
cd tools && make && make install && cd ..
cd jit && make && make install && cd ..
cd jitplus && make && make install && cd ..
cd jitdynamic && make && make install && cd ..
mkdir -p $BINDIR/include/jit/
cp -fr include/jit/*.h $BINDIR/include/jit/
cd ..

cp -f zmqcpp/zmq.hpp $BINDIR/include
cp -fr pegtl-0.31/include/* $BINDIR/include

# Cleanup
hg clean -X toolchain/tools/*

