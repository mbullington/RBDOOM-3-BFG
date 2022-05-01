#!/bin/bash

cd ..
rm -rf build
mkdir build
cd build

cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCOMPILE_COMMANDS=ON ../neo

# cp compile_commands.json to root of project so clangd can use it
cp compile_commands.json ..
