FROM ubuntu:22.04

MAINTAINER Timothy Flynn <trflynn89@pm.me>

ARG CLANG_VERSION
ARG GCC_VERSION
ARG JDK_VERSION

# Install base tools
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
        bzip2 \
        ccache \
        git \
        lcov \
        make \
        python3 \
        python3-pip \
        software-properties-common \
        sudo \
    && \
    \
    echo "Set disable_coredump false" >> /etc/sudo.conf

# Install gcc
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/ppa \
    && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
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
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y \
        clang-$CLANG_VERSION \
        lld-$CLANG_VERSION \
        llvm-$CLANG_VERSION \
    && \
    \
    for tool in clang clang++ ld.lld lld llvm-ar llvm-cov llvm-profdata llvm-strip; \
    do \
        update-alternatives --install \
            /usr/bin/$tool $tool /usr/bin/$tool-$CLANG_VERSION 1; \
    done

# Install OpenJDK
RUN if test ! -z "$JDK_VERSION" ; then \
        DEBIAN_FRONTEND=noninteractive apt-get install -y openjdk-$JDK_VERSION-jdk; \
    fi

# Install Python dependencies
RUN pip3 install requests

# Cleanup
RUN apt-get clean -y && \
    apt-get autoremove -y && \
    apt-get purge -y && \
    rm -rf /tmp/* /var/tmp/* /var/cache/* /var/lib/apt/lists/*
