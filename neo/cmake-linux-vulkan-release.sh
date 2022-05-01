#!/bin/bash

cd ..
rm -rf build
mkdir build
cd build

cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DUSE_VULKAN=ON -DSPIRV_SHADERC=OFF ../neo
