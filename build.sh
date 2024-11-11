#!/bin/bash

pushd $(dirname "${BASH_SOURCE[0]}")
HOME=$(pwd)
BUILD="${HOME}/build"
INSTALL="/usr/local/lib"

function do_exit {
    popd
    popd
    exit $1
}

if [ ! -d "${BUILD}" ]; then
    mkdir ${BUILD}
fi

pushd ${BUILD}

# Clean Build Dir
if [ "$1" == "-c" ] ; then
    echo "[*] Cleaning build directory"
    rm -rf *
fi

cmake ..
RET=$?
if [ ${RET} -ne 0 ] ; then
    echo "[-] COMMAND: 'cmake ..' failed; please see the above output"
    do_exit 1
fi
echo "[+] COMMAND: 'cmake ..' succeeded"

cmake --build .
RET=$?
if [ ${RET} -ne 0 ] ; then
    echo "[-] COMMAND: 'cmake --build .' failed; please see the above output"
    do_exit 1
fi
echo "[+] COMMAND: 'cmake --build .' succeeded"

if [ "$1" == "-i" ]; then
    echo "[*] Installing in ${INSTALL}"
    cp ${BUILD}/lib/libFuncLog.so ${INSTALL}/libFuncLog.so
fi

do_exit 0
