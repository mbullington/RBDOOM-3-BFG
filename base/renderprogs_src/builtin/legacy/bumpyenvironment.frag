layout( binding = 1 ) uniform samplerCube samp0;
layout( binding = 2 ) uniform sampler2D samp1;

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec2 vofi_TexCoord0;
layout( location = 1 ) in vec3 vofi_TexCoord1;
layout( location = 2 ) in vec3 vofi_TexCoord2;
layout( location = 3 ) in vec3 vofi_TexCoord3;
layout( location = 4 ) in vec3 vofi_TexCoord4;
layout( location = 5 ) in vec4 vofi_Color;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	vec4 bump = texture( samp1, vofi_TexCoord0 ) * 2.0 - 1.0;
	vec3 localNormal;
	#if defined(USE_NORMAL_FMT_RGB8)
	localNormal = vec3( bump.rg, 0.0 );
	#else 
	localNormal = vec3( bump.wy, 0.0 );
	#endif 
	localNormal.z = sqrt( 1.0 - dot3( localNormal, localNormal ) );
	vec3 globalNormal;
	globalNormal.x = dot3( localNormal, vofi_TexCoord2 );
	globalNormal.y = dot3( localNormal, vofi_TexCoord3 );
	globalNormal.z = dot3( localNormal, vofi_TexCoord4 );
	vec3 globalEye = normalize( vofi_TexCoord1 );
	vec3 reflectionVector = globalNormal * dot3( globalEye, globalNormal );
	reflectionVector = ( reflectionVector * 2.0 ) - globalEye;
	vec4 envMap = texture( samp0, reflectionVector );
	fo_FragColor = vec4( sRGBToLinearRGB( envMap.xyz ), 1.0 ) * vofi_Color;
}