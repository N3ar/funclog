#!/bin/bash

pushd $(dirname "${BASH_SOURCE[0]}")
TEST=$(pwd)

BUILD="${TEST}/../build"

NAME="hello"
TGT="${TEST}/${NAME}.c"

do_exit() {
    popd
    exit $1
}

# TODO Show help
if [ "{$1}" == "-h" ] ; then
    echo "-f update .ll file"
    echo "-p run pass only"
    exit 0
fi

# Emit LLVM
if [ "{$1}" != "-p" ]; then
    echo "[*] **** generating LLVM-IR"
    clang -S -emit-llvm ${TGT}
    # clang -S -emit-llvm -llogger ${TGT}
fi

# Run LLVM Pass - Instrument
echo "[*] **** RUNNING PASS THROUGH OPT"
if ! opt -load-pass-plugin="${BUILD}/lib/libFuncLog.so" -passes="funclog" -S "${NAME}.ll" -o "instr-${NAME}.ll" ; then
    echo "[-] opt failed to run pass"
    do_exit 1
fi

# Compile Instrumented LLVM
echo "[*] **** Building instrumented executable"
if ! clang -llogger "instr-${NAME}.ll" -o "${NAME}" -v ; then
    echo "[-] clang could not build final executable"
    do_exit 1
fi

# Exec Instrumented code
echo "[*] **** Executing Instrumented Code"
if ! ./${NAME} ; then
    echo "[-] Final Executable Crashed"
    do_exit 1
fi

do_exit 0
