/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 2014-2018 Robert Beckebans
Copyright (C) 2022 Stephen Pridham

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

#include "RenderCommon.h"
#include "Framebuffer.h"

namespace id {

globalFramebuffers_t globalFramebuffers;

void Framebuffer::Init() {
  int screenWidth = renderSystem->GetWidth();
  int screenHeight = renderSystem->GetHeight();

  globalFramebuffers.hdrFBO = new Framebuffer(screenWidth, screenHeight);
  globalFramebuffers.hdrFBO->Update(FMT_RGBA16F,
                                    globalImages->currentRenderHDRImage,
                                    globalImages->currentDepthImage);
}

void Framebuffer::Shutdown() { globalFramebuffers.hdrFBO = NULL; }

void Framebuffer::CheckFramebuffers() {
  int screenWidth = renderSystem->GetWidth();
  int screenHeight = renderSystem->GetHeight();

  // Check for HDR framebuffer.
  Framebuffer *hdrFBO = globalFramebuffers.hdrFBO;
  if (hdrFBO->GetWidth() != screenWidth ||
      hdrFBO->GetHeight() != screenHeight) {
    globalImages->currentRenderHDRImage->Resize(screenWidth, screenHeight);
    globalImages->currentDepthImage->Resize(screenWidth, screenHeight);

    globalFramebuffers.hdrFBO->Resize(screenWidth, screenHeight);
    globalFramebuffers.hdrFBO->Update(FMT_RGBA16F,
                                      globalImages->currentRenderHDRImage,
                                      globalImages->currentDepthImage);
  }
}

}  // namespace id