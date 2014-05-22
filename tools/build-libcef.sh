#!/bin/bash

here=`pwd`
cefbranch="1916"
cefdir="${here}/libs/chromium/src/cef"
binary_distrib="${cefdir}/binary_distrib"
binary_package="${binary_distrib}/cef_binary_3.1916.1706_linux64"

export GYP_GENERATORS='ninja'
export PATH="${PATH}:${here}/libs/depot_tools"

set -e

mkdir -p ${here}/libs
python `pwd`/tools/automate.py --download-dir=`pwd`/libs \
    --branch=${cefbranch} --force-build

cd "${here}"

mkdir -p libs/cef_binary

if [ -d "${binary_package}" ]; then
    rm -rf libs/cef_binary/*
    rsync -ar --delete "${binary_package}/" libs/cef_binary/
    cd libs/cef_binary
    ./build.sh Release && ./build.sh Debug
else
    echo "CEF3 Binary Package not found."
    exit -1
fi

cd "${here}"
exit 0

