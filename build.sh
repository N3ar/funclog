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

# Make build directory if needed
if [ ! -d "${BUILD}" ]; then
    mkdir ${BUILD}
fi
pushd ${BUILD}

# Clean Build Dir
if [ "$1" == "-c" ] ; then
    echo "[*] Cleaning build directory"
    rm -rf *
fi

# Run CMAKE
cmake ..
RET=$?
if [ ${RET} -ne 0 ] ; then
    echo "[-] COMMAND: 'cmake ..' failed; please see the above output"
    do_exit 1
fi
echo "[+] COMMAND: 'cmake ..' succeeded"

# Perform build with CMAKE
cmake --build .
RET=$?
if [ ${RET} -ne 0 ] ; then
    echo "[-] COMMAND: 'cmake --build .' failed; please see the above output"
    do_exit 1
fi
echo "[+] COMMAND: 'cmake --build .' succeeded"

# Install the Shared Object
if [ "$1" == "-i" ]; then
    echo "[*] Installing in ${INSTALL}"
    cp ${BUILD}/lib/libFuncLog.so ${INSTALL}/libFuncLog.so
fi

# Exit success
do_exit 0
