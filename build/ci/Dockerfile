# docker image rm "trflynn89/libfly:ubuntu1904_gcc9"
# docker build --tag "trflynn89/libfly:ubuntu1904_gcc9" .
# docker push "trflynn89/libfly:ubuntu1904_gcc9"
FROM ubuntu:19.04

MAINTAINER Timothy Flynn <trflynn89@pm.me>

ENV CC gcc-9
ENV CXX g++-9
ENV GCOV gcov-9

RUN apt-get update && \
    # Install build dependencies
    apt-get install -y \
        $CC \
        $CC-multilib \
        $CXX \
        $CXX-multilib \
        curl \
        git \
        lcov \
        make \
        rsync \
        && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/$CC 1 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/$CXX 1 && \
    update-alternatives --install /usr/bin/gcov gcov /usr/bin/$GCOV 1 && \
    \
    # Replace LCOV with a more recent version that supports GCC 9
    git clone https://github.com/linux-test-project/lcov.git && \
    (cd lcov && git reset --hard 75fbae1cfc5027f818a0bb865bf6f96fab3202da) && \
    (cd lcov && make install) && \
    \
    # Cleanup
    apt-get clean -y && \
    apt-get autoremove -y && \
    apt-get purge -y && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* lcov
