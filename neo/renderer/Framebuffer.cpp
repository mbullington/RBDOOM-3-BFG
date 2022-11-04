/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 2014-2018 Robert Beckebans
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

#include "RenderCommon.h"
#include "Framebuffer.h"

namespace id {

BufferedFramebuffer::BufferedFramebuffer(int width, int height) {
  this->width = width;
  this->height = height;

  for (int i = 0; i < NUM_FRAME_DATA; i++) {
    _frames[i] = new Framebuffer(width, height);
  }
}

BufferedFramebuffer::~BufferedFramebuffer() {
  for (int i = 0; i < NUM_FRAME_DATA; i++) {
    delete _frames[i];
  }
}

void BufferedFramebuffer::Resize(int width, int height) {
  this->width = width;
  this->height = height;

  for (int i = 0; i < NUM_FRAME_DATA; i++) {
    _frames[i]->Resize(width, height);
  }
}

void BufferedFramebuffer::Update(textureFormat_t format,
                                 RefPtr<idBufferedImage> image,
                                 RefPtr<idBufferedImage> depthImage) {
  for (int i = 0; i < NUM_FRAME_DATA; i++) {
    _frames[i]->Update(format, image->Get(i),
                       depthImage == NULL ? NULL : depthImage->Get(i));
  }
}

}  // namespace id