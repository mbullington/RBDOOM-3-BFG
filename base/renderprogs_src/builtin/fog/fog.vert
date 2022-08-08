
layout( binding = 0 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpMVPmatrixX;
	vec4 rpMVPmatrixY;
	vec4 rpMVPmatrixZ;
	vec4 rpMVPmatrixW;
	vec4 rpTexGen0S;
	vec4 rpTexGen0T;
	vec4 rpTexGen1S;
	vec4 rpTexGen1T;
	vec4 rpJitterTexOffset;
};

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec3 in_Position;
layout( location = 1 ) in vec2 in_TexCoord;
layout( location = 2 ) in vec4 in_Normal;
layout( location = 3 ) in vec4 in_Tangent;
layout( location = 4 ) in vec4 in_Color;

layout( location = 0 ) out vec2 vofi_TexCoord0;
layout( location = 1 ) out vec2 vofi_TexCoord1;

void main()
{
	vec4 position4 = vec4( in_Position, 1.0 );

	gl_Position.x = dot4( position4, rpMVPmatrixX );
	gl_Position.y = dot4( position4, rpMVPmatrixY );
	gl_Position.z = dot4( position4, rpMVPmatrixZ );
	gl_Position.w = dot4( position4, rpMVPmatrixW );
	vofi_TexCoord0.x = dot4( position4, rpTexGen0S );
	vofi_TexCoord0.y = dot4( position4, rpTexGen0T );
	vofi_TexCoord1.x = dot4( position4, rpTexGen1S );
	vofi_TexCoord1.y = dot4( position4, rpTexGen1T );
}