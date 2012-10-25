#!/bin/bash
BINDIR=$PWD/bin
cd protobuf-2.4.1
./configure --prefix=$BINDIR
make && make install
cd ..



