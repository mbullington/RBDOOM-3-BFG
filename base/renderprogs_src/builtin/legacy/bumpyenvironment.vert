
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

layout( location = 0 ) out vec2 vofi_TexCoord0;
layout( location = 1 ) out vec3 vofi_TexCoord1;
layout( location = 2 ) out vec3 vofi_TexCoord2;
layout( location = 3 ) out vec3 vofi_TexCoord3;
layout( location = 4 ) out vec3 vofi_TexCoord4;
layout( location = 5 ) out vec4 vofi_Color;

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
	vofi_TexCoord0 = in_TexCoord.xy;
	vec4 toEye = rpLocalViewOrigin - position4;
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