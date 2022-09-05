/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 2014-2016 Robert Beckebans
Copyright (C) 2022 Michael Bullington

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition
Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your option)
any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see
<http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain
additional terms. You should have received a copy of these additional terms
immediately following the terms and conditions of the GNU General Public License
which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a
copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional
terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite
120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#pragma once

#include "RenderFwd.h"

#if defined(USE_VULKAN)
#include <vulkan/vulkan.h>
#endif

namespace id {

enum commandBufferOptions_t {
  CMD_BUF_OPT_NONE = 0,
  // This flag needs to be set for any command buffer created with 'new',
  // outside of a render pass.
  //
  // Generally, command buffers should be ephemeral, and only saved in specific
  // instances (like postprocessing), or where we *know* we're going to be
  // submitting the same thing every time.
  //
  // This flag trades the ability for the command buffer to be created outside
  // of a render pass, for the ability to not re-record inside of one.
  CMD_BUF_OPT_HEAP_ALLOCATED = BIT(1),
  // This flag is needed for CommandBuffer to create a Fence object on Vulkan.
  // This is used for CPU synchronization, but may unnecessary for the
  // majority of CommandBuffers.
  CMD_BUF_OPT_CREATE_FENCE = BIT(2),
  // This flag will skip creating the Semaphore object on Vulkan, meaning this
  // object will have no dependencies.
  //
  // Usually this should be the "last link" in the chain (unless you're using
  // swap), and not having this flag will create Vulkan API validation errors.
  CMD_BUF_OPT_SKIP_SEMAPHORE = BIT(3)
};

class CommandBuffer {
 public:
  CommandBuffer(CommandBuffer **dependencies, size_t numDependencies,
                uint8_t opts = CMD_BUF_OPT_NONE);

  CommandBuffer(uint8_t opts = CMD_BUF_OPT_NONE)
      : CommandBuffer(NULL, 0, opts) {}

  virtual ~CommandBuffer();

  void Bind(Framebuffer *frameBuffer);
  void Unbind();

  void Begin();
  void End();
  void MakeActive();

  void SetDependencies(CommandBuffer **dependencies, short numDependencies);
  void Submit();

  Framebuffer *GetFramebuffer() const { return frameBuffer; }

#if defined(USE_VULKAN)
  VkCommandBuffer GetHandle() const { return handle; }
  VkSemaphore GetSemaphore() const { return semaphore; }
  VkFence GetFence() const { return fence; }
#endif

 private:
  bool isRecording;
  bool isBound;
  bool isHeapAllocated;

  int frameParity;
  bool waitOnSwapAcquire;

  bool shouldCreateFence;
  bool shouldSkipSemaphore;

  idList<CommandBuffer *> dependencies;
  Framebuffer *frameBuffer;

#if defined(USE_VULKAN)
  VkCommandBuffer handle;
  VkSemaphore semaphore;
  VkFence fence;
#endif
};

}  // namespace id