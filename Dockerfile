# FILE: Dockerfile

FROM ubuntu:20.04

# install dependencies
RUN apt update\
    && DEBIAN_FRONTEND=noninteractive \
       TZ=US/Eastern \
       apt install -y \
        build-essential \ 
        libssl-dev \ 
        libssh-dev \
        cmake \
        python3 \
        python3-pyqt5 

# update the APT sources from our local copy
#COPY sources.list /etc/apt/sources.list