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

#if !defined(USE_VULKAN)

Framebuffer::Framebuffer(const char* name, int w, int h) {
  fboName = name;

  frameBuffer = 0;

  colorBuffer = 0;
  colorFormat = 0;

  depthBuffer = 0;
  depthFormat = 0;

  width = w;
  height = h;

  glGenFramebuffers(1, &frameBuffer);

  framebuffers.Append(this);
}

Framebuffer::~Framebuffer() { glDeleteFramebuffers(1, &frameBuffer); }

void Framebuffer::Shutdown() { framebuffers.DeleteContents(true); }

void Framebuffer::Bind() {
  RENDERLOG_PRINTF("Framebuffer::Bind( %s )\n", fboName.c_str());

  if (tr.backend.currentFramebuffer != this) {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    tr.backend.currentFramebuffer = this;
  }
}

bool Framebuffer::IsBound() { return (tr.backend.currentFramebuffer == this); }

void Framebuffer::Unbind() {
  RENDERLOG_PRINTF("Framebuffer::Unbind()\n");

  // if(tr.backend.framebuffer != NULL)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    tr.backend.currentFramebuffer = NULL;
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

  bool notCreatedYet = colorBuffer == 0;
  if (notCreatedYet) {
    glGenRenderbuffers(1, &colorBuffer);
  }

  glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);

  if (notCreatedYet) {
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                              GL_RENDERBUFFER, colorBuffer);
  }

  GL_CheckErrors();
}

void Framebuffer::AddDepthBuffer(textureFormat_t format) {
  auto params = idImage::GetTextureParams(format);
  depthFormat = params.internalFormat;

  bool notCreatedYet = depthBuffer == 0;
  if (notCreatedYet) {
    glGenRenderbuffers(1, &depthBuffer);
  }

  glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);

  if (notCreatedYet) {
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, depthBuffer);
  }

  GL_CheckErrors();
}

void Framebuffer::AttachImage2D(idImage* image, int mipmapLod) {
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         image->texnum, mipmapLod);

  image->opts.isRenderTarget = true;
}

void Framebuffer::AttachImageDepth(idImage* image) {
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                         GL_TEXTURE_2D, image->texnum, 0);

  image->opts.isRenderTarget = true;
}

void Framebuffer::AttachImageDepthLayer(idImage* image, int layer) {
  glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, image->texnum,
                            0, layer);

  image->opts.isRenderTarget = true;
}

void Framebuffer::Commit() {
  int prev;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev);

  glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

  int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status == GL_FRAMEBUFFER_COMPLETE) {
    glBindFramebuffer(GL_FRAMEBUFFER, prev);
    return;
  }

  // something went wrong
  switch (status) {
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      common->Error(
          "Framebuffer::Check( %s ): Framebuffer incomplete, incomplete "
          "attachment",
          fboName.c_str());
      break;

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      common->Error(
          "Framebuffer::Check( %s ): Framebuffer incomplete, missing "
          "attachment",
          fboName.c_str());
      break;

    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      common->Error(
          "Framebuffer::Check( %s ): Framebuffer incomplete, missing draw "
          "buffer",
          fboName.c_str());
      break;

    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      common->Error(
          "Framebuffer::Check( %s ): Framebuffer incomplete, missing read "
          "buffer",
          fboName.c_str());
      break;

    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
      common->Error(
          "Framebuffer::Check( %s ): Framebuffer incomplete, missing layer "
          "targets",
          fboName.c_str());
      break;

    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
      common->Error(
          "Framebuffer::Check( %s ): Framebuffer incomplete, missing "
          "multisample",
          fboName.c_str());
      break;

    case GL_FRAMEBUFFER_UNSUPPORTED:
      common->Error("Framebuffer::Check( %s ): Unsupported framebuffer format",
                    fboName.c_str());
      break;

    default:
      common->Error("Framebuffer::Check( %s ): Unknown error 0x%X",
                    fboName.c_str(), status);
      break;
  };

  glBindFramebuffer(GL_FRAMEBUFFER, prev);
}

#endif  // #if !defined(USE_VULKAN)