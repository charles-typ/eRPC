#!/bin/bash

mkdir -p ../build
cd ../build

# https://stackoverflow.com/questions/17275348/how-to-specify-new-gcc-path-for-cmake
export CC=/opt/rh/devtoolset-8/root/usr/bin/gcc && export CXX=/opt/rh/devtoolset-8/root/usr/bin/g++ && cmake ..

#make -j$(nproc)