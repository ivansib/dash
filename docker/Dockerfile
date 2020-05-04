FROM debian:stretch
LABEL maintainer="Sibcoin Developers <dev@sibcoin.org>"
LABEL description="Dockerised SibcoinCore, built from Travis"

RUN apt-get update && apt-get -y upgrade && apt-get clean && rm -fr /var/cache/apt/*

COPY bin/* /usr/bin/
