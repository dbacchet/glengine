#!/bin/bash
CURR_DIR="$( cd "$(dirname "$0")" ; pwd -P )"

CONT_NAME="ubuntu_glengine_map_editor"

docker build ${CURR_DIR} -t ${CONT_NAME}

docker run --gpus all -it \
    --env=DISPLAY=${DISPLAY} \
    --volume=/tmp/.X11-unix:/tmp/.X11-unix:rw \
    --volume=$XAUTHORITY:/home/user/.Xauthority \
    --name ${CONT_NAME} \
    --rm \
    -e NVIDIA_DRIVER_CAPABILITIES=graphics \
    --volume=${CURR_DIR}/../:/code \
    $@ \
    ${CONT_NAME} /bin/bash

