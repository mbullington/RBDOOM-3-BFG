#pragma static USE_GPU_SKINNING _skinned

#if defined(USE_GPU_SKINNING)
layout( binding = 2 ) uniform UBOV {
	vec4 rpScreenCorrectionFactor;
	vec4 rpWindowCoord;
	vec4 rpGlobalLightOrigin;
	vec4 rpJitterTexScale;
	vec4 rpJitterTexOffset;
	vec4 rpCascadeDistances;
};
#else
layout( binding = 1 ) uniform UBOV {
	vec4 rpScreenCorrectionFactor;
	vec4 rpWindowCoord;
	vec4 rpGlobalLightOrigin;
	vec4 rpJitterTexScale;
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

ivec3 GetBaseGridCoord( vec3 origin )
{
	vec3 lightGridOrigin = rpGlobalLightOrigin.xyz;
	vec3 lightGridSize = rpJitterTexScale.xyz;
	ivec3 lightGridBounds = ivec3( rpJitterTexOffset.x, rpJitterTexOffset.y, rpJitterTexOffset.z );
	ivec3 pos;
	vec3 lightOrigin = origin - lightGridOrigin;
	for( int i = 0; i < 3; i++ )
	{
		float v;
		v = lightOrigin[i] * ( 1.0 / lightGridSize[i] );
		pos[i] = int( floor( v ) );
		if( pos[i] < 0 )
		{
			pos[i] = 0;
		}
		else if( pos[i] >= lightGridBounds[i] - 1 )
		{
			pos[i] = lightGridBounds[i] - 1;
		}
	}
	return pos;
}
void main()
{
	int LIGHTGRID_IRRADIANCE_SIZE = 32;
	vec3 globalPosition = vofi_TexCoord0.xyz;
	vec3 globalNormal = normalize( vofi_TexCoord1 );
	vec2 normalizedOctCoord = octEncode( globalNormal );
	vec2 normalizedOctCoordZeroOne = ( normalizedOctCoord + vec2( 1.0 ) ) * 0.5;
	vec3 lightGridOrigin = rpGlobalLightOrigin.xyz;
	vec3 lightGridSize = rpJitterTexScale.xyz;
	ivec3 lightGridBounds = ivec3( rpJitterTexOffset.x, rpJitterTexOffset.y, rpJitterTexOffset.z );
	float invXZ = ( 1.0 / ( lightGridBounds[0] * lightGridBounds[2] ) );
	float invY = ( 1.0 / lightGridBounds[1] );
	normalizedOctCoordZeroOne.x *= invXZ;
	normalizedOctCoordZeroOne.y *= invY;
	ivec3 gridStep;
	gridStep[0] = 1;
	gridStep[1] = lightGridBounds[0];
	gridStep[2] = lightGridBounds[0] * lightGridBounds[1];
	ivec3 gridCoord = GetBaseGridCoord( globalPosition );
	normalizedOctCoordZeroOne.x += ( gridCoord[0] * gridStep[0] + gridCoord[2] * gridStep[1] ) * invXZ;
	normalizedOctCoordZeroOne.y += ( gridCoord[1] * invY );
	#if 1
	vec2 octCoordNormalizedToTextureDimensions = normalizedOctCoordZeroOne * rpScreenCorrectionFactor.w;
	vec2 probeTopLeftPosition;
	probeTopLeftPosition.x = ( gridCoord[0] * gridStep[0] + gridCoord[2] * gridStep[1] ) * rpScreenCorrectionFactor.z + rpScreenCorrectionFactor.z * 0.5;
	probeTopLeftPosition.y = ( gridCoord[1] ) * rpScreenCorrectionFactor.z + rpScreenCorrectionFactor.z * 0.5;
	vec2 normalizedProbeTopLeftPosition = probeTopLeftPosition * rpCascadeDistances.zw;
	normalizedOctCoordZeroOne.xy = normalizedProbeTopLeftPosition + octCoordNormalizedToTextureDimensions;
	#endif 
	vec4 envMap = texture( samp0, normalizedOctCoordZeroOne, 0 );
	fo_FragColor = vec4( envMap.xyz, 1.0 ) * 1.0 * vofi_Color;
}