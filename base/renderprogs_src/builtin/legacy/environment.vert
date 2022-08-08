
layout( binding = 0 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpLocalViewOrigin;
	vec4 rpColor;
	vec4 rpMVPmatrixX;
	vec4 rpMVPmatrixY;
	vec4 rpMVPmatrixZ;
	vec4 rpMVPmatrixW;
	vec4 rpJitterTexOffset;
};

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec3 in_Position;
layout( location = 1 ) in vec4 in_Normal;
layout( location = 2 ) in vec4 in_Color;

layout( location = 0 ) out vec3 vofi_TexCoord0;
layout( location = 1 ) out vec3 vofi_TexCoord1;
layout( location = 2 ) out vec4 vofi_Color;

void main()
{
	vec4 position4 = vec4( in_Position, 1.0 );

	vec4 vNormal = in_Normal * 2.0 - 1.0;
	gl_Position.x = dot4( position4, rpMVPmatrixX );
	gl_Position.y = dot4( position4, rpMVPmatrixY );
	gl_Position.z = dot4( position4, rpMVPmatrixZ );
	gl_Position.w = dot4( position4, rpMVPmatrixW );
	vec4 toEye = rpLocalViewOrigin - position4;
	vofi_TexCoord0 = toEye.xyz;
	vofi_TexCoord1 = vNormal.xyz;
	vofi_Color = sRGBAToLinearRGBA( rpColor );
}