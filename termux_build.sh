#!/bin/bash
cd $(dirname $0)
mkdir build && cd build
cmake ..
make -j4
cp aimbot ../
echo "Build complete. Run ./aimbot"
