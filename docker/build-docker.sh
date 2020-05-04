#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR/..

DOCKER_IMAGE=${DOCKER_IMAGE:-sibcoin/sibcoind-develop}
DOCKER_TAG=${DOCKER_TAG:-latest}

BUILD_DIR=${BUILD_DIR:-.}

rm docker/bin/*
mkdir docker/bin
cp $BUILD_DIR/src/sibcoind docker/bin/
cp $BUILD_DIR/src/sibcoin-cli docker/bin/
cp $BUILD_DIR/src/sibcoin-tx docker/bin/
strip docker/bin/sibcoind
strip docker/bin/sibcoin-cli
strip docker/bin/sibcoin-tx

docker build --pull -t $DOCKER_IMAGE:$DOCKER_TAG -f docker/Dockerfile docker
