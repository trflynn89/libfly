# docker image rm "trflynn89/libfly:ubuntu18_gcc8"
# docker build --tag "trflynn89/libfly:ubuntu18_gcc8" .
# docker push "trflynn89/libfly:ubuntu18_gcc8"
FROM ubuntu:18.04

MAINTAINER Timothy Flynn <trflynn89@pm.me>

ENV CC gcc-8
ENV CXX g++-8
ENV GCOV gcov-8
ENV LCOV https://github.com/linux-test-project/lcov/releases/download/v1.14/lcov-1.14.tar.gz

RUN apt-get update && \
    # Install build dependencies
    apt-get install -y \
        $CC \
        $CC-multilib \
        $CXX \
        $CXX-multilib \
        curl \
        lcov \
        make \
        rsync \
        && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/$CC 1 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/$CXX 1 && \
    update-alternatives --install /usr/bin/gcov gcov /usr/bin/$GCOV 1 && \
    \
    # Replace LCOV with a more recent version that supports GCC 8
    (cd /tmp && curl -L $LCOV -o lcov.tar.gz) && \
    (cd /tmp && tar -xf lcov.tar.gz) && \
    (cd /tmp && tar -tf lcov.tar.gz | head -n1 | xargs -i mv {} lcov) && \
    (cd /tmp && find lcov/bin -type f ! -name "*.*" | xargs -i mv {} /usr/bin/) && \
    \
    # Cleanup
    apt-get clean && \
    apt-get autoremove && \
    apt-get purge && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
