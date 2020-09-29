#!/bin/bash
CURR_DIR="$( cd "$(dirname "$0")" ; pwd -P )"

docker build ${CURR_DIR} -t ubuntu_glengine

docker run --gpus all -it \
    --env=DISPLAY=:0 \
    --volume=/tmp/.X11-unix:/tmp/.X11-unix:rw \
    --name ubuntu_glengine \
    --rm \
    -e NVIDIA_DRIVER_CAPABILITIES=graphics \
    --volume=${CURR_DIR}/../:/code \
    ubuntu_glengine /bin/bash
