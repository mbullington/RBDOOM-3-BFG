#!/bin/bash

cd ..
rm -rf build
mkdir build
cd build

CMAKE_FLAGS=()

VULKAN_OR_OPENGL=$((echo "Vulkan" && echo "OpenGL") | fzf)
if [[ "$VULKAN_OR_OPENGL" == "Vulkan" ]]; then
    CMAKE_FLAGS+=(-DUSE_VULKAN=ON)
fi

DEBUG_RELEASE_OR_RETAIL=$((echo "Debug" && echo "Release" && echo "Retail") | fzf)
if [[ "$DEBUG_RELEASE_OR_RETAIL" == "Debug" ]]; then
    CMAKE_FLAGS+=(-DCOMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug)
elif [[ "$DEBUG_RELEASE_OR_RETAIL" == "Release" ]]; then
    CMAKE_FLAGS+=(-DCMAKE_BUILD_TYPE=Release)
else
    CMAKE_FLAGS+=(-DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS_RELEASE="-DID_RETAIL")
fi

echo "$VULKAN_OR_OPENGL $DEBUG_RELEASE_OR_RETAIL"
echo $CMAKE_FLAGS
sleep 2

cmake -G Ninja $CMAKE_FLAGS ../neo -Wno-dev

# Copy compile_commands.json to root of project so VSCode can use it.
if [[ "$DEBUG_RELEASE_OR_RETAIL" == "Debug" ]]; then
    cp compile_commands.json ..
fi
