/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013-2018 Robert Beckebans
Copyright (C) 2016-2017 Dustin Land

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

#include <memory>

#include "Dict.h"
#include "Lexer.h"
#include "RenderCommon.h"
#include "renderer/RenderProgs.h"
#include "framework/Common.h"
#include "framework/FileSystem.h"

// For GLSL we need to have the names for the renderparms so we can look up
// their run time indices within the renderprograms
static const char* GLSLParmNames[RENDERPARM_TOTAL] = {
    "rpScreenCorrectionFactor", "rpWindowCoord", "rpDiffuseModifier",
    "rpSpecularModifier",

    "rpLocalLightOrigin", "rpLocalViewOrigin",

    "rpLightProjectionS", "rpLightProjectionT", "rpLightProjectionQ",
    "rpLightFalloffS",

    "rpBumpMatrixS", "rpBumpMatrixT",

    "rpDiffuseMatrixS", "rpDiffuseMatrixT",

    "rpSpecularMatrixS", "rpSpecularMatrixT",

    "rpVertexColorModulate", "rpVertexColorAdd",

    "rpColor", "rpViewOrigin", "rpGlobalEyePos",

    "rpMVPmatrixX", "rpMVPmatrixY", "rpMVPmatrixZ", "rpMVPmatrixW",

    "rpModelMatrixX", "rpModelMatrixY", "rpModelMatrixZ", "rpModelMatrixW",

    "rpProjectionMatrixX", "rpProjectionMatrixY", "rpProjectionMatrixZ",
    "rpProjectionMatrixW",

    "rpModelViewMatrixX", "rpModelViewMatrixY", "rpModelViewMatrixZ",
    "rpModelViewMatrixW",

    "rpTextureMatrixS", "rpTextureMatrixT",

    "rpTexGen0S", "rpTexGen0T", "rpTexGen0Q", "rpTexGen0Enabled",

    "rpTexGen1S", "rpTexGen1T", "rpTexGen1Q", "rpTexGen1Enabled",

    "rpWobbleSkyX", "rpWobbleSkyY", "rpWobbleSkyZ",

    "rpOverbright", "rpEnableSkinning", "rpAlphaTest",

    // RB begin
    "rpAmbientColor",

    "rpGlobalLightOrigin", "rpJitterTexScale", "rpJitterTexOffset",
    "rpCascadeDistances",

    "rpShadowMatrices", "rpShadowMatrix0Y", "rpShadowMatrix0Z",
    "rpShadowMatrix0W",

    "rpShadowMatrix1X", "rpShadowMatrix1Y", "rpShadowMatrix1Z",
    "rpShadowMatrix1W",

    "rpShadowMatrix2X", "rpShadowMatrix2Y", "rpShadowMatrix2Z",
    "rpShadowMatrix2W",

    "rpShadowMatrix3X", "rpShadowMatrix3Y", "rpShadowMatrix3Z",
    "rpShadowMatrix3W",

    "rpShadowMatrix4X", "rpShadowMatrix4Y", "rpShadowMatrix4Z",
    "rpShadowMatrix4W",

    "rpShadowMatrix5X", "rpShadowMatrix5Y", "rpShadowMatrix5Z",
    "rpShadowMatrix5W",

    "rpUser0", "rpUser1", "rpUser2", "rpUser3", "rpUser4", "rpUser5", "rpUser6",
    "rpUser7"
    // RB end
};

// RB begin
const char* idRenderProgManager::GLSLMacroNames[MAX_SHADER_MACRO_NAMES] = {
    "USE_GPU_SKINNING", "LIGHT_POINT", "LIGHT_PARALLEL", "BRIGHTPASS",
    "HDR_DEBUG",        "USE_SRGB",    "USE_PBR"};
// RB end

