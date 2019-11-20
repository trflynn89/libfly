# docker image rm "trflynn89/libfly:ubuntu1910_gcc9_qt5132"
# docker build --tag "trflynn89/libfly:ubuntu1910_gcc9_qt5132" .
# docker push "trflynn89/libfly:ubuntu1910_gcc9_qt5132"
#
# docker run --cap-add SYS_PTRACE --name libfly -it trflynn89/libfly:ubuntu1910_gcc9_qt5132
# docker rm libfly
FROM ubuntu:19.10

MAINTAINER Timothy Flynn <trflynn89@pm.me>

ENV CC gcc-9
ENV CXX g++-9
ENV GCOV gcov-9

ARG QT_INSTALLER=qt-unified-linux-x64-online.run
ARG QT_URL=http://qt.mirror.constant.com/official_releases/online_installers

COPY qt.js /tmp/qt.js

RUN apt-get update && \
    apt-get install -y \
        # Install build dependencies
        $CC \
        $CC-multilib \
        $CXX \
        $CXX-multilib \
        curl \
        lcov \
        make \
        rsync \
        sudo \
        \
        # Install Qt dependencies
        libdbus-1-dev \
        libglib2.0-0 \
        libx11-xcb1 \
        mesa-common-dev \
        \
        && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/$CC 1 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/$CXX 1 && \
    update-alternatives --install /usr/bin/gcov gcov /usr/bin/$GCOV 1 && \
    \
    # Install and compress Qt
    (cd /tmp && curl -O $QT_URL/$QT_INSTALLER) && \
    chmod +x /tmp/$QT_INSTALLER && \
    /tmp/$QT_INSTALLER --platform minimal --script /tmp/qt.js && \
    rm -rf /opt/Qt/Docs /opt/Qt/Examples /opt/Qt/Licenses /opt/Qt/Tools && \
    (cd /opt && tar -czf Qt.tar.gz Qt) && \
    \
    # Cleanup
    apt-get clean -y && \
    apt-get autoremove -y && \
    apt-get purge -y && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* /opt/Qt
