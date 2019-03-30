# docker image rm "trflynn89/libfly:ubuntu1904_gcc9"
# docker build --tag "trflynn89/libfly:ubuntu1904_gcc9" .
# docker push "trflynn89/libfly:ubuntu1904_gcc9"
FROM ubuntu:19.04

MAINTAINER Timothy Flynn <trflynn89@pm.me>

ENV CC gcc-9
ENV CXX g++-9
ENV GCOV gcov-9
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
    # Replace LCOV with a more recent version that supports GCC 9
    (cd /tmp && curl -L $LCOV -o lcov.tar.gz) && \
    (cd /tmp && tar -xf lcov.tar.gz) && \
    (cd /tmp && tar -tf lcov.tar.gz | head -n1 | xargs -i mv {} lcov) && \
    (cd /tmp && find lcov/bin -type f ! -name "*.*" | xargs -i mv {} /usr/bin/) && \
    \
    # Cleanup
    apt-get clean -y && \
    apt-get autoremove -y && \
    apt-get purge -y && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
