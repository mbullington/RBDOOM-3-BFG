layout( binding = 2 ) uniform samplerCube samp0;

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec3 vofi_TexCoord0;
layout( location = 1 ) in vec3 vofi_TexCoord1;
layout( location = 2 ) in vec4 vofi_Color;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	vec3 globalNormal = normalize( vofi_TexCoord1 );
	vec3 globalEye = normalize( vofi_TexCoord0 );
	vec3 reflectionVector = _float3( dot3( globalEye, globalNormal ) );
	reflectionVector *= globalNormal;
	reflectionVector = ( reflectionVector * 2.0 ) - globalEye;
	vec4 envMap = texture( samp0, reflectionVector );
	fo_FragColor = vec4( sRGBToLinearRGB( envMap.xyz ), 1.0 ) * vofi_Color;
}