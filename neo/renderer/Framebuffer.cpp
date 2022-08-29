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

idList<Framebuffer*> Framebuffer::framebuffers;

globalFramebuffers_t globalFramebuffers;

Framebuffer* Framebuffer::Find(const char* name) {
  for (int i = 0; i < framebuffers.Num(); i++) {
    if (idStr::Icmp(framebuffers[i]->fboName, name) == 0) {
      return framebuffers[i];
    }
  }

  return nullptr;
}

void Framebuffer::Init() {
  // tr.backend.currentFramebuffer = NULL;
  // TODO: SHADOWMAPS

  // int width, height;
  // width = height = r_shadowMapImageSize.GetInteger();

  // for (int i = 0; i < MAX_SHADOWMAP_RESOLUTIONS; i++) {
  //   width = height = shadowMapResolutions[i];

  //   globalFramebuffers.shadowFBO[i] =
  //       new Framebuffer(va("_shadowMap%i", i), width, height);
  //   globalFramebuffers.shadowFBO[i]->Bind();
  //   glDrawBuffers(0, NULL);
  // }

  // HDR

  int screenWidth = renderSystem->GetWidth();
  int screenHeight = renderSystem->GetHeight();

  globalFramebuffers.hdrFBO =
      new Framebuffer("_hdr", screenWidth, screenHeight);
  globalFramebuffers.hdrFBO->Bind();

  globalFramebuffers.hdrFBO->AddColor(FMT_RGBA16F,
                                      globalImages->currentRenderHDRImage);
  globalFramebuffers.hdrFBO->AddDepth(globalImages->currentDepthImage);

  globalFramebuffers.hdrFBO->Commit();

  // HDR CUBEMAP CAPTURE

  globalFramebuffers.envprobeFBO = new Framebuffer(
      "_envprobeRender", ENVPROBE_CAPTURE_SIZE, ENVPROBE_CAPTURE_SIZE);
  globalFramebuffers.envprobeFBO->Bind();
  globalFramebuffers.envprobeFBO->AddColor(FMT_RGBA16F,
                                           globalImages->envprobeHDRImage);
  globalFramebuffers.envprobeFBO->AddDepth(globalImages->envprobeDepthImage);
  globalFramebuffers.envprobeFBO->Commit();

  // HDR DOWNSCALE

  globalFramebuffers.hdr64FBO = new Framebuffer("_hdr64", 64, 64);
  globalFramebuffers.hdr64FBO->Bind();
  globalFramebuffers.hdr64FBO->AddColor(FMT_RGBA16F,
                                        globalImages->currentRenderHDRImage64);
  globalFramebuffers.hdr64FBO->Commit();

  // BLOOM

  for (int i = 0; i < MAX_BLOOM_BUFFERS; i++) {
    globalFramebuffers.bloomRenderFBO[i] =
        new Framebuffer(va("_bloomRender%i", i), screenWidth, screenHeight);
    globalFramebuffers.bloomRenderFBO[i]->Bind();
    globalFramebuffers.bloomRenderFBO[i]->AddColor(
        FMT_RGBA8, globalImages->bloomRenderImage[i]);
    globalFramebuffers.bloomRenderFBO[i]->Commit();
  }

  // AMBIENT OCCLUSION

  for (int i = 0; i < MAX_SSAO_BUFFERS; i++) {
    globalFramebuffers.ambientOcclusionFBO[i] =
        new Framebuffer(va("_aoRender%i", i), screenWidth, screenHeight);
    globalFramebuffers.ambientOcclusionFBO[i]->Bind();
    globalFramebuffers.ambientOcclusionFBO[i]->AddColor(
        FMT_RGBA8, globalImages->ambientOcclusionImage[i]);
    globalFramebuffers.ambientOcclusionFBO[i]->Commit();
  }

  // HIERARCHICAL Z BUFFER

  for (int i = 0; i < MAX_HIERARCHICAL_ZBUFFERS; i++) {
    globalFramebuffers.csDepthFBO[i] = new Framebuffer(
        va("_csz%i", i), screenWidth / (1 << i), screenHeight / (1 << i));
    globalFramebuffers.csDepthFBO[i]->Bind();
    globalFramebuffers.csDepthFBO[i]->AddColor(
        FMT_R32F, globalImages->hierarchicalZbufferImage);
    globalFramebuffers.csDepthFBO[i]->Commit();
  }

  // GEOMETRY BUFFER

  globalFramebuffers.geometryBufferFBO =
      new Framebuffer("_gbuffer", screenWidth, screenHeight);
  globalFramebuffers.geometryBufferFBO->Bind();

  // it is ideal to share the depth buffer between the HDR main context and the
  // geometry render target
  globalFramebuffers.geometryBufferFBO->AddColor(
      FMT_RGBA16F, globalImages->currentNormalsImage);
  globalFramebuffers.geometryBufferFBO->AddDepth(
      globalImages->currentDepthImage);

  globalFramebuffers.geometryBufferFBO->Commit();

  // SMAA

  globalFramebuffers.smaaEdgesFBO =
      new Framebuffer("_smaaEdges", screenWidth, screenHeight);
  globalFramebuffers.smaaEdgesFBO->Bind();
  globalFramebuffers.smaaEdgesFBO->AddColor(FMT_RGBA8,
                                            globalImages->smaaEdgesImage);
  globalFramebuffers.smaaEdgesFBO->Commit();

  globalFramebuffers.smaaBlendFBO =
      new Framebuffer("_smaaBlend", screenWidth, screenHeight);
  globalFramebuffers.smaaBlendFBO->Bind();
  globalFramebuffers.smaaBlendFBO->AddColor(FMT_RGBA8,
                                            globalImages->smaaBlendImage);
  globalFramebuffers.smaaBlendFBO->Commit();

  Unbind();
}

