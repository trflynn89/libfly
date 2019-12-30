FROM ubuntu:19.10

MAINTAINER Timothy Flynn <trflynn89@pm.me>

ARG QT_INSTALLER_URL
ARG QT_INSTALLER_EXE

ARG CLANG_VERSION
ARG GCC_VERSION

COPY qt.js /tmp/qt.js

# Install base tools
RUN apt-get update && \
    apt-get install -y \
        curl \
        lcov \
        make \
        rsync \
        sudo

# Install Qt dependencies
RUN apt-get install -y \
        libdbus-1-3 \
        libfontconfig1 \
        libfreetype6 \
        libgl1 \
        libglib2.0-0 \
        libx11-xcb1 \
        mesa-common-dev

# Install and compress Qt
RUN (cd /tmp && curl -O $QT_INSTALLER_URL/$QT_INSTALLER_EXE) && \
    chmod +x /tmp/$QT_INSTALLER_EXE && \
    /tmp/$QT_INSTALLER_EXE --platform minimal --script /tmp/qt.js && \
    rm -rf /opt/Qt/Docs /opt/Qt/Examples /opt/Qt/Tools && \
    (cd /opt && tar -czf Qt.tar.gz Qt && rm -rf Qt)

# Install gcc
RUN apt-get install -y \
        gcc-$GCC_VERSION \
        gcc-$GCC_VERSION-multilib \
        g++-$GCC_VERSION \
        g++-$GCC_VERSION-multilib \
    && \
    \
    for tool in gcc g++ gcov; \
    do \
        update-alternatives --install \
            /usr/bin/$tool $tool /usr/bin/$tool-$GCC_VERSION 1; \
    done

# Install clang
RUN apt-get install -y \
        clang-$CLANG_VERSION \
        lld-$CLANG_VERSION \
        llvm-$CLANG_VERSION \
    && \
    \
    for tool in clang clang++ lld llvm-ar llvm-cov llvm-profdata llvm-strip; \
    do \
        update-alternatives --install \
            /usr/bin/$tool $tool /usr/bin/$tool-$CLANG_VERSION 1; \
    done

# Cleanup
RUN apt-get clean -y && \
    apt-get autoremove -y && \
    apt-get purge -y && \
    rm -rf /tmp/* /var/tmp/* /var/cache/* /var/lib/apt/lists/*
