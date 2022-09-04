/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 2014-2020 Robert Beckebans
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

#include "precompiled.h"
#pragma hdrstop

#include <vulkan/vulkan.h>
#include <optional>
#include <memory>

#include "../RenderCommon.h"
#include "../CommandBuffer.h"

namespace id {

CommandBuffer::CommandBuffer(optional<CommandBuffer **> dependencies,
                             short numDependencies,
                             commandBufferOptions_t opts) {
  isRecording = false;
  isBound = false;

  frameParity = -1;

  shouldCreateFence = opts & CMD_BUF_OPT_CREATE_FENCE;
  shouldAllowMultipleRecordsPerFrame =
      opts & CMD_BUF_OPT_ALLOW_MULTIPLE_RECORDS_PER_FRAME;

  waitOnSwapAcquire = false;
  if (numDependencies > 0) {
    this->dependencies.Append(*dependencies, numDependencies);
  }

  assert(vkcontext.device);
  assert(vkcontext.commandPool);

  // Create the command buffer.
  {
    VkCommandBufferAllocateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = vkcontext.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    ID_VK_CHECK(vkAllocateCommandBuffers(vkcontext.device, &info, &handle));
  }

  // Create the semaphore.
  {
    VkSemaphoreCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    ID_VK_CHECK(vkCreateSemaphore(vkcontext.device, &info, NULL, &semaphore));
  }

  // Create the fence.
  if (shouldCreateFence) {
    VkFenceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };

    vkCreateFence(vkcontext.device, &info, NULL, &fence);
  } else {
    fence = NULL;
  }
}

CommandBuffer::~CommandBuffer() {
  // When we're done with these resources, we can add them to the Vulkan
  // deletion queue.
  //
  // Deletion happens at the end of the frame.
  // TODO(mbullington): What happens here with dependencies? Should we reference
  // count?
  vulkanDeletionQueue_t &deletionQueue =
      vkcontext.deletionQueue[vkcontext.frameParity];

  if (handle) deletionQueue.commandBuffers.Append(handle);
  if (semaphore) deletionQueue.semaphores.Append(semaphore);
  if (fence) deletionQueue.fences.Append(fence);
}

void CommandBuffer::Bind(id::Framebuffer *frameBuffer) {
  if (!isRecording) {
    common->Warning("CommandBuffer::Unbind: Command buffer is not recording");
    return;
  }
  if (isBound) {
    common->Warning("CommandBuffer::Bind: Command buffer is already bound");
    return;
  }
  isBound = true;

  // Make sure regardless of Unbind, this remains true.
  if (frameBuffer->isSwapImage) waitOnSwapAcquire = true;

  VkRenderPassBeginInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  info.renderPass = frameBuffer->renderPass;
  info.framebuffer = frameBuffer->frameBuffer;
  info.renderArea.extent = {static_cast<uint32_t>(frameBuffer->width),
                            static_cast<uint32_t>(frameBuffer->height)};

  assert(handle);
  vkCmdBeginRenderPass(handle, &info, VK_SUBPASS_CONTENTS_INLINE);

  // Set the scissor.
  {
    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = {static_cast<uint32_t>(frameBuffer->width),
                      static_cast<uint32_t>(frameBuffer->height)};

    vkCmdSetScissor(handle, 0, 1, &scissor);
  }
}

void CommandBuffer::Unbind() {
  if (!isRecording) {
    common->Warning("CommandBuffer::Unbind: Command buffer is not recording");
    return;
  }
  if (!isBound) {
    common->Warning("CommandBuffer::Unbind: Command buffer not bound");
    return;
  }
  isBound = false;

  vkCmdEndRenderPass(handle);
}

void CommandBuffer::Begin() {
  if (isRecording) {
    common->Warning("CommandBuffer::Begin: Already recording");
    return;
  }

  if (frameParity == vkcontext.frameParity &&
      !shouldAllowMultipleRecordsPerFrame) {
    common->FatalError("CommandBuffer::Begin: Already recorded this frame");
    return;
  }

  isRecording = true;

  assert(handle);

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  ID_VK_CHECK(vkBeginCommandBuffer(handle, &beginInfo));
}

void CommandBuffer::End() {
  if (!isRecording) {
    common->Warning("CommandBuffer::End: Not recording");
    return;
  }

  // Allow for implicit unbinding of the command buffer.
  if (isBound) {
    Unbind();
  }

  isRecording = false;

  assert(handle);
  ID_VK_CHECK(vkEndCommandBuffer(handle));
}

void CommandBuffer::MakeActive() { vkcontext.currentCommandBuffer = this; }

void CommandBuffer::Submit(optional<CommandBuffer **> dependencies,
                           short numDependencies) {
  if (isRecording) {
    End();
  }

  idList<VkSemaphore> waitSemaphores;
  waitSemaphores.Resize(this->dependencies.Num() + numDependencies);
  for (auto &dependency : this->dependencies) {
    waitSemaphores.Append(dependency->semaphore);
  }
  for (short i = 0; i < numDependencies; i++) {
    waitSemaphores.Append((*dependencies)[i]->semaphore);
  }

  if (waitOnSwapAcquire) {
    if (!tr.backend.inRenderPass) {
      common->Error("CommandBuffer::Submit: Waiting on swap acquire %s",
                    "but not in a render pass");
      return;
    } else {
      waitSemaphores.Append(vkcontext.acquireSemaphores[vkcontext.frameParity]);
    }
  }

  VkPipelineStageFlags dstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.waitSemaphoreCount = waitSemaphores.Num();
  submitInfo.pWaitSemaphores = waitSemaphores.Ptr();
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &semaphore;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &handle;
  submitInfo.pWaitDstStageMask = &dstStageMask;

  ID_VK_CHECK(
      vkQueueSubmit(vkcontext.graphicsQueue, 1, &submitInfo, this->fence));

  // Reset our internal flag here.
  frameParity = -1;
}

}  // namespace id
