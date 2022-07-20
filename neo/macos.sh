#!/bin/zsh

cd ..
rm -rf build
mkdir build
cd build

CMAKE_FLAGS=()

# XCode is useful for the amazing Metal debugger.
NINJA_OR_XCODE=$((echo "Ninja" && echo "XCode") | fzf)
if [[ "$NINJA_OR_XCODE" == "Ninja" ]]; then
    CMAKE_FLAGS+=(-G Ninja)
else
    CMAKE_FLAGS+=(-G XCode)
fi

VULKAN_OR_OPENGL=$((echo "Vulkan" && echo "OpenGL") | fzf)
if [[ "$VULKAN_OR_OPENGL" == "Vulkan" ]]; then
    CMAKE_FLAGS+=(-DUSE_VULKAN=ON -DUSE_MoltenVK=ON)
fi

DEBUG_RELEASE_OR_RETAIL=$((echo "Debug" && echo "Release" && echo "Retail") | fzf)
if [[ "$DEBUG_RELEASE_OR_RETAIL" == "Debug" ]]; then
    CMAKE_FLAGS+=(-DCOMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug)
elif [[ "$DEBUG_RELEASE_OR_RETAIL" == "Release" ]]; then
    CMAKE_FLAGS+=(-DCMAKE_BUILD_TYPE=Release)
else
    CMAKE_FLAGS+=(-DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS_RELEASE="-DID_RETAIL")
fi

TESTS_OR_NOT=$((echo "No tests" && echo "Tests") | fzf)
if [[ "$TESTS_OR_NOT" == "Tests" ]]; then
    CMAKE_FLAGS+=(-DRUN_TESTS=ON)
fi

# Stuff that's generally useful.
CMAKE_FLAGS+=(
    # change or remove -DCMAKE_OSX_DEPLOYMENT_TARGET=<version> to match supported runtime targets
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.12
    -DOPENAL_LIBRARY=/usr/local/opt/openal-soft/lib/libopenal.dylib
    -DOPENAL_INCLUDE_DIR=/usr/local/opt/openal-soft/include
)

echo "$NINJA_OR_XCODE $VULKAN_OR_OPENGL $DEBUG_RELEASE_OR_RETAIL $TESTS_OR_NOT"
echo $CMAKE_FLAGS
sleep 2

cmake $CMAKE_FLAGS ../neo -Wno-dev

# Copy compile_commands.json to root of project so VSCode can use it.
if [[ "$DEBUG_RELEASE_OR_RETAIL" == "Debug" ]]; then
    cp compile_commands.json ..
fi
