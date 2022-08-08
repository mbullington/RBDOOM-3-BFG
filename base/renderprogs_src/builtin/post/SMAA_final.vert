
layout( binding = 0 ) uniform UBOV {
	vec4 rpScreenCorrectionFactor;
	vec4 rpWindowCoord;
	vec4 rpJitterTexOffset;
};

#include "renderprogs_src/global.glsl"
#define SMAA_INCLUDE_VS 1
#define SMAA_INCLUDE_PS 0
#include "renderprogs_src/builtin/post/SMAA.glsl"

layout( location = 0 ) in vec3 in_Position;
layout( location = 1 ) in vec2 in_TexCoord;
layout( location = 2 ) in vec4 in_Normal;
layout( location = 3 ) in vec4 in_Tangent;
layout( location = 4 ) in vec4 in_Color;

layout( location = 0 ) out vec2 vofi_TexCoord0;
layout( location = 1 ) out vec4 vofi_TexCoord1;

void main()
{
	vec4 position4 = vec4( in_Position, 1.0 );

	gl_Position = position4;
	vofi_TexCoord0 = in_TexCoord;
	vec4 offset;
	SMAANeighborhoodBlendingVS( in_TexCoord, offset );
	vofi_TexCoord1 = offset;
}