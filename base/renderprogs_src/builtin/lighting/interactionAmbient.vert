
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

	vec4 normal = in_Normal * 2.0 - 1.0;
	vec4 tangent = in_Tangent * 2.0 - 1.0;
	vec3 binormal = cross( normal.xyz, tangent.xyz ) * tangent.w;
	gl_Position.x = dot4( position4, rpMVPmatrixX );
	gl_Position.y = dot4( position4, rpMVPmatrixY );
	gl_Position.z = dot4( position4, rpMVPmatrixZ );
	gl_Position.w = dot4( position4, rpMVPmatrixW );
	vec4 defaultTexCoord = vec4( 0.0, 0.5, 0.0, 1.0 );
	vec4 toLight = rpLocalLightOrigin - position4;
	vofi_TexCoord1 = defaultTexCoord;
	vofi_TexCoord1.x = dot4( in_TexCoord.xy, rpBumpMatrixS );
	vofi_TexCoord1.y = dot4( in_TexCoord.xy, rpBumpMatrixT );
	vofi_TexCoord2 = defaultTexCoord;
	vofi_TexCoord2.x = dot4( position4, rpLightFalloffS );
	vofi_TexCoord3.x = dot4( position4, rpLightProjectionS );
	vofi_TexCoord3.y = dot4( position4, rpLightProjectionT );
	vofi_TexCoord3.z = 0.0;
	vofi_TexCoord3.w = dot4( position4, rpLightProjectionQ );
	vofi_TexCoord4 = defaultTexCoord;
	vofi_TexCoord4.x = dot4( in_TexCoord.xy, rpDiffuseMatrixS );
	vofi_TexCoord4.y = dot4( in_TexCoord.xy, rpDiffuseMatrixT );
	vofi_TexCoord5 = defaultTexCoord;
	vofi_TexCoord5.x = dot4( in_TexCoord.xy, rpSpecularMatrixS );
	vofi_TexCoord5.y = dot4( in_TexCoord.xy, rpSpecularMatrixT );
	toLight = normalize( toLight );
	vec4 toView = normalize( rpLocalViewOrigin - position4 );
	vec4 halfAngleVector = toLight + toView;
	vofi_TexCoord6.x = dot3( tangent, halfAngleVector );
	vofi_TexCoord6.y = dot3( binormal, halfAngleVector );
	vofi_TexCoord6.z = dot3( normal, halfAngleVector );
	vofi_TexCoord6.w = 1.0;
	vofi_Color = ( swizzleColor( in_Color ) * rpVertexColorModulate ) + rpVertexColorAdd;
}