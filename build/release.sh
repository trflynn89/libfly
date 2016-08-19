#!/usr/bin/env bash
DIR=`dirname "$(readlink -f "$0")"`
NPROC=`nproc`

# Create a clean build and run unit tests.
# $1 = True if this is a release build, false otherwise.
# $2 = Build configuration.
do_build()
{
    conf="-j $NPROC -C $DIR/.. $2"

    make $conf clean && make $conf all && ( $1 || make $conf tests )

    if [ $? != 0 ] ; then
        echo "error: Failed build $1"
        exit 1
    fi
}

do_build false "release=0 arch=i386"
do_build false "release=0 arch=x86_64"
do_build true "release=1 arch=i386"
do_build true "release=1 arch=x86_64"
