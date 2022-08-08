
layout( binding = 1 ) uniform UBO_MAT {
	 vec4 matrices[ 408 ];
};

layout( binding = 0 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpMVPmatrixX;
	vec4 rpMVPmatrixY;
	vec4 rpMVPmatrixZ;
	vec4 rpMVPmatrixW;
	vec4 rpProjectionMatrixY;
	vec4 rpProjectionMatrixW;
	vec4 rpModelViewMatrixZ;
	vec4 rpEnableSkinning;
	vec4 rpJitterTexOffset;
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

layout( location = 0 ) out vec4 vofi_TexCoord0;
layout( location = 1 ) out vec4 vofi_TexCoord1;

void main()
{
	vec4 position4 = vec4( in_Position, 1.0 );

	#include "renderprogs_src/skinning.glsl"
	vec4 textureScroll = rpUser0;
	vofi_TexCoord0 = vec4( in_TexCoord.xy, 0, 0 ) + textureScroll;
	vec4 vec = vec4( 0, 1, 0, 1 );
	vec.z = dot4( modelPosition, rpModelViewMatrixZ );
	float magicProjectionAdjust = 0.43;
	float x = dot4( vec, rpProjectionMatrixY ) * magicProjectionAdjust;
	float w = dot4( vec, rpProjectionMatrixW );
	w = max( w, 1.0 );
	x /= w;
	x = min( x, 0.02 );
	vec4 deformMagnitude = rpUser1;
	vofi_TexCoord1 = x * deformMagnitude;
}