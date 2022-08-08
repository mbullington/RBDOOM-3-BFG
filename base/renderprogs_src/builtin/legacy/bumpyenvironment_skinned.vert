
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
	vec4 rpModelMatrixX;
	vec4 rpModelMatrixY;
	vec4 rpModelMatrixZ;
	vec4 rpJitterTexOffset;
};

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec3 in_Position;
layout( location = 1 ) in vec2 in_TexCoord;
layout( location = 2 ) in vec4 in_Normal;
layout( location = 3 ) in vec4 in_Tangent;
layout( location = 4 ) in vec4 in_Color;
layout( location = 5 ) in vec4 in_Color2;

layout( location = 0 ) out vec2 vofi_TexCoord0;
layout( location = 1 ) out vec3 vofi_TexCoord1;
layout( location = 2 ) out vec3 vofi_TexCoord2;
layout( location = 3 ) out vec3 vofi_TexCoord3;
layout( location = 4 ) out vec3 vofi_TexCoord4;
layout( location = 5 ) out vec4 vofi_Color;

void main()
{
	vec4 position4 = vec4( in_Position, 1.0 );

	vec4 vNormal = in_Normal * 2.0 - 1.0;
	vec4 vTangent = in_Tangent * 2.0 - 1.0;
	vec3 vBinormal = cross( vNormal.xyz, vTangent.xyz ) * vTangent.w;
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
	vec3 tangent;
	tangent.x = dot3( matX, vTangent );
	tangent.y = dot3( matY, vTangent );
	tangent.z = dot3( matZ, vTangent );
	tangent = normalize( tangent );
	vec3 binormal;
	binormal.x = dot3( matX, vBinormal );
	binormal.y = dot3( matY, vBinormal );
	binormal.z = dot3( matZ, vBinormal );
	binormal = normalize( binormal );
	vec4 modelPosition;
	modelPosition.x = dot4( matX, position4 );
	modelPosition.y = dot4( matY, position4 );
	modelPosition.z = dot4( matZ, position4 );
	modelPosition.w = 1.0;
	gl_Position.x = dot4( modelPosition, rpMVPmatrixX );
	gl_Position.y = dot4( modelPosition, rpMVPmatrixY );
	gl_Position.z = dot4( modelPosition, rpMVPmatrixZ );
	gl_Position.w = dot4( modelPosition, rpMVPmatrixW );
	vofi_TexCoord0 = in_TexCoord.xy;
	vec4 toEye = rpLocalViewOrigin - modelPosition;
	vofi_TexCoord1.x = dot3( toEye, rpModelMatrixX );
	vofi_TexCoord1.y = dot3( toEye, rpModelMatrixY );
	vofi_TexCoord1.z = dot3( toEye, rpModelMatrixZ );
	vofi_TexCoord2.x = dot3( tangent, rpModelMatrixX );
	vofi_TexCoord3.x = dot3( tangent, rpModelMatrixY );
	vofi_TexCoord4.x = dot3( tangent, rpModelMatrixZ );
	vofi_TexCoord2.y = dot3( binormal, rpModelMatrixX );
	vofi_TexCoord3.y = dot3( binormal, rpModelMatrixY );
	vofi_TexCoord4.y = dot3( binormal, rpModelMatrixZ );
	vofi_TexCoord2.z = dot3( normal, rpModelMatrixX );
	vofi_TexCoord3.z = dot3( normal, rpModelMatrixY );
	vofi_TexCoord4.z = dot3( normal, rpModelMatrixZ );
	vofi_Color = rpColor;
}