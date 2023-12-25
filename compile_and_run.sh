#!/bin/bash

#pushd compiler
#make clean
#reset
#make
#popd

compiler/compss tests.ss
ret=$?
echo $ret
[ $ret -eq 0 ] || exit


pushd vm
make clean
reset
make
popd

vm/vm -s tests.sb
#gdb --args vm/vm -s tests.sb