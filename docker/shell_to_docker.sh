#! /usr/bin/env bash

CONT_NAME="ubuntu_glengine_map_editor"
NAME=${CONT_NAME}
if [ -n "$1" ]
  then
    NAME=$1
fi
echo bashing into ${NAME}
docker exec -it ${NAME} /bin/bash
