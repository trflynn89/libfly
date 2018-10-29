#!/usr/bin/env bash
DIR=$(dirname "$(readlink -f "$0")")
NPROC=$(nproc)

# Create a clean build and run unit tests.
# $1 = True if this is a release build, false otherwise.
# $2 = Build architecture (x64 or x86).
do_build()
{
    if [[ $1 == true ]] ; then
        conf="-j $NPROC -C $DIR release=1 arch=$2"
        goal="libfly"
    else
        conf="-j $NPROC -C $DIR release=0 arch=$2"
        goal="libfly tests"
    fi

    make $conf clean && make $conf $goal

    if [[ $? != 0 ]] ; then
        echo "error: Failed build $1"
        exit 1
    fi
}

do_build false "x86"
do_build false "x64"
do_build true "x86"
do_build true "x64"
