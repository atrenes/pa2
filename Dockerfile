FROM ubuntu:14.04

RUN apt-get update && \
    apt-get install -y clang && \
    apt-get install -y make && \
    apt-get install -y binutils

ADD . /app/src
WORKDIR /app/src

RUN make
RUN ./a.out -p 3
