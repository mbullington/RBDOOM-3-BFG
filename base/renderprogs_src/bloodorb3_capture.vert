
layout( binding = 0 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpMVPmatrixX;
	vec4 rpMVPmatrixY;
	vec4 rpMVPmatrixZ;
	vec4 rpMVPmatrixW;
	vec4 rpJitterTexOffset;
	vec4 rpUser0;
	vec4 rpUser1;
	vec4 rpUser2;
};

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec3 in_Position;
layout( location = 1 ) in vec2 in_TexCoord;
layout( location = 2 ) in vec4 in_Normal;
layout( location = 3 ) in vec4 in_Tangent;
layout( location = 4 ) in vec4 in_Color;

layout( location = 0 ) out vec2 vofi_TexCoord0;
layout( location = 1 ) out vec2 vofi_TexCoord1;
layout( location = 2 ) out vec2 vofi_TexCoord2;
layout( location = 3 ) out vec2 vofi_TexCoord3;
layout( location = 4 ) out vec2 vofi_TexCoord4;

void main()
{
	vec4 position4 = vec4( in_Position, 1.0 );

	gl_Position.x = dot4( position4, rpMVPmatrixX );
	gl_Position.y = dot4( position4, rpMVPmatrixY );
	gl_Position.z = dot4( position4, rpMVPmatrixZ );
	gl_Position.w = dot4( position4, rpMVPmatrixW );
	vec4 centerScaleTex = rpUser0;
	vec4 rotateTex = rpUser1;
	vec2 tc0 = CenterScale( in_TexCoord, centerScaleTex.xy );
	vofi_TexCoord0 = Rotate2D( tc0, rotateTex.xy );
	vofi_TexCoord1 = Rotate2D( tc0, vec2( rotateTex.z, -rotateTex.w ) );
	vofi_TexCoord2 = Rotate2D( tc0, rotateTex.zw );
	vofi_TexCoord3 = in_TexCoord;
	vec4 colorFactor = rpUser2;
	vofi_TexCoord4 = colorFactor.xx;
}