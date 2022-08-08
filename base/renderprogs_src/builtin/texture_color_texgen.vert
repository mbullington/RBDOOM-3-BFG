
layout( binding = 0 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpVertexColorModulate;
	vec4 rpVertexColorAdd;
	vec4 rpColor;
	vec4 rpMVPmatrixX;
	vec4 rpMVPmatrixY;
	vec4 rpMVPmatrixZ;
	vec4 rpMVPmatrixW;
	vec4 rpTextureMatrixS;
	vec4 rpTextureMatrixT;
	vec4 rpTexGen0S;
	vec4 rpTexGen0T;
	vec4 rpTexGen0Q;
	vec4 rpJitterTexOffset;
};

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec3 in_Position;
layout( location = 1 ) in vec2 in_TexCoord;
layout( location = 2 ) in vec4 in_Normal;
layout( location = 3 ) in vec4 in_Tangent;
layout( location = 4 ) in vec4 in_Color;

layout( location = 0 ) out vec4 vofi_TexCoord0;
layout( location = 1 ) out vec4 vofi_Color;

void main()
{
	vec4 position4 = vec4( in_Position, 1.0 );

	gl_Position.x = dot4( position4, rpMVPmatrixX );
	gl_Position.y = dot4( position4, rpMVPmatrixY );
	gl_Position.z = dot4( position4, rpMVPmatrixZ );
	gl_Position.w = dot4( position4, rpMVPmatrixW );
	vec4 tc0;
	tc0.x = dot4( position4, rpTexGen0S );
	tc0.y = dot4( position4, rpTexGen0T );
	tc0.z = 0.0;
	tc0.w = dot4( position4, rpTexGen0Q );
	vofi_TexCoord0.x = dot4( tc0, rpTextureMatrixS );
	vofi_TexCoord0.y = dot4( tc0, rpTextureMatrixT );
	vofi_TexCoord0.zw = tc0.zw;
	vec4 vertexColor = ( swizzleColor( in_Color ) * rpVertexColorModulate ) + rpVertexColorAdd;
	vofi_Color = vertexColor * rpColor;
}