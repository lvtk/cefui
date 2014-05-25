#!/bin/bash

here=`pwd`
cefbranch="trunk"
cefcheckout="e13421c0cfd863a332e600d9cfec3f614d040be2"

cefdir="${here}/libs/chromium/src/cef"
binary_distrib="${cefdir}/binary_distrib"
binary_package="${binary_distrib}/cef_binary_3.1935.1710_linux64"
export GYP_GENERATORS='ninja'
export GYP_DEFINES=''
export PATH="${PATH}:${here}/libs/depot_tools"

set -e

mkdir -p ${here}/libs
python `pwd`/tools/automate.py --download-dir=`pwd`/libs \
    --branch=${cefbranch} --use-svn $*

cd "${here}"

mkdir -p libs/cef_binary

if [ -d "${binary_package}" ]; then
    echo "Creating binary package in 'libs/cef_binary'"
    rm -rf libs/cef_binary/*
    rsync -ar --delete "${binary_package}/" libs/cef_binary/
else
    echo "CEF3 Binary Package not found."
    exit -1
fi

cd "${here}"
exit 0
