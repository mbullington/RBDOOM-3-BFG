#if defined( USE_GPU_SKINNING )

layout( binding = 1 ) uniform UBO_MAT {
	 vec4 matrices[ 408 ];
};
#endif // defined( USE_GPU_SKINNING )

layout( binding = 0 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpLocalViewOrigin;
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
	vec4 rpModelMatrixX;
	vec4 rpModelMatrixY;
	vec4 rpModelMatrixZ;
	vec4 rpModelMatrixW;
	vec4 rpJitterTexOffset;
};

#include "renderprogs_src/global.glsl"
#if defined( USE_GPU_SKINNING )
#endif 

layout( location = 0 ) in vec3 in_Position;
layout( location = 1 ) in vec2 in_TexCoord;
layout( location = 2 ) in vec4 in_Normal;
layout( location = 3 ) in vec4 in_Tangent;
layout( location = 4 ) in vec4 in_Color;
layout( location = 5 ) in vec4 in_Color2;

layout( location = 0 ) out vec4 vofi_TexCoord0;
layout( location = 1 ) out vec4 vofi_TexCoord1;
layout( location = 2 ) out vec4 vofi_TexCoord2;
layout( location = 3 ) out vec4 vofi_TexCoord3;
layout( location = 4 ) out vec4 vofi_TexCoord4;
layout( location = 5 ) out vec4 vofi_TexCoord5;
layout( location = 6 ) out vec4 vofi_TexCoord6;
layout( location = 7 ) out vec4 vofi_TexCoord7;
layout( location = 8 ) out vec4 vofi_Color;

void main()
{
	vec4 position4 = vec4( in_Position, 1.0 );

	vec4 vNormal = in_Normal * 2.0 - 1.0;
	vec4 vTangent = in_Tangent * 2.0 - 1.0;
	vec3 vBitangent = cross( vNormal.xyz, vTangent.xyz ) * vTangent.w;
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
	vec3 tangent;
	tangent.x = dot3( matX, vTangent );
	tangent.y = dot3( matY, vTangent );
	tangent.z = dot3( matZ, vTangent );
	tangent = normalize( tangent );
	vec3 bitangent;
	bitangent.x = dot3( matX, vBitangent );
	bitangent.y = dot3( matY, vBitangent );
	bitangent.z = dot3( matZ, vBitangent );
	bitangent = normalize( bitangent );
	vec4 modelPosition;
	modelPosition.x = dot4( matX, position4 );
	modelPosition.y = dot4( matY, position4 );
	modelPosition.z = dot4( matZ, position4 );
	modelPosition.w = 1.0;
	#else 
	vec4 modelPosition = position4;
	vec3 normal = vNormal.xyz;
	vec3 tangent = vTangent.xyz;
	vec3 bitangent = vBitangent.xyz;
	#endif 
	gl_Position.x = dot4( modelPosition, rpMVPmatrixX );
	gl_Position.y = dot4( modelPosition, rpMVPmatrixY );
	gl_Position.z = dot4( modelPosition, rpMVPmatrixZ );
	gl_Position.w = dot4( modelPosition, rpMVPmatrixW );
	vec4 defaultTexCoord = vec4( 0.0, 0.5, 0.0, 1.0 );
	vec4 toLight = normalize( vec4( 0.0, 0.5, 1.0, 1.0 ) );
	vofi_TexCoord0 = defaultTexCoord;
	vofi_TexCoord0.x = dot4( in_TexCoord.xy, rpBumpMatrixS );
	vofi_TexCoord0.y = dot4( in_TexCoord.xy, rpBumpMatrixT );
	vofi_TexCoord1 = defaultTexCoord;
	vofi_TexCoord1.x = dot4( in_TexCoord.xy, rpDiffuseMatrixS );
	vofi_TexCoord1.y = dot4( in_TexCoord.xy, rpDiffuseMatrixT );
	vofi_TexCoord2 = defaultTexCoord;
	vofi_TexCoord2.x = dot4( in_TexCoord.xy, rpSpecularMatrixS );
	vofi_TexCoord2.y = dot4( in_TexCoord.xy, rpSpecularMatrixT );
	vec4 toEye = normalize( rpLocalViewOrigin - modelPosition );
	vofi_TexCoord3.x = dot3( toEye, rpModelMatrixX );
	vofi_TexCoord3.y = dot3( toEye, rpModelMatrixY );
	vofi_TexCoord3.z = dot3( toEye, rpModelMatrixZ );
	vofi_TexCoord4.x = dot3( tangent, rpModelMatrixX );
	vofi_TexCoord5.x = dot3( tangent, rpModelMatrixY );
	vofi_TexCoord6.x = dot3( tangent, rpModelMatrixZ );
	vofi_TexCoord4.y = dot3( bitangent, rpModelMatrixX );
	vofi_TexCoord5.y = dot3( bitangent, rpModelMatrixY );
	vofi_TexCoord6.y = dot3( bitangent, rpModelMatrixZ );
	vofi_TexCoord4.z = dot3( normal, rpModelMatrixX );
	vofi_TexCoord5.z = dot3( normal, rpModelMatrixY );
	vofi_TexCoord6.z = dot3( normal, rpModelMatrixZ );
	vec4 worldPosition;
	worldPosition.x = dot4( modelPosition, rpModelMatrixX );
	worldPosition.y = dot4( modelPosition, rpModelMatrixY );
	worldPosition.z = dot4( modelPosition, rpModelMatrixZ );
	worldPosition.w = dot4( modelPosition, rpModelMatrixW );
	vofi_TexCoord7 = worldPosition;
	#if defined( USE_GPU_SKINNING )
	vofi_Color = vec4( 1.0, 1.0, 1.0, 1.0 );
	#else 
	vofi_Color = ( swizzleColor( in_Color ) * rpVertexColorModulate ) + rpVertexColorAdd;
	#endif 
}