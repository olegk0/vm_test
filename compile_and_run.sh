#!/bin/bash

#pushd compiler
#make clean
#reset
#make
#popd

compiler/compss example.ss
ret=$?
echo $ret
[ $ret -eq 0 ] || exit


pushd vm
make clean
reset
make
popd

vm/vm example.sb