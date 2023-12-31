# Base image
FROM ubuntu:20.04

# Install the support for native MPI
RUN apt-get update && apt-get install -y \
        build-essential             \
        wget                        \
        libpcre3                    \
        libmpfr-dev                 \
        libssl-dev                  \
        clang                       \
        python3                     \
        python3-pip                  \
        ca-certificates             \
        --no-install-recommends     \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install seaborn matplotlib pandas numpy

RUN wget http://www.mpich.org/static/downloads/3.1.4/mpich-3.1.4.tar.gz --no-check-certificate \
    && tar xf mpich-3.1.4.tar.gz \
    && cd mpich-3.1.4 \
    && ./configure --disable-fortran --enable-fast=all,O3 --prefix=/usr \
    && make -j$(nproc) \
    && make install \
    && ldconfig \
    && cd .. \
    && rm -rf mpich-3.1.4 \
    && rm mpich-3.1.4.tar.gz

# Configure the environment
WORKDIR ~/project/
COPY . .
RUN mkdir -p tests/implementation/results
RUN mkdir -p plotting/figures

# Compile the library
RUN /bin/bash ./sourceme.sh