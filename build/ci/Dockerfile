# docker image rm "trflynn89/libfly:ubuntu1910_clang9_gcc9_qt5140"
# docker build --tag "trflynn89/libfly:ubuntu1910_clang9_gcc9_qt5140" .
# docker push "trflynn89/libfly:ubuntu1910_clang9_gcc9_qt5140"
#
# docker run --cap-add SYS_PTRACE --name libfly -it trflynn89/libfly:ubuntu1910_clang9_gcc9_qt5140
# docker rm libfly
FROM ubuntu:19.10

MAINTAINER Timothy Flynn <trflynn89@pm.me>

ARG CLANG_VERSION=9
ARG GCC_VERSION=9

ARG QT_URL=http://qt.mirror.constant.com/official_releases/online_installers
ARG QT_INSTALLER=qt-unified-linux-x64-online.run

COPY qt.js /tmp/qt.js

RUN apt-get update && \
    apt-get install -y \
        # Install build dependencies
        clang-$CLANG_VERSION \
        llvm-$CLANG_VERSION \
        \
        gcc-$GCC_VERSION \
        gcc-$GCC_VERSION-multilib \
        g++-$GCC_VERSION \
        g++-$GCC_VERSION-multilib \
        \
        curl \
        lcov \
        make \
        rsync \
        sudo \
        \
        # Install Qt dependencies
        libdbus-1-dev \
        libgl1 \
        libglib2.0-0 \
        libx11-xcb1 \
        mesa-common-dev \
        \
        && \
    update-alternatives --install /usr/bin/clang clang /usr/bin/clang-$CLANG_VERSION 1 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-$CLANG_VERSION 1 && \
    update-alternatives --install /usr/bin/llvm-ar llvm-ar /usr/bin/llvm-ar-$CLANG_VERSION 1 && \
    update-alternatives --install /usr/bin/llvm-cov llvm-cov /usr/bin/llvm-cov-$CLANG_VERSION 1 && \
    update-alternatives --install /usr/bin/llvm-profdata llvm-profdata /usr/bin/llvm-profdata-$CLANG_VERSION 1 && \
    update-alternatives --install /usr/bin/llvm-strip llvm-strip /usr/bin/llvm-strip-$CLANG_VERSION 1 && \
    \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-$GCC_VERSION 1 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-$GCC_VERSION 1 && \
    update-alternatives --install /usr/bin/gcov gcov /usr/bin/gcov-$GCC_VERSION 1 && \
    \
    # Install and compress Qt
    (cd /tmp && curl -O $QT_URL/$QT_INSTALLER) && \
    chmod +x /tmp/$QT_INSTALLER && \
    /tmp/$QT_INSTALLER --platform minimal --script /tmp/qt.js && \
    rm -rf /opt/Qt/Docs /opt/Qt/Examples /opt/Qt/Tools && \
    (cd /opt && tar -czf Qt.tar.gz Qt) && \
    \
    # Cleanup
    apt-get clean -y && \
    apt-get autoremove -y && \
    apt-get purge -y && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* /opt/Qt
