#!/bin/zsh

# Install homebrew dependencies.
brew install wget cmake sdl2 openal-soft ninja vulkan-headers molten-vk fzf

echo "NEXT STEPS"
echo "Add to your ~/.zshrc: export MOLTENVK_SDK=$(find /usr/local/Cellar/molten-vk/* | head -n 1)"