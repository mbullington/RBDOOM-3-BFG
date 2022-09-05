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
#pragma hdrstop

#include "../RenderCommon.h"
#include "../Framebuffer.h"

namespace id {

Framebuffer::Framebuffer(int w, int h) {
  frameBuffer = NULL;
  renderPass = NULL;

  width = w;
  height = h;
}

Framebuffer::~Framebuffer() {
  // When we're done with these resources, we can add them to the Vulkan
  // deletion queue.
  vulkanDeletionQueue_t& deletionQueue =
      vkcontext.deletionQueue[vkcontext.frameParity];

  if (renderPass) deletionQueue.renderPasses.Append(renderPass);
  if (frameBuffer) deletionQueue.framebuffers.Append(frameBuffer);
}

void Framebuffer::Update(textureFormat_t format, idImage* image,
                         idImage* depthImage) {
  VkFormat vkFormat = idImage::GetTextureParams(format).internalFormat;

  image->opts.isRenderTarget = true;
  VkImageView vkImageView = image->GetView();

  VkImageView vkDepthImageView = VK_NULL_HANDLE;
  bool hasDepth = depthImage != NULL;
  if (hasDepth) {
    depthImage->opts.isRenderTarget = true;
    vkDepthImageView = depthImage->GetView();
  }

  VkUpdate(vkFormat, vkImageView, vkDepthImageView);
}

void Framebuffer::VkUpdate(VkFormat format, VkImageView imageView,
                           VkImageView depthImageView, bool isSwapImage) {
  this->isSwapImage = isSwapImage;
  bool hasDepth = depthImageView != VK_NULL_HANDLE;

  // Create the render pass
  {
    VkAttachmentDescription attachments[2] = {
        // Color
        {
            .format = format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_GENERAL,
        },
        // Depth
        {
            .format = vkcontext.depthFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        },
    };

    VkAttachmentReference colorRef = {};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef = {};
    depthRef.attachment = 1;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorRef,
    };
    if (hasDepth) {
      // Modify the subpass to include the depth attachment.
      subpass.pDepthStencilAttachment = &depthRef;
    }

    VkRenderPassCreateInfo renderPassCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = hasDepth ? 2u : 1u,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 0,
    };

    ID_VK_CHECK(vkCreateRenderPass(vkcontext.device, &renderPassCreateInfo,
                                   NULL, &renderPass));
  }

  // Create the framebuffer
  {
    VkImageView imageAttachments[2] = {imageView, depthImageView};
    VkFramebufferCreateInfo frameBufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = renderPass,
        .attachmentCount = hasDepth ? 2u : 1u,
        .pAttachments = imageAttachments,
        .width = static_cast<uint32_t>(width),
        .height = static_cast<uint32_t>(height),
        .layers = 1,
    };

    ID_VK_CHECK(vkCreateFramebuffer(vkcontext.device, &frameBufferCreateInfo,
                                    NULL, &frameBuffer));
  }
}

}  // namespace id