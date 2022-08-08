#pragma static BRIGHTPASS _write

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec3 in_Position;
layout( location = 1 ) in vec2 in_TexCoord;

layout( location = 0 ) out vec2 vofi_TexCoord0;

void main()
{
	vec4 position4 = vec4( in_Position, 1.0 );

	gl_Position = position4;
	vofi_TexCoord0 = in_TexCoord;
}