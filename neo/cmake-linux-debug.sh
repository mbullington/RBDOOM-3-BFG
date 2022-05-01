#!/bin/bash

cd ..
rm -rf build
mkdir build
cd build

cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCOMPILE_COMMANDS=ON -DOpenGL_GL_PREFERENCE=GLVND ../neo

# cp compile_commands.json to root of project so clangd can use it
cp compile_commands.json ..
