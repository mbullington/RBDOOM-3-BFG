#!/bin/zsh

cd ..
rm -rf build
mkdir build
cd build

# change or remove -DCMAKE_OSX_DEPLOYMENT_TARGET=<version> to match supported runtime targets
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET=10.12 -DCOMPILE_COMMANDS=ON -DOPENAL_LIBRARY=/usr/local/opt/openal-soft/lib/libopenal.dylib -DOPENAL_INCLUDE_DIR=/usr/local/opt/openal-soft/include ../neo -Wno-dev

# cp compile_commands.json to root of project so clangd can use it
cp compile_commands.json ..
