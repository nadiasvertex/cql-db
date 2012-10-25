#!/bin/bash
BINDIR=$PWD/tools
mkdir -p $BINDIR
cd protobuf-2.4.1
./configure --prefix=$BINDIR
make && make install
cd ..
hg clean -X toolchain/tools/*



