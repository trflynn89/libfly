# docker image rm "trflynn89/libfly:ubuntu18_gcc8"
# docker build --tag "trflynn89/libfly:ubuntu18_gcc8" .
# docker push "trflynn89/libfly:ubuntu18_gcc8"
FROM ubuntu:18.04

ENV LCOV_URL https://github.com/linux-test-project/lcov/releases/download/v1.14/lcov-1.14.tar.gz

RUN apt-get update && apt-get install -y \
    curl \
    g++-8 \
    g++-8-multilib \
    gcc-8 \
    gcc-8-multilib \
    lcov \
    make \
    rsync

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 800 --slave /usr/bin/g++ g++ /usr/bin/g++-8 && \
    update-alternatives --install /usr/bin/gcov gcov /usr/bin/gcov-8 800

# Replace LCOV with a more recent version that supports GCC 8
RUN curl -L $LCOV_URL -o lcov.tar.gz && \
    tar -xf lcov.tar.gz && \
    (tar -tf lcov.tar.gz | head -n1 | xargs -i mv {} lcov) && \
    (find lcov/bin -type f ! -name "*.*" | xargs -i mv {} /usr/bin/) && \
    rm -r lcov*
