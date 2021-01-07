#!/bin/bash
set -e

CURR_DIR="$( cd "$(dirname "$0")" ; pwd -P )"

OS_PREFIX="win32"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS_PREFIX="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS_PREFIX="osx"
fi

mkdir -p ${CURR_DIR}/generated
for shader_file in ${CURR_DIR}/*.glsl
do
    BASENAME=`basename ${shader_file}`
    ${CURR_DIR}/bin/${OS_PREFIX}/sokol-shdc -i ${shader_file} -o ${CURR_DIR}/generated/${BASENAME}.h -l glsl330
done
