#pragma static USE_GPU_SKINNING _skinned

layout( binding = 1 ) uniform UBO_MAT {
	 vec4 matrices[ 408 ];
};

layout( binding = 0 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpLocalLightOrigin;
	vec4 rpLocalViewOrigin;
	vec4 rpLightProjectionS;
	vec4 rpLightProjectionT;
	vec4 rpLightProjectionQ;
	vec4 rpLightFalloffS;
	vec4 rpBumpMatrixS;
	vec4 rpBumpMatrixT;
	vec4 rpDiffuseMatrixS;
	vec4 rpDiffuseMatrixT;
	vec4 rpSpecularMatrixS;
	vec4 rpSpecularMatrixT;
	vec4 rpVertexColorModulate;
	vec4 rpVertexColorAdd;
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

layout( location = 0 ) out vec4 vofi_TexCoord1;
layout( location = 1 ) out vec4 vofi_TexCoord2;
layout( location = 2 ) out vec4 vofi_TexCoord3;
layout( location = 3 ) out vec4 vofi_TexCoord4;
layout( location = 4 ) out vec4 vofi_TexCoord5;
layout( location = 5 ) out vec4 vofi_TexCoord6;
layout( location = 6 ) out vec4 vofi_Color;

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
	vec4 defaultTexCoord = vec4( 0.0, 0.5, 0.0, 1.0 );
	vec4 toLight = rpLocalLightOrigin - modelPosition;
	vofi_TexCoord1 = defaultTexCoord;
	vofi_TexCoord1.x = dot4( in_TexCoord.xy, rpBumpMatrixS );
	vofi_TexCoord1.y = dot4( in_TexCoord.xy, rpBumpMatrixT );
	vofi_TexCoord2 = defaultTexCoord;
	vofi_TexCoord2.x = dot4( modelPosition, rpLightFalloffS );
	vofi_TexCoord3.x = dot4( modelPosition, rpLightProjectionS );
	vofi_TexCoord3.y = dot4( modelPosition, rpLightProjectionT );
	vofi_TexCoord3.z = 0.0;
	vofi_TexCoord3.w = dot4( modelPosition, rpLightProjectionQ );
	vofi_TexCoord4 = defaultTexCoord;
	vofi_TexCoord4.x = dot4( in_TexCoord.xy, rpDiffuseMatrixS );
	vofi_TexCoord4.y = dot4( in_TexCoord.xy, rpDiffuseMatrixT );
	vofi_TexCoord5 = defaultTexCoord;
	vofi_TexCoord5.x = dot4( in_TexCoord.xy, rpSpecularMatrixS );
	vofi_TexCoord5.y = dot4( in_TexCoord.xy, rpSpecularMatrixT );
	toLight = normalize( toLight );
	vec4 toView = normalize( rpLocalViewOrigin - modelPosition );
	vec4 halfAngleVector = toLight + toView;
	vofi_TexCoord6.x = dot3( tangent, halfAngleVector );
	vofi_TexCoord6.y = dot3( binormal, halfAngleVector );
	vofi_TexCoord6.z = dot3( normal, halfAngleVector );
	vofi_TexCoord6.w = 1.0;
	vofi_Color = ( swizzleColor( in_Color ) * rpVertexColorModulate ) + rpVertexColorAdd;
}