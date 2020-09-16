#!/bin/bash

CURRENT_DIR=`pwd`
OUTPUT=$CURRENT_DIR/output
BUILD_DIR=${BUILD_DIR:-${CURRENT_DIR}/cmake_build}
BUILD_TYPE=${BUILD_TYPE:-Debug}
INSTALL_DIR=${INSTALL_DIR:-${BUILD_DIR}/${BUILD_TYPE}_install}

if [ "$1"x = "clean"x ]
then
  rm -rf ${BUILD_DIR}
  rm -rf ${OUTPUT}
  exit 0
fi

mkdir -p ${OUTPUT}
mkdir -p ${BUILD_DIR}

cmake -H${CURRENT_DIR} -B${BUILD_DIR} \
  -DCMAKE_C_COMPILER=/usr/local/bin/gcc \
  -DCMAKE_CXX_COMPILER=/usr/local/bin/g++ \
  -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} && \
cmake --build ${BUILD_DIR} -- -j && \
cmake --build ${BUILD_DIR} --target install

cp -rf ${INSTALL_DIR}/* ${OUTPUT}

