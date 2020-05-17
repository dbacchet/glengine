FROM ubuntu:16.04
MAINTAINER Davide

RUN apt-get update
RUN apt-get install -y build-essential
RUN apt-get install -y gdb
RUN apt-get install -y git
RUN apt-get install -y python3
RUN apt-get install -y python3-pip
RUN apt-get install -y vim
RUN apt-get install -y zsh
RUN apt-get install -y curl
RUN apt-get install -y wget
RUN apt-get install -y htop
# recent cmake
ADD https://cmake.org/files/v3.12/cmake-3.12.3-Linux-x86_64.sh /tmp/
RUN mkdir -p /opt/cmake && sh /tmp/cmake-3.12.3-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
RUN ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake
# neovim
RUN apt-get install -y software-properties-common
RUN apt-add-repository -y ppa:neovim-ppa/stable
RUN apt-get update
RUN apt-get install -y neovim
RUN pip3 install --user neovim
# gdbgui
RUN pip3 install --user gdbgui
EXPOSE 5000
# configure environment
RUN cd ~ && git clone https://github.com/dbacchet/env_config.git .env_config && cp -r .env_config/.config .
RUN cd ~ && git clone https://github.com/dbacchet/oh-my-zsh.git .oh-my-zsh && cp -r .env_config/.zshrc .

# RUN cd ~ && git clone https://github.com/dbacchet/job-system.git

# IMPORTANT!!!
# to enable gdb inside the container, security options must be overridden:
# docker run -p 5000:5000 --cap-add=SYS_PTRACE --security-opt seccomp=unconfined -ti ubuntu1604_dev /bin/zsh