const char* vertexInsert = {
// SRS - OSX OpenGL only supports up to GLSL 4.1, but current RenderProgs
// shaders seem to work as-is on OSX OpenGL drivers
#if defined(__APPLE__) && !defined(USE_VULKAN)
    "#version 410\n"
#else
    "#version 450\n"
#endif
    "#pragma shader_stage( vertex )\n"
    "#extension GL_ARB_separate_shader_objects : enable\n"
    //"#define PC\n"
    "\n"
    //"float saturate( float v ) { return clamp( v, 0.0, 1.0 ); }\n"
    //"vec2 saturate( vec2 v ) { return clamp( v, 0.0, 1.0 ); }\n"
    //"vec3 saturate( vec3 v ) { return clamp( v, 0.0, 1.0 ); }\n"
    //"vec4 saturate( vec4 v ) { return clamp( v, 0.0, 1.0 ); }\n"
    //"vec4 tex2Dlod( sampler2D sampler, vec4 texcoord ) { return textureLod(
    // sampler, texcoord.xy, texcoord.w ); }\n"
    //"\n"
};

const char* fragmentInsert = {
// SRS - OSX OpenGL only supports up to GLSL 4.1, but current RenderProgs
// shaders seem to work as-is on OSX OpenGL drivers
#if defined(__APPLE__) && !defined(USE_VULKAN)
    "#version 410\n"
#else
    "#version 450\n"
#endif
    "#pragma shader_stage( fragment )\n"
    "#extension GL_ARB_separate_shader_objects : enable\n"
    //"#define PC\n"
    "\n"
    "void clip( float v ) { if ( v < 0.0 ) { discard; } }\n"
    "void clip( vec2 v ) { if ( any( lessThan( v, vec2( 0.0 ) ) ) ) { discard; "
    "} }\n"
    "void clip( vec3 v ) { if ( any( lessThan( v, vec3( 0.0 ) ) ) ) { discard; "
    "} }\n"
    "void clip( vec4 v ) { if ( any( lessThan( v, vec4( 0.0 ) ) ) ) { discard; "
    "} }\n"
    "\n"
    "float saturate( float v ) { return clamp( v, 0.0, 1.0 ); }\n"
    "vec2 saturate( vec2 v ) { return clamp( v, 0.0, 1.0 ); }\n"
    "vec3 saturate( vec3 v ) { return clamp( v, 0.0, 1.0 ); }\n"
    "vec4 saturate( vec4 v ) { return clamp( v, 0.0, 1.0 ); }\n"
    "\n"
    //"vec4 tex2D( sampler2D sampler, vec2 texcoord ) { return texture( sampler,
    // texcoord.xy ); }\n" "vec4 tex2D( sampler2DShadow sampler, vec3 texcoord )
    //{ return vec4( texture( sampler, texcoord.xyz ) ); }\n"
    //"\n"
    //"vec4 tex2D( sampler2D sampler, vec2 texcoord, vec2 dx, vec2 dy ) { return
    // textureGrad( sampler, texcoord.xy, dx, dy ); }\n" "vec4 tex2D(
    // sampler2DShadow sampler, vec3 texcoord, vec2 dx, vec2 dy ) { return vec4(
    // textureGrad( sampler, texcoord.xyz, dx, dy ) ); }\n"
    //"\n"
    //"vec4 texCUBE( samplerCube sampler, vec3 texcoord ) { return texture(
    // sampler, texcoord.xyz ); }\n" "vec4 texCUBE( samplerCubeShadow sampler,
    // vec4 texcoord ) { return vec4( texture( sampler, texcoord.xyzw ) ); }\n"
    //"\n"
    //"vec4 tex1Dproj( sampler1D sampler, vec2 texcoord ) { return textureProj(
    // sampler, texcoord ); }\n" "vec4 tex2Dproj( sampler2D sampler, vec3
    // texcoord ) { return textureProj( sampler, texcoord ); }\n" "vec4
    // tex3Dproj( sampler3D sampler, vec4 texcoord ) { return textureProj(
    // sampler, texcoord ); }\n"
    //"\n"
    //"vec4 tex1Dbias( sampler1D sampler, vec4 texcoord ) { return texture(
    // sampler, texcoord.x, texcoord.w ); }\n" "vec4 tex2Dbias( sampler2D
    // sampler, vec4 texcoord ) { return texture( sampler, texcoord.xy,
    // texcoord.w ); }\n" "vec4 tex3Dbias( sampler3D sampler, vec4 texcoord ) {
    // return texture( sampler, texcoord.xyz, texcoord.w ); }\n" "vec4
    // texCUBEbias( samplerCube sampler, vec4 texcoord ) { return texture(
    // sampler, texcoord.xyz, texcoord.w ); }\n"
    //"\n"
    //"vec4 tex1Dlod( sampler1D sampler, vec4 texcoord ) { return textureLod(
    // sampler, texcoord.x, texcoord.w ); }\n" "vec4 tex2Dlod( sampler2D
    // sampler, vec4 texcoord ) { return textureLod( sampler, texcoord.xy,
    // texcoord.w );
    //}\n" "vec4 tex3Dlod( sampler3D sampler, vec4 texcoord ) { return
    // textureLod( sampler, texcoord.xyz, texcoord.w ); }\n" "vec4 texCUBElod(
    // samplerCube sampler, vec4 texcoord ) { return textureLod( sampler,
    // texcoord.xyz, texcoord.w ); }\n"
    //"\n"
};

