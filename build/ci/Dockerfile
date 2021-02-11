FROM ubuntu:20.10

MAINTAINER Timothy Flynn <trflynn89@pm.me>

ARG CLANG_VERSION
ARG GCC_VERSION
ARG JDK_VERSION

# Install base tools
RUN apt-get update && \
    apt-get install -y \
        curl \
        lcov \
        make \
        sudo \
    && \
    \
    echo "Set disable_coredump false" >> /etc/sudo.conf

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
        clang-format-$CLANG_VERSION \
        lld-$CLANG_VERSION \
        llvm-$CLANG_VERSION \
    && \
    \
    for tool in clang clang++ clang-format lld llvm-ar llvm-cov llvm-profdata llvm-strip; \
    do \
        update-alternatives --install \
            /usr/bin/$tool $tool /usr/bin/$tool-$CLANG_VERSION 1; \
    done

# Install OpenJDK
RUN if test ! -z "$JDK_VERSION" ; then \
        DEBIAN_FRONTEND=noninteractive apt-get install -y openjdk-$JDK_VERSION-jdk; \
    fi

# Cleanup
RUN apt-get clean -y && \
    apt-get autoremove -y && \
    apt-get purge -y && \
    rm -rf /tmp/* /var/tmp/* /var/cache/* /var/lib/apt/lists/*