void Framebuffer::CheckFramebuffers() {
  int screenWidth = renderSystem->GetWidth();
  int screenHeight = renderSystem->GetHeight();

  if (globalFramebuffers.hdrFBO->GetWidth() != screenWidth ||
      globalFramebuffers.hdrFBO->GetHeight() != screenHeight) {
    Unbind();

    // HDR
    globalImages->currentRenderHDRImage->Resize(screenWidth, screenHeight);
    globalImages->currentDepthImage->Resize(screenWidth, screenHeight);

    globalFramebuffers.hdrFBO->width = screenWidth;
    globalFramebuffers.hdrFBO->height = screenHeight;

    // BLOOM

    for (int i = 0; i < MAX_BLOOM_BUFFERS; i++) {
      globalImages->bloomRenderImage[i]->Resize(screenWidth / 4,
                                                screenHeight / 4);

      globalFramebuffers.bloomRenderFBO[i]->width = screenWidth / 4;
      globalFramebuffers.bloomRenderFBO[i]->height = screenHeight / 4;
    }

    // AMBIENT OCCLUSION

    for (int i = 0; i < MAX_SSAO_BUFFERS; i++) {
      globalImages->ambientOcclusionImage[i]->Resize(screenWidth, screenHeight);

      globalFramebuffers.ambientOcclusionFBO[i]->width = screenWidth;
      globalFramebuffers.ambientOcclusionFBO[i]->height = screenHeight;
    }

    // HIERARCHICAL Z BUFFER

    globalImages->hierarchicalZbufferImage->Resize(screenWidth, screenHeight);

    for (int i = 0; i < MAX_HIERARCHICAL_ZBUFFERS; i++) {
      globalFramebuffers.csDepthFBO[i]->width = screenWidth / (1 << i);
      globalFramebuffers.csDepthFBO[i]->height = screenHeight / (1 << i);
    }

    // GEOMETRY BUFFER

    globalImages->currentNormalsImage->Resize(screenWidth, screenHeight);

    globalFramebuffers.geometryBufferFBO->width = screenWidth;
    globalFramebuffers.geometryBufferFBO->height = screenHeight;

    // SMAA

    globalImages->smaaEdgesImage->Resize(screenWidth, screenHeight);

    globalFramebuffers.smaaEdgesFBO->width = screenWidth;
    globalFramebuffers.smaaEdgesFBO->height = screenHeight;

    globalImages->smaaBlendImage->Resize(screenWidth, screenHeight);

    globalFramebuffers.smaaBlendFBO->width = screenWidth;
    globalFramebuffers.smaaBlendFBO->height = screenHeight;

    Unbind();
  }
}