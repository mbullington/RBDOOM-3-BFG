#!/bin/zsh

# Install homebrew dependencies.
brew install wget cmake sdl2 openal-soft ffmpeg ninja vulkan-headers molten-vk

SHADERC_DIR=/opt/shaderc
if [ ! -d "$SHADERC_DIR" ]; then
    cd ~/Downloads
    wget https://storage.googleapis.com/shaderc/artifacts/prod/graphics_shader_compiler/shaderc/macos/continuous_clang_release/388/20220202-124410/install.tgz
    tar -xzvf install.tgz
    mv install /opt/shaderc
fi

echo "NEXT STEPS"
echo "Add /opt/shaderc/bin to your PATH"
echo "Add to your ~/.zshrc: export MOLTENVK_SDK=$(find /usr/local/Cellar/molten-vk/* | head -n 1)"