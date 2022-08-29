/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 2014-2020 Robert Beckebans

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
#include "vulkan/vulkan_core.h"
#pragma hdrstop

#include "../RenderCommon.h"
#include "../Framebuffer.h"

#if defined(USE_VULKAN)

Framebuffer::Framebuffer(const char* name, int w, int h) {
  fboName = name;

  frameBuffer = NULL;
  renderPass = NULL;

  colorImageView = NULL;
  depthImageView = NULL;

  commandBuffer = NULL;
  semaphore = NULL;

  colorFormat = VK_FORMAT_UNDEFINED;
  depthFormat = VK_FORMAT_UNDEFINED;

  width = w;
  height = h;

  framebuffers.Append(this);
}

Framebuffer::~Framebuffer() {
  if (renderPass != NULL) {
    vkDestroyRenderPass(vkcontext.device, renderPass, NULL);
  }

  if (frameBuffer != NULL) {
    vkDestroyFramebuffer(vkcontext.device, frameBuffer, NULL);
  }

  // commandBuffer is collected by the command pool

  if (semaphore != NULL) {
    vkDestroySemaphore(vkcontext.device, semaphore, NULL);
  }
}

void Framebuffer::Shutdown() { framebuffers.DeleteContents(true); }

void Framebuffer::Bind() {
  RENDERLOG_PRINTF("Framebuffer::Bind( %s )\n", fboName.c_str());
  tr.backend.currentFramebuffer = this;

  if (tr.backend.inRenderPass) {
    // begin command buffer
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    ID_VK_CHECK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = GetRenderPass();
    renderPassBeginInfo.framebuffer = GetFramebuffer();
    renderPassBeginInfo.renderArea.extent = {
        static_cast<uint32_t>(GetWidth()), static_cast<uint32_t>(GetHeight())};

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo,
                         VK_SUBPASS_CONTENTS_INLINE);
  }
}

bool Framebuffer::IsBound() { return (tr.backend.currentFramebuffer == this); }

void Framebuffer::Unbind() {
  if (Framebuffer::IsDefaultFramebufferActive()) return;
  Framebuffer* fb = tr.backend.currentFramebuffer;

  RENDERLOG_PRINTF("Framebuffer::Unbind()\n");
  tr.backend.currentFramebuffer = NULL;

  if (tr.backend.inRenderPass) {
    vkCmdEndRenderPass(fb->commandBuffer);
    ID_VK_CHECK(vkEndCommandBuffer(fb->commandBuffer))

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &fb->semaphore;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &fb->commandBuffer;
    ID_VK_CHECK(
        vkQueueSubmit(vkcontext.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));

    // Add semaphore to list for main command buffer.
    tr.backend.renderPassSemaphores.Append(fb->semaphore);
  }
}

bool Framebuffer::IsDefaultFramebufferActive() {
  return (tr.backend.currentFramebuffer == NULL);
}

Framebuffer* Framebuffer::GetActiveFramebuffer() {
  return tr.backend.currentFramebuffer;
}

void Framebuffer::AddColorBuffer(textureFormat_t format) {
  auto params = idImage::GetTextureParams(format);
  colorFormat = params.internalFormat;
}

void Framebuffer::AddDepthBuffer(textureFormat_t format) {
  auto params = idImage::GetTextureParams(format);
  depthFormat = params.internalFormat;
}

void Framebuffer::AttachImage2D(idImage* image, int mipmapLod) {
  colorImageView = image->GetView();
  image->opts.isRenderTarget = true;
}

void Framebuffer::AttachImageDepth(idImage* image) {
  depthImageView = image->GetView();
  image->opts.isRenderTarget = true;
}

void Framebuffer::AttachImageDepthLayer(idImage* image, int layer) {
  // TODO
  //   glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
  //   image->texnum,
  //                             0, layer);
  //   image->opts.isRenderTarget = true;
}

void Framebuffer::CreateRenderPass() {
  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = colorFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

  VkAttachmentReference colorRef = {};
  colorRef.attachment = 0;
  colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription depthAttachment = {};
  VkAttachmentReference depthRef = {};
  bool hasDepth = depthFormat != 0;
  if (hasDepth) {
    depthAttachment.format = depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    depthRef.attachment = 1;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  }

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorRef;
  if (hasDepth) {
    // Modify the subpass to include the depth attachment
    subpass.pDepthStencilAttachment = &depthRef;
  }

  VkRenderPassCreateInfo renderPassCreateInfo = {};
  renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassCreateInfo.subpassCount = 1;
  renderPassCreateInfo.pSubpasses = &subpass;
  renderPassCreateInfo.dependencyCount = 0;
  if (hasDepth) {
    renderPassCreateInfo.attachmentCount = 2;
    VkAttachmentDescription attachments[2] = {colorAttachment, depthAttachment};
    renderPassCreateInfo.pAttachments = attachments;
  } else {
    renderPassCreateInfo.attachmentCount = 1;
    VkAttachmentDescription attachments[1] = {colorAttachment};
    renderPassCreateInfo.pAttachments = attachments;
  }

  ID_VK_CHECK(vkCreateRenderPass(vkcontext.device, &renderPassCreateInfo, NULL,
                                 &renderPass));
}

void Framebuffer::CreateFramebuffer() {
  VkFramebufferCreateInfo frameBufferCreateInfo = {};
  frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  frameBufferCreateInfo.renderPass = renderPass;
  frameBufferCreateInfo.width = renderSystem->GetWidth();
  frameBufferCreateInfo.height = renderSystem->GetHeight();
  frameBufferCreateInfo.layers = 1;

  bool hasDepth = depthFormat != 0;
  if (hasDepth) {
    frameBufferCreateInfo.attachmentCount = 2;
    VkImageView attachments[2] = {colorImageView, depthImageView};
    frameBufferCreateInfo.pAttachments = attachments;
  } else {
    frameBufferCreateInfo.attachmentCount = 1;
    frameBufferCreateInfo.pAttachments = &colorImageView;
  }

  ID_VK_CHECK(vkCreateFramebuffer(vkcontext.device, &frameBufferCreateInfo,
                                  NULL, &frameBuffer));
}

void Framebuffer::CreateCommandBuffer() {
  VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
  commandBufferAllocateInfo.sType =
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  commandBufferAllocateInfo.commandPool = vkcontext.commandPool;
  commandBufferAllocateInfo.commandBufferCount = 1;

  ID_VK_CHECK(vkAllocateCommandBuffers(
      vkcontext.device, &commandBufferAllocateInfo, &commandBuffer));
}

void Framebuffer::CreateSemaphore() {
  VkSemaphoreCreateInfo semaphoreCreateInfo = {};
  semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  ID_VK_CHECK(vkCreateSemaphore(vkcontext.device, &semaphoreCreateInfo, NULL,
                                &semaphore));
}

void Framebuffer::Commit() {
  // In commit we'll actually create the framebuffer and renderpass.
  CreateRenderPass();
  CreateFramebuffer();
  CreateCommandBuffer();
  CreateSemaphore();
}

#endif  // #if !defined(USE_VULKAN)