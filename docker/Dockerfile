FROM ubuntu:20.04
MAINTAINER Davide

ENV DEBIAN_FRONTEND="noninteractive"
RUN apt-get update
RUN apt-get install -y bash-completion sudo
# c/cpp build env
RUN apt-get install -y build-essential gdb lcov git vim wget htop unzip sudo cmake cmake-curses-gui
# deps for opengl and glfw
RUN apt-get install -y libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
# RUN apt-get install -y libglfw3-dev # built with the engine
# PCL and deps (man, PCL brings in every lib created since the 80s...)
RUN apt-get install -y libpcl-dev
# protocol buffers
RUN apt-get install -y libprotobuf-dev protobuf-compiler

# Add basic user
ENV USERNAME user
RUN useradd -ms /bin/bash ${USERNAME} && \
    usermod -aG sudo ${USERNAME} && \
    echo "user ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/${USERNAME} && \
    chmod 0440 /etc/sudoers.d/${USERNAME}

ENV HOME /home/${USERNAME}
USER ${USERNAME}
