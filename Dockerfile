FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive

# Install pre-requisites
RUN apt update
RUN apt install cmake g++ gcc nano less telnet valgrind -y

WORKDIR /kinslayer
RUN ulimit -S -c unlimited
RUN ldconfig

EXPOSE 2222

ENTRYPOINT ["/kinslayer/docker-start.sh"]