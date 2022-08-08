
#if defined(USE_GPU_SKINNING)
layout( binding = 2 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpDiffuseModifier;
	vec4 rpSpecularModifier;
	vec4 rpLocalLightOrigin;
	vec4 rpGlobalEyePos;
	vec4 rpWobbleSkyX;
	vec4 rpWobbleSkyY;
	vec4 rpWobbleSkyZ;
	vec4 rpAmbientColor;
	vec4 rpJitterTexOffset;
	vec4 rpCascadeDistances;
};
#else
layout( binding = 1 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpDiffuseModifier;
	vec4 rpSpecularModifier;
	vec4 rpLocalLightOrigin;
	vec4 rpGlobalEyePos;
	vec4 rpWobbleSkyX;
	vec4 rpWobbleSkyY;
	vec4 rpWobbleSkyZ;
	vec4 rpAmbientColor;
	vec4 rpJitterTexOffset;
	vec4 rpCascadeDistances;
};
#endif
#if defined(USE_GPU_SKINNING)
layout( binding = 3 ) uniform sampler2D samp0;
layout( binding = 4 ) uniform sampler2D samp1;
layout( binding = 5 ) uniform sampler2D samp2;
layout( binding = 6 ) uniform sampler2D samp3;
layout( binding = 7 ) uniform sampler2D samp4;
layout( binding = 8 ) uniform sampler2D samp7;
layout( binding = 9 ) uniform sampler2D samp8;
layout( binding = 10 ) uniform sampler2D samp9;
layout( binding = 11 ) uniform sampler2D samp10;
#else
layout( binding = 2 ) uniform sampler2D samp0;
layout( binding = 3 ) uniform sampler2D samp1;
layout( binding = 4 ) uniform sampler2D samp2;
layout( binding = 5 ) uniform sampler2D samp3;
layout( binding = 6 ) uniform sampler2D samp4;
layout( binding = 7 ) uniform sampler2D samp7;
layout( binding = 8 ) uniform sampler2D samp8;
layout( binding = 9 ) uniform sampler2D samp9;
layout( binding = 10 ) uniform sampler2D samp10;
#endif // else

#include "renderprogs_src/global.glsl"
#include "renderprogs_src/BRDF.glsl"

layout( location = 0 ) in vec4 vofi_TexCoord0;
layout( location = 1 ) in vec4 vofi_TexCoord1;
layout( location = 2 ) in vec4 vofi_TexCoord2;
layout( location = 3 ) in vec4 vofi_TexCoord3;
layout( location = 4 ) in vec4 vofi_TexCoord4;
layout( location = 5 ) in vec4 vofi_TexCoord5;
layout( location = 6 ) in vec4 vofi_TexCoord6;
layout( location = 7 ) in vec4 vofi_TexCoord7;
layout( location = 8 ) in vec4 vofi_Color;

layout( location = 0 ) out vec4 fo_FragColor;

