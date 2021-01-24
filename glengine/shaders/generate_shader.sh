#!/bin/bash
set -e

shader_file=$1
output_file=$2

CURR_DIR="$( cd "$(dirname "$0")" ; pwd -P )"

OS_PREFIX="win32"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS_PREFIX="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS_PREFIX="osx"
fi

OUT_DIR=`dirname ${output_file}`
mkdir -p ${OUT_DIR}
${CURR_DIR}/bin/${OS_PREFIX}/sokol-shdc -i ${shader_file} -o ${output_file} -l glsl330:metal_macos