// RB begin
const char* vertexInsert_GLSL_ES_3_00 = {
    "#version 300 es\n"
    "#define PC\n"
    "precision mediump float;\n"

    //"#extension GL_ARB_gpu_shader5 : enable\n"
    "\n"
    "float saturate( float v ) { return clamp( v, 0.0, 1.0 ); }\n"
    "vec2 saturate( vec2 v ) { return clamp( v, 0.0, 1.0 ); }\n"
    "vec3 saturate( vec3 v ) { return clamp( v, 0.0, 1.0 ); }\n"
    "vec4 saturate( vec4 v ) { return clamp( v, 0.0, 1.0 ); }\n"
    //"vec4 tex2Dlod( sampler2D sampler, vec4 texcoord ) { return textureLod(
    // sampler, texcoord.xy, texcoord.w ); }\n"
    "\n"};

const char* fragmentInsert_GLSL_ES_3_00 = {
    "#version 300 es\n"
    "#define PC\n"
    "precision mediump float;\n"
    "precision lowp sampler2D;\n"
    "precision lowp sampler2DShadow;\n"
    "precision lowp sampler2DArray;\n"
    "precision lowp sampler2DArrayShadow;\n"
    "precision lowp samplerCube;\n"
    "precision lowp samplerCubeShadow;\n"
    "precision lowp sampler3D;\n"
    "\n"
    "void clip( float v ) { if ( v < 0.0 ) { discard; } }\n"
    "void clip( vec2 v ) { if ( any( lessThan( v, vec2( 0.0 ) ) ) ) { discard; "
    "} }\n"
    "void clip( vec3 v ) { if ( any( lessThan( v, vec3( 0.0 ) ) ) ) { discard; "
    "} }\n"
    "void clip( vec4 v ) { if ( any( lessThan( v, vec4( 0.0 ) ) ) ) { discard; "
    "} }\n"
    "\n"
    "float saturate( float v ) { return clamp( v, 0.0, 1.0 ); }\n"
    "vec2 saturate( vec2 v ) { return clamp( v, 0.0, 1.0 ); }\n"
    "vec3 saturate( vec3 v ) { return clamp( v, 0.0, 1.0 ); }\n"
    "vec4 saturate( vec4 v ) { return clamp( v, 0.0, 1.0 ); }\n"
    "\n"
    "vec4 tex2D( sampler2D sampler, vec2 texcoord ) { return texture( sampler, "
    "texcoord.xy ); }\n"
    "vec4 tex2D( sampler2DShadow sampler, vec3 texcoord ) { return vec4( "
    "texture( sampler, texcoord.xyz ) ); }\n"
    "\n"
    "vec4 tex2D( sampler2D sampler, vec2 texcoord, vec2 dx, vec2 dy ) { return "
    "textureGrad( sampler, texcoord.xy, dx, dy ); }\n"
    "vec4 tex2D( sampler2DShadow sampler, vec3 texcoord, vec2 dx, vec2 dy ) { "
    "return vec4( textureGrad( sampler, texcoord.xyz, dx, dy ) ); }\n"
    "\n"
    "vec4 texCUBE( samplerCube sampler, vec3 texcoord ) { return texture( "
    "sampler, texcoord.xyz ); }\n"
    "vec4 texCUBE( samplerCubeShadow sampler, vec4 texcoord ) { return vec4( "
    "texture( sampler, texcoord.xyzw ) ); }\n"
    "\n"
    //"vec4 tex1Dproj( sampler1D sampler, vec2 texcoord ) { return textureProj(
    // sampler, texcoord ); }\n"
    "vec4 tex2Dproj( sampler2D sampler, vec3 texcoord ) { return textureProj( "
    "sampler, texcoord ); }\n"
    "vec4 tex3Dproj( sampler3D sampler, vec4 texcoord ) { return textureProj( "
    "sampler, texcoord ); }\n"
    "\n"
    //"vec4 tex1Dbias( sampler1D sampler, vec4 texcoord ) { return texture(
    // sampler, texcoord.x, texcoord.w ); }\n"
    "vec4 tex2Dbias( sampler2D sampler, vec4 texcoord ) { return texture( "
    "sampler, texcoord.xy, texcoord.w ); }\n"
    "vec4 tex3Dbias( sampler3D sampler, vec4 texcoord ) { return texture( "
    "sampler, texcoord.xyz, texcoord.w ); }\n"
    "vec4 texCUBEbias( samplerCube sampler, vec4 texcoord ) { return texture( "
    "sampler, texcoord.xyz, texcoord.w ); }\n"
    "\n"
    //"vec4 tex1Dlod( sampler1D sampler, vec4 texcoord ) { return textureLod(
    // sampler, texcoord.x, texcoord.w ); }\n"
    "vec4 tex2Dlod( sampler2D sampler, vec4 texcoord ) { return textureLod( "
    "sampler, texcoord.xy, texcoord.w ); }\n"
    "vec4 tex3Dlod( sampler3D sampler, vec4 texcoord ) { return textureLod( "
    "sampler, texcoord.xyz, texcoord.w ); }\n"
    "vec4 texCUBElod( samplerCube sampler, vec4 texcoord ) { return "
    "textureLod( sampler, texcoord.xyz, texcoord.w ); }\n"
    "\n"};