bool AABBRayIntersection( vec3 b[2], vec3 start, vec3 dir, out float scale )
{
	int i, ax0, ax1, ax2, side, inside;
	float f;
	vec3 hit;
	ax0 = -1;
	inside = 0;
	for( i = 0; i < 3; i++ )
	{
		if( start[i] < b[0][i] )
		{
			side = 0;
		}
		else if( start[i] > b[1][i] )
		{
			side = 1;
		}
		else
		{
			inside++;
			continue;
		}
		if( dir[i] == 0.0 )
		{
			continue;
		}
		f = ( start[i] - b[side][i] );
		if( ax0 < 0 || abs( f ) > abs( scale * dir[i] ) )
		{
			scale = - ( f / dir[i] );
			ax0 = i;
		}
	}
	if( ax0 < 0 )
	{
		scale = 0.0;
		return ( inside == 3 );
	}
	ax1 = ( ax0 + 1 ) % 3;
	ax2 = ( ax0 + 2 ) % 3;
	hit[ax1] = start[ax1] + scale * dir[ax1];
	hit[ax2] = start[ax2] + scale * dir[ax2];
	return ( hit[ax1] >= b[0][ax1] && hit[ax1] <= b[1][ax1] &&
	hit[ax2] >= b[0][ax2] && hit[ax2] <= b[1][ax2] );
}
vec2 OctTexCoord( vec3 worldDir )
{
	vec2 normalizedOctCoord = octEncode( worldDir );
	vec2 normalizedOctCoordZeroOne = ( normalizedOctCoord + vec2( 1.0 ) ) * 0.5;
	#if 0
	vec2 octCoordNormalizedToTextureDimensions = ( normalizedOctCoordZeroOne * ( rpCascadeDistances.x - float( 2.0 ) ) ) / rpCascadeDistances.xy;
	vec2 probeTopLeftPosition = vec2( 1.0, 1.0 );
	vec2 normalizedProbeTopLeftPosition = probeTopLeftPosition * rpCascadeDistances.zw;
	normalizedOctCoordZeroOne.xy = normalizedProbeTopLeftPosition + octCoordNormalizedToTextureDimensions;
	#endif 
	return normalizedOctCoordZeroOne;
}
void main()
{
	vec4 bumpMap = texture( samp0, vofi_TexCoord0.xy );
	vec4 YCoCG = texture( samp2, vofi_TexCoord1.xy );
	vec4 specMapSRGB = texture( samp1, vofi_TexCoord2.xy );
	vec4 specMap = sRGBAToLinearRGBA( specMapSRGB );
	vec3 diffuseMap = sRGBToLinearRGB( ConvertYCoCgToRGB( YCoCG ) );
	vec3 localNormal;
	#if defined(USE_NORMAL_FMT_RGB8)
	localNormal.xy = bumpMap.rg - 0.5;
	#else 
	localNormal.xy = bumpMap.wy - 0.5;
	#endif 
	localNormal.z = sqrt( abs( dot( localNormal.xy, localNormal.xy ) - 0.25 ) );
	localNormal = normalize( localNormal );
	vec3 globalNormal;
	globalNormal.x = dot3( localNormal, vofi_TexCoord4 );
	globalNormal.y = dot3( localNormal, vofi_TexCoord5 );
	globalNormal.z = dot3( localNormal, vofi_TexCoord6 );
	globalNormal = normalize( globalNormal );
	vec3 globalPosition = vofi_TexCoord7.xyz;
	vec3 globalView = normalize( rpGlobalEyePos.xyz - globalPosition );
	vec3 reflectionVector = globalNormal * dot3( globalView, globalNormal );
	reflectionVector = normalize( ( reflectionVector * 2.0 ) - globalView );
	#if 0
	float hitScale = 0.0;
	vec3 bounds[2];
	bounds[0].x = rpWobbleSkyX.x;
	bounds[0].y = rpWobbleSkyX.y;
	bounds[0].z = rpWobbleSkyX.z;
	bounds[1].x = rpWobbleSkyY.x;
	bounds[1].y = rpWobbleSkyY.y;
	bounds[1].z = rpWobbleSkyY.z;
	vec3 rayStart = vofi_TexCoord7.xyz;
	rayStart += reflectionVector * 10000.0;
	if( ( rpWobbleSkyX.w > 0.0 ) && AABBRayIntersection( bounds, rayStart, -reflectionVector, hitScale ) )
	{
		vec3 hitPoint = rayStart - reflectionVector * hitScale;
		reflectionVector = hitPoint - rpWobbleSkyZ.xyz;
	}
	#endif 
	float vDotN = saturate( dot3( globalView, globalNormal ) );
	#if defined( USE_PBR )
	float metallic = specMapSRGB.g;
	float roughness = specMapSRGB.r;
	float glossiness = 1.0 - roughness;
	vec3 dielectricColor = vec3( 0.04 );
	vec3 baseColor = diffuseMap;
	vec3 diffuseColor = baseColor * ( 1.0 - metallic );
	vec3 specularColor = mix( dielectricColor, baseColor, metallic );
	#if defined( DEBUG_PBR )
	diffuseColor = vec3( 0.0, 0.0, 0.0 );
	specularColor = vec3( 0.0, 1.0, 0.0 );
	#endif 
	vec3 kS = Fresnel_SchlickRoughness( specularColor, vDotN, roughness );
	vec3 kD = ( vec3( 1.0, 1.0, 1.0 ) - kS ) * ( 1.0 - metallic );
	#else 
	float roughness = EstimateLegacyRoughness( specMapSRGB.rgb );
	vec3 diffuseColor = diffuseMap;
	vec3 specularColor = specMap.rgb;
	#if defined( DEBUG_PBR )
	diffuseColor = vec3( 0.0, 0.0, 0.0 );
	specularColor = vec3( 1.0, 0.0, 0.0 );
	#endif 
	vec3 kS = Fresnel_SchlickRoughness( specularColor, vDotN, roughness );
	vec3 kD = ( vec3( 1.0, 1.0, 1.0 ) - kS );
	#endif 
	vec2 screenTexCoord = gl_FragCoord.xy * rpWindowCoord.xy;
	float ao = 1.0;
	ao = texture( samp4, screenTexCoord ).r;
	vec2 normalizedOctCoordZeroOne = OctTexCoord( globalNormal );
	vec3 irradiance = texture( samp7, normalizedOctCoordZeroOne ).rgb;
	vec3 diffuseLight = ( kD * irradiance * diffuseColor ) * ao * ( rpDiffuseModifier.xyz * 1.0 );
	float MAX_REFLECTION_LOD = 6.0;
	float mip = clamp( ( roughness * MAX_REFLECTION_LOD ), 0.0, MAX_REFLECTION_LOD );
	normalizedOctCoordZeroOne = OctTexCoord( reflectionVector );
	vec3 radiance = textureLod( samp8, normalizedOctCoordZeroOne, mip ).rgb * rpLocalLightOrigin.x;
	radiance += textureLod( samp9, normalizedOctCoordZeroOne, mip ).rgb * rpLocalLightOrigin.y;
	radiance += textureLod( samp10, normalizedOctCoordZeroOne, mip ).rgb * rpLocalLightOrigin.z;
	vec2 envBRDF = texture( samp3, vec2( max( vDotN, 0.0 ), roughness ) ).rg;
	#if 0
	fo_FragColor.rgb = vec3( envBRDF.x, envBRDF.y, 0.0 );
	fo_FragColor.w = vofi_Color.a;
	return;
	#endif 
	float specAO = ComputeSpecularAO( vDotN, ao, roughness );
	vec3 specularLight = radiance * ( kS * envBRDF.x + vec3( envBRDF.y ) ) * specAO * ( rpSpecularModifier.xyz * 1.0 );
	#if 1
	float horizonFade = 1.3;
	float horiz = saturate( 1.0 + horizonFade * saturate( dot3( reflectionVector, globalNormal ) ) );
	horiz *= horiz;
	#endif 
	vec3 lightColor = ( rpAmbientColor.rgb );
	fo_FragColor.rgb = ( diffuseLight + specularLight * horiz ) * lightColor * vofi_Color.rgb;
	fo_FragColor.w = vofi_Color.a;
}