#!/bin/bash
cd /store/workspace/lattice
rm -f lattice_test
scons
./lattice_test

