FROM ubuntu:18.04
MAINTAINER Davide

RUN apt-get update
RUN apt-get install -y build-essential gdb lcov git vim wget htop unzip sudo cmake
RUN apt-get install -y libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev libglfw3-dev

####################################################################
# Add basic user
# # Replace 1000 with your user/group id
####################################################################
ENV USERNAME user
RUN export uid=1000 gid=1000 && \
    mkdir -p /home/${USERNAME} && \
    echo "${USERNAME}:x:${uid}:${gid}:${USERNAME},,,:/home/${USERNAME}:/bin/bash" >> /etc/passwd && \
    echo "${USERNAME}:x:${uid}:" >> /etc/group && \
    echo "${USERNAME} ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/${USERNAME} && \
    chmod 0440 /etc/sudoers.d/${USERNAME} && \
    chown ${uid}:${gid} -R /home/${USERNAME}

ENV HOME /home/${USERNAME}
USER ${USERNAME}