// RB end

struct builtinConversion_t {
  const char* nameCG;
  const char* nameGLSL;
} builtinConversion[] = {
    {"frac", "fract"}, {"lerp", "mix"}, {"rsqrt", "inversesqrt"},
    {"ddx", "dFdx"},   {"ddy", "dFdy"},

    {NULL, NULL}};

struct inOutVariable_t {
  idStr type;
  idStr nameCg;
  idStr nameGLSL;
  bool declareInOut;
};

#define SHADER_DEBUG_WRITE_TO_SAVEPATH 1

idRenderProgManager::preprocess_glsl_t idRenderProgManager::PreprocessGLSL(
    const idStr& in, const char* name, rpStage_t stage,
    const idStrList& compileMacros, bool builtin) {
  idParser src;
  src.LoadMemory(in.c_str(), in.Length(), name);

  idStrStatic<256> sourceName = "filename ";
  sourceName += name;
  src.AddDefine(sourceName);
  src.AddDefine("PC");

  for (int i = 0; i < compileMacros.Num(); i++) {
    src.AddDefine(compileMacros[i]);
  }

  src.AddDefine("USE_UNIFORM_ARRAYS");

  if (r_useHalfLambertLighting.GetBool()) {
    src.AddDefine("USE_HALF_LAMBERT");
  }

  if (r_useHDR.GetBool()) {
    src.AddDefine("USE_LINEAR_RGB");
  }

  if (r_pbrDebug.GetBool()) {
    src.AddDefine("DEBUG_PBR");
  }

  // SMAA configuration
  src.AddDefine("SMAA_GLSL_3");
  src.AddDefine("SMAA_PRESET_HIGH");

  // RB: tell shader debuggers what shader we look at
  idStr out;
  idStr filenameHint = "// filename " + idStr(name) + "\n";

  // Add obligatory GLSL preamble.
  if (stage == SHADER_STAGE_VERTEX) {
    out += vertexInsert;
    out += filenameHint;
  } else {
    out.ReAllocate(idStr::Length(fragmentInsert) + in.Length() * 2, false);
    out += fragmentInsert;
    out += filenameHint;
  }

  idList<idStr> uniforms;
  // Can be either "ubo" or "sampler."
  // Skip GPU skinning binding if used.
  idList<idStr> bindings;

  idToken token;
  char newline[128] = {"\n"};
  std::function<void()> tokenOut = [&out, &token, &newline]() {
    if (token.linesCrossed > 0) {
      out += newline;
    }
    if (token.WhiteSpaceBeforeToken()) {
      out += " ";
    }
    out += token;
    out += " ";
  };

  // The job of this block is largely to figure out the layout of the uniforms &
  // bindings.
  //
  // The uniforms will be contained in a block like this (we just need to
  // collect the names):
  //
  // layout( binding = 0 ) uniform UBOV {
  //    vec4 rpProjectionMatrixY;
  // };
  //
  // Otherwise, we just need to count the samplers:
  // layout( binding = 1 ) uniform sampler2D samp0;
  while (!src.EndOfFile()) {
    while (src.ReadToken(&token)) {
      if (token == "layout" && src.PeekTokenString("(")) {
        tokenOut();

        // Read until the end of the binding.
        while (src.ReadToken(&token)) {
          tokenOut();
          if (token == ")") {
            break;
          }
        }

        src.ReadToken(&token);
        tokenOut();
        if (token == "uniform") {
          src.ReadToken(&token);
          tokenOut();

          bool getname = false;
          if (token == "UBOV") {
            bindings.Append("ubo");

            // Separate these by a semicolon (or the starting "{").
            while (src.ReadToken(&token)) {
              tokenOut();
              if (token == "}") {
                break;
              }
              if (token == ";" || token == "{") {
                getname = true;
                continue;
              }
              // If getname, get the next token and that's the name:
              // i.e. vec4 rpProjectionMatrixY; -> rpProjectionMatrixY
              if (getname) {
                src.ReadToken(&token);
                tokenOut();
                // Add this to our list of uniforms.
                uniforms.Append(token);

                getname = false;
              }
            }
          } else if (token == "UBO_MAT") {
            bindings.Append("ubo");
          } else if (token == "sampler2D" || token == "samplerCube" ||
                     token == "sampler3D" || token == "sampler2DArrayShadow" ||
                     token == "sampler2DArray") {
            // Easy—just count the samplers.
            bindings.Append("sampler");
          }
        }
      } else {
        tokenOut();
      }
    }
  }

  return preprocess_glsl_t{out, uniforms, bindings};
}

