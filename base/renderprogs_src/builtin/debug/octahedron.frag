#pragma static USE_GPU_SKINNING _skinned

#if defined(USE_GPU_SKINNING)
layout( binding = 2 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpJitterTexOffset;
	vec4 rpCascadeDistances;
};
#else
layout( binding = 1 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpJitterTexOffset;
	vec4 rpCascadeDistances;
};
#endif
#if defined(USE_GPU_SKINNING)
layout( binding = 3 ) uniform sampler2D samp0;
#else
layout( binding = 2 ) uniform sampler2D samp0;
#endif // else

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
	vec2 normalizedOctCoord = octEncode( reflectionVector );
	vec2 normalizedOctCoordZeroOne = ( normalizedOctCoord + vec2( 1.0 ) ) * 0.5;
	#if 0
	vec2 octCoordNormalizedToTextureDimensions = ( normalizedOctCoordZeroOne * ( rpCascadeDistances.x - float( 2.0 ) ) ) / rpCascadeDistances.xy;
	vec2 probeTopLeftPosition = vec2( 1.0, 1.0 );
	vec2 normalizedProbeTopLeftPosition = probeTopLeftPosition * rpCascadeDistances.zw;
	normalizedOctCoordZeroOne.xy = normalizedProbeTopLeftPosition + octCoordNormalizedToTextureDimensions;
	#endif 
	vec4 envMap = texture( samp0, normalizedOctCoordZeroOne, 0 );
	fo_FragColor = vec4( envMap.xyz, 1.0 ) * vofi_Color * 1.0;
}