/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013-2020 Robert Beckebans

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

#include "../RenderLog.h"
#include "../RenderCommon.h"
#include "../CommandBuffer.h"

/*
================================================================================================
Contains the RenderLog implementation.
TODO: Emit statistics to the logfile at the end of views and frames.
================================================================================================
*/

idCVar r_logLevel("r_logLevel", "0", CVAR_INTEGER,
                  "1 = blocks only, 2 = everything", 1, 2);

static const int LOG_LEVEL_BLOCKS_ONLY = 1;
static const int LOG_LEVEL_EVERYTHING = 2;

const char* renderLogMainBlockLabels[] = {
    ASSERT_ENUM_STRING(MRB_GPU_TIME, 0),
    ASSERT_ENUM_STRING(MRB_BEGIN_DRAWING_VIEW, 1),
    ASSERT_ENUM_STRING(MRB_FILL_DEPTH_BUFFER, 2),
    ASSERT_ENUM_STRING(MRB_FILL_GEOMETRY_BUFFER, 3),  // RB
    ASSERT_ENUM_STRING(MRB_SSAO_PASS, 4),             // RB
    ASSERT_ENUM_STRING(MRB_AMBIENT_PASS, 5),          // RB
    ASSERT_ENUM_STRING(MRB_DRAW_INTERACTIONS, 6),
    ASSERT_ENUM_STRING(MRB_DRAW_SHADER_PASSES, 7),
    ASSERT_ENUM_STRING(MRB_FOG_ALL_LIGHTS, 8),
    ASSERT_ENUM_STRING(MRB_BLOOM, 9),
    ASSERT_ENUM_STRING(MRB_DRAW_SHADER_PASSES_POST, 10),
    ASSERT_ENUM_STRING(MRB_DRAW_DEBUG_TOOLS, 11),
    ASSERT_ENUM_STRING(MRB_CAPTURE_COLORBUFFER, 12),
    ASSERT_ENUM_STRING(MRB_POSTPROCESS, 13),
    ASSERT_ENUM_STRING(MRB_DRAW_GUI, 14),
    ASSERT_ENUM_STRING(MRB_TOTAL, 15)};

compile_time_assert(NUM_TIMESTAMP_QUERIES >= (MRB_TOTAL_QUERIES));

/*
================================================================================================

idRenderLog

================================================================================================
*/

idRenderLog renderLog;

// RB begin
/*
========================
idRenderLog::idRenderLog
========================
*/
idRenderLog::idRenderLog() {}

/*
========================
idRenderLog::OpenMainBlock
========================
*/
void idRenderLog::OpenMainBlock(renderLogMainBlock_t block) {
  // SRS - Use glConfig.timerQueryAvailable flag to control timestamp capture
  // for all platforms
  if (glConfig.timerQueryAvailable) {
    mainBlock = block;

    id::CommandBuffer* cmd = vklocal.currentCommandBuffer;
    if (!cmd) {
      common->Warning("idRenderLog::OpenMainBlock: no current command buffer");
      return;
    }

    if (vkcontext.queryIndex[vkcontext.frameParity] >=
        (NUM_TIMESTAMP_QUERIES - 1)) {
      return;
    }

    VkQueryPool queryPool = vkcontext.queryPools[vkcontext.frameParity];

    uint32 queryIndex =
        vkcontext.queryAssignedIndex[vkcontext.frameParity][mainBlock * 2 + 0] =
            vkcontext.queryIndex[vkcontext.frameParity]++;
    vkCmdWriteTimestamp(cmd->GetHandle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                        queryPool, queryIndex);
  }
}

/*
========================
idRenderLog::CloseMainBlock
========================
*/
void idRenderLog::CloseMainBlock() {
  // SRS - Use glConfig.timerQueryAvailable flag to control timestamp capture
  // for all platforms
  if (glConfig.timerQueryAvailable) {
    id::CommandBuffer* cmd = vklocal.currentCommandBuffer;
    if (!cmd) {
      common->Warning("idRenderLog::CloseMainBlock: no current command buffer");
      return;
    }

    if (vkcontext.queryIndex[vkcontext.frameParity] >=
        (NUM_TIMESTAMP_QUERIES - 1)) {
      return;
    }

    VkQueryPool queryPool = vkcontext.queryPools[vkcontext.frameParity];

    uint32 queryIndex =
        vkcontext.queryAssignedIndex[vkcontext.frameParity][mainBlock * 2 + 1] =
            vkcontext.queryIndex[vkcontext.frameParity]++;
    vkCmdWriteTimestamp(cmd->GetHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                        queryPool, queryIndex);
  }
}

/*
========================
idRenderLog::OpenBlock
========================
*/
void idRenderLog::OpenBlock(const char* szName, const idVec4& color) {
  if (r_logLevel.GetInteger() <= 0) {
    return;
  }

  id::CommandBuffer* cmd = vklocal.currentCommandBuffer;
  if (!cmd) {
    common->Warning("PC_BeginNamedEvent: no current command buffer");
    return;
  }

  // start an annotated group of calls under the this name
  // SRS - Prefer VK_EXT_debug_utils over
  // VK_EXT_debug_marker/VK_EXT_debug_report (deprecated by VK_EXT_debug_utils)
  if (vkcontext.debugUtilsSupportAvailable) {
    VkDebugUtilsLabelEXT label = {};
    label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    label.pLabelName = szName;
    label.color[0] = color.x;
    label.color[1] = color.y;
    label.color[2] = color.z;
    label.color[3] = color.w;

    qvkCmdBeginDebugUtilsLabelEXT(cmd->GetHandle(), &label);
  } else if (vkcontext.debugMarkerSupportAvailable) {
    VkDebugMarkerMarkerInfoEXT label = {};
    label.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
    label.pMarkerName = szName;
    label.color[0] = color.x;
    label.color[1] = color.y;
    label.color[2] = color.z;
    label.color[3] = color.w;

    qvkCmdDebugMarkerBeginEXT(cmd->GetHandle(), &label);
  }
}

/*
========================
idRenderLog::CloseBlock
========================
*/
void idRenderLog::CloseBlock() {
  if (r_logLevel.GetInteger() <= 0) {
    return;
  }

  id::CommandBuffer* cmd = vklocal.currentCommandBuffer;
  if (!cmd) {
    common->Warning("idRenderLog::CloseBlock: no current command buffer");
    return;
  }

  // SRS - Prefer VK_EXT_debug_utils over
  // VK_EXT_debug_marker/VK_EXT_debug_report (deprecated by VK_EXT_debug_utils)
  if (vkcontext.debugUtilsSupportAvailable) {
    qvkCmdEndDebugUtilsLabelEXT(cmd->GetHandle());
  } else if (vkcontext.debugMarkerSupportAvailable) {
    qvkCmdDebugMarkerEndEXT(cmd->GetHandle());
  }
}
