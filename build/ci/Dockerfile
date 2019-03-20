FROM ubuntu:18.04

ENV DOCKER_URL https://github.com/linux-test-project/lcov/releases/download/v1.14/lcov-1.14.tar.gz

RUN apt-get update && apt-get install -y \
    curl \
    g++-8 \
    g++-8-multilib \
    gcc-8 \
    gcc-8-multilib \
    lcov \
    make \
    rsync

RUN curl -L $DOCKER_URL -o lcov.tar.gz && \
    tar -xf lcov.tar.gz && \
    (tar -tf lcov.tar.gz | head -n1 | xargs -i mv {} lcov) && \
    mv lcov/bin/lcov /usr/bin/lcov && \
    rm -r lcov*
