#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR/..

DOCKER_IMAGE=${DOCKER_IMAGE:-zenxpay/zenxd-develop}
DOCKER_TAG=${DOCKER_TAG:-latest}

BUILD_DIR=${BUILD_DIR:-.}

rm docker/bin/*
mkdir docker/bin
cp $BUILD_DIR/src/zenxd docker/bin/
cp $BUILD_DIR/src/zenx-cli docker/bin/
cp $BUILD_DIR/src/zenx-tx docker/bin/
strip docker/bin/zenxd
strip docker/bin/zenx-cli
strip docker/bin/zenx-tx

docker build --pull -t $DOCKER_IMAGE:$DOCKER_TAG -f docker/Dockerfile docker
