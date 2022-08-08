/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

layout( binding = 0 ) uniform UBOV {
    vec4 rpMVPmatrixX;
	vec4 rpMVPmatrixY;
	vec4 rpMVPmatrixZ;
	vec4 rpMVPmatrixW;
	vec4 rpUser0;
    vec4 rpUser1;
};

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec3 in_Position;
layout( location = 1 ) in vec2 in_TexCoord;
layout( location = 2 ) in vec4 in_Normal;
layout( location = 3 ) in vec4 in_Tangent;
layout( location = 4 ) in vec4 in_Color;
layout( location = 5 ) in vec4 in_Color2;

layout( location = 0 ) out vec4 vofi_Color;
layout( location = 1 ) out vec3 vofi_TexCoord0;

void main()
{
	gl_Position.x = dot4( vec4(in_Position, 1.0), rpMVPmatrixX );
	gl_Position.y = dot4( vec4(in_Position, 1.0), rpMVPmatrixY );
	gl_Position.z = dot4( vec4(in_Position, 1.0), rpMVPmatrixZ );
	gl_Position.w = dot4( vec4(in_Position, 1.0), rpMVPmatrixW );

	vofi_Color = rpUser1; // targetHue

	vofi_TexCoord0.x = in_TexCoord.x;
	vofi_TexCoord0.y = 1.0 - in_TexCoord.y;

	vofi_TexCoord0.z = rpUser0.x; // fraction
}
