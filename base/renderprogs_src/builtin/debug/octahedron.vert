#pragma static USE_GPU_SKINNING _skinned

layout( binding = 1 ) uniform UBO_MAT {
	 vec4 matrices[ 408 ];
};

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
layout( location = 1 ) in vec2 in_TexCoord;
layout( location = 2 ) in vec4 in_Normal;
layout( location = 3 ) in vec4 in_Tangent;
layout( location = 4 ) in vec4 in_Color;
layout( location = 5 ) in vec4 in_Color2;

layout( location = 0 ) out vec3 vofi_TexCoord0;
layout( location = 1 ) out vec3 vofi_TexCoord1;
layout( location = 2 ) out vec4 vofi_Color;

void main()
{
	vec4 position4 = vec4( in_Position, 1.0 );

	vec4 vNormal = in_Normal * 2.0 - 1.0;
	#if defined( USE_GPU_SKINNING )
	float w0 = in_Color2.x;
	float w1 = in_Color2.y;
	float w2 = in_Color2.z;
	float w3 = in_Color2.w;
	vec4 matX, matY, matZ;
	int joint = int( in_Color.x * 255.1 * 3.0 );
	matX = matrices[int( joint + 0 )] * w0;
	matY = matrices[int( joint + 1 )] * w0;
	matZ = matrices[int( joint + 2 )] * w0;
	joint = int( in_Color.y * 255.1 * 3.0 );
	matX += matrices[int( joint + 0 )] * w1;
	matY += matrices[int( joint + 1 )] * w1;
	matZ += matrices[int( joint + 2 )] * w1;
	joint = int( in_Color.z * 255.1 * 3.0 );
	matX += matrices[int( joint + 0 )] * w2;
	matY += matrices[int( joint + 1 )] * w2;
	matZ += matrices[int( joint + 2 )] * w2;
	joint = int( in_Color.w * 255.1 * 3.0 );
	matX += matrices[int( joint + 0 )] * w3;
	matY += matrices[int( joint + 1 )] * w3;
	matZ += matrices[int( joint + 2 )] * w3;
	vec3 normal;
	normal.x = dot3( matX, vNormal );
	normal.y = dot3( matY, vNormal );
	normal.z = dot3( matZ, vNormal );
	normal = normalize( normal );
	vec4 modelPosition;
	modelPosition.x = dot4( matX, position4 );
	modelPosition.y = dot4( matY, position4 );
	modelPosition.z = dot4( matZ, position4 );
	modelPosition.w = 1.0;
	#else 
	vec4 modelPosition = position4;
	vec4 normal = vNormal;
	#endif 
	gl_Position.x = dot4( modelPosition, rpMVPmatrixX );
	gl_Position.y = dot4( modelPosition, rpMVPmatrixY );
	gl_Position.z = dot4( modelPosition, rpMVPmatrixZ );
	gl_Position.w = dot4( modelPosition, rpMVPmatrixW );
	vec4 toEye = rpLocalViewOrigin - position4;
	vofi_TexCoord0 = toEye.xyz;
	vofi_TexCoord1 = normal.xyz;
	vofi_Color = sRGBAToLinearRGBA( rpColor );
}