/*
================================================================================================
idRenderProgManager::FindGLSLProgram
================================================================================================
*/
int idRenderProgManager::FindGLSLProgram(const char* name, int vIndex,
                                         int fIndex) {
  for (int i = 0; i < renderProgs.Num(); ++i) {
    if ((renderProgs[i].vertexShaderIndex == vIndex) &&
        (renderProgs[i].fragmentShaderIndex == fIndex)) {
      return i;
    }
  }

  renderProg_t program;
  program.name = name;
  int index = renderProgs.Append(program);
  LoadGLSLProgram(index, vIndex, fIndex);
  return index;
}

/*
================================================================================================
idRenderProgManager::GetGLSLParmName
================================================================================================
*/
const char* idRenderProgManager::GetGLSLParmName(int rp) const {
  assert(rp < RENDERPARM_TOTAL);
  return GLSLParmNames[rp];
}

// RB begin
const char* idRenderProgManager::GetGLSLMacroName(shaderFeature_t sf) const {
  assert(sf < MAX_SHADER_MACRO_NAMES);

  return GLSLMacroNames[sf];
}
// RB end

/*
================================================================================================
idRenderProgManager::SetUniformValue
================================================================================================
*/
void idRenderProgManager::SetUniformValue(const renderParm_t rp,
                                          const float* value) {
  for (int i = 0; i < 4; i++) {
    uniforms[rp][i] = value[i];
  }
}
