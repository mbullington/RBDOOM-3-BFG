#pragma static LIGHT_POINT _point
#pragma static LIGHT_PARALLEL _parallel
#pragma static USE_GPU_SKINNING _skinned
#pragma static USE_PBR _PBR

#if defined(USE_GPU_SKINNING)
layout( binding = 2 ) uniform UBOV {
	vec4 rpScreenCorrectionFactor;
	vec4 rpWindowCoord;
	vec4 rpDiffuseModifier;
	vec4 rpSpecularModifier;
	vec4 rpJitterTexScale;
	vec4 rpJitterTexOffset;
	vec4 rpCascadeDistances;
	vec4 rpShadowMatrices[6*4];
};
#else
layout( binding = 1 ) uniform UBOV {
	vec4 rpScreenCorrectionFactor;
	vec4 rpWindowCoord;
	vec4 rpDiffuseModifier;
	vec4 rpSpecularModifier;
	vec4 rpJitterTexScale;
	vec4 rpJitterTexOffset;
	vec4 rpCascadeDistances;
	vec4 rpShadowMatrices[6*4];
};
#endif
#if defined(USE_GPU_SKINNING)
layout( binding = 3 ) uniform sampler2D samp0;
layout( binding = 4 ) uniform sampler2D samp1;
layout( binding = 5 ) uniform sampler2D samp2;
layout( binding = 6 ) uniform sampler2D samp3;
layout( binding = 7 ) uniform sampler2D samp4;
layout( binding = 8 ) uniform sampler2DArrayShadow samp5;
layout( binding = 9 ) uniform sampler2D samp6;
#else
layout( binding = 2 ) uniform sampler2D samp0;
layout( binding = 3 ) uniform sampler2D samp1;
layout( binding = 4 ) uniform sampler2D samp2;
layout( binding = 5 ) uniform sampler2D samp3;
layout( binding = 6 ) uniform sampler2D samp4;
layout( binding = 7 ) uniform sampler2DArrayShadow samp5;
layout( binding = 8 ) uniform sampler2D samp6;
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
layout( location = 8 ) in vec4 vofi_TexCoord8;
layout( location = 9 ) in vec4 vofi_TexCoord9;
layout( location = 10 ) in vec4 vofi_Color;

layout( location = 0 ) out vec4 fo_FragColor;

float BlueNoise( vec2 n, float x )
{
	vec2 uv = n.xy * rpJitterTexOffset.xy;
	float noise = texture( samp6, uv ).r;
	noise = fract( noise + c_goldenRatioConjugate * rpJitterTexOffset.w * x );
	return noise;
}
vec2 VogelDiskSample( float sampleIndex, float samplesCount, float phi )
{
	float goldenAngle = 2.4;
	float r = sqrt( sampleIndex + 0.5 ) / sqrt( samplesCount );
	float theta = sampleIndex * goldenAngle + phi;
	float sine = sin( theta );
	float cosine = cos( theta );
	return vec2( r * cosine, r * sine );
}
void main()
{
	vec4 bumpMap = texture( samp0, vofi_TexCoord1.xy );
	vec4 lightFalloff = ( idtex2Dproj( samp3, vofi_TexCoord2 ) );
	vec4 lightProj = ( idtex2Dproj( samp4, vofi_TexCoord3 ) );
	vec4 YCoCG = texture( samp2, vofi_TexCoord4.xy );
	vec4 specMapSRGB = texture( samp1, vofi_TexCoord5.xy );
	vec4 specMap = sRGBAToLinearRGBA( specMapSRGB );
	vec3 lightVector = normalize( vofi_TexCoord0.xyz );
	vec3 viewVector = normalize( vofi_TexCoord6.xyz );
	vec3 diffuseMap = sRGBToLinearRGB( ConvertYCoCgToRGB( YCoCG ) );
	vec3 localNormal;
	#if defined(USE_NORMAL_FMT_RGB8)
	localNormal.xy = bumpMap.rg - 0.5;
	#else 
	localNormal.xy = bumpMap.wy - 0.5;
	#endif 
	localNormal.z = sqrt( abs( dot( localNormal.xy, localNormal.xy ) - 0.25 ) );
	localNormal = normalize( localNormal );
	float ldotN = saturate( dot3( localNormal, lightVector ) );
	float halfLdotN = dot3( localNormal, lightVector ) * 0.5 + 0.5;
	halfLdotN *= halfLdotN;
	float lambert = mix( ldotN, halfLdotN, 0.5 );
	int shadowIndex = 0;
	#if defined( LIGHT_POINT )
	vec3 toLightGlobal = normalize( vofi_TexCoord8.xyz );
	float axis[6];
	axis[0] = -toLightGlobal.x;
	axis[1] = toLightGlobal.x;
	axis[2] = -toLightGlobal.y;
	axis[3] = toLightGlobal.y;
	axis[4] = -toLightGlobal.z;
	axis[5] = toLightGlobal.z;
	for( int i = 0; i < 6; i++ )
	{
		if( axis[i] > axis[shadowIndex] )
		{
			shadowIndex = i;
		}
	}
	#endif // #if defined( POINTLIGHT )
	#if defined( LIGHT_PARALLEL )
	float viewZ = -vofi_TexCoord9.z;
	shadowIndex = 4;
	for( int i = 0; i < 4; i++ )
	{
		if( viewZ < rpCascadeDistances[i] )
		{
			shadowIndex = i;
			break;
		}
	}
	#endif 
	#if 0
	if( shadowIndex == 0 )
	{
		fo_FragColor = vec4( 1.0, 0.0, 0.0, 1.0 );
	}
	else if( shadowIndex == 1 )
	{
		fo_FragColor = vec4( 0.0, 1.0, 0.0, 1.0 );
	}
	else if( shadowIndex == 2 )
	{
		fo_FragColor = vec4( 0.0, 0.0, 1.0, 1.0 );
	}
	else if( shadowIndex == 3 )
	{
		fo_FragColor = vec4( 1.0, 1.0, 0.0, 1.0 );
	}
	else if( shadowIndex == 4 )
	{
		fo_FragColor = vec4( 1.0, 0.0, 1.0, 1.0 );
	}
	else if( shadowIndex == 5 )
	{
		fo_FragColor = vec4( 0.0, 1.0, 1.0, 1.0 );
	}
	return;
	#endif 
	vec4 shadowMatrixX = rpShadowMatrices[ int ( shadowIndex * 4 + 0 ) ];
	vec4 shadowMatrixY = rpShadowMatrices[ int ( shadowIndex * 4 + 1 ) ];
	vec4 shadowMatrixZ = rpShadowMatrices[ int ( shadowIndex * 4 + 2 ) ];
	vec4 shadowMatrixW = rpShadowMatrices[ int ( shadowIndex * 4 + 3 ) ];
	vec4 modelPosition = vec4( vofi_TexCoord7.xyz, 1.0 );
	vec4 shadowTexcoord;
	shadowTexcoord.x = dot4( modelPosition, shadowMatrixX );
	shadowTexcoord.y = dot4( modelPosition, shadowMatrixY );
	shadowTexcoord.z = dot4( modelPosition, shadowMatrixZ );
	shadowTexcoord.w = dot4( modelPosition, shadowMatrixW );
	float bias = 0.001;
	shadowTexcoord.xyz /= shadowTexcoord.w;
	shadowTexcoord.z = shadowTexcoord.z * rpScreenCorrectionFactor.w;
	shadowTexcoord.w = float( shadowIndex );
	#if 0
	fo_FragColor.xyz = vec3( shadowTexcoord.z, shadowTexcoord.z, shadowTexcoord.z );
	fo_FragColor.w = 1.0;
	return;
	#endif 
	#if 0
	vec4 base = shadowTexcoord;
	base.xy += rpJitterTexScale.xy * -0.5;
	float shadow = 0.0;
	float numSamples = 16;
	float stepSize = 1.0 / numSamples;
	vec2 jitterTC = ( gl_FragCoord.xy * rpScreenCorrectionFactor.xy ) + rpJitterTexOffset.ww;
	for( float i = 0.0; i < numSamples; i += 1.0 )
	{
		vec4 jitter = base + texture( samp6, jitterTC.xy ) * rpJitterTexScale;
		jitter.zw = shadowTexcoord.zw;
		shadow += texture( samp5, jitter.xywz );
		jitterTC.x += stepSize;
	}
	shadow *= stepSize;
	#elif 0
	vec2 poissonDisk[12] = vec2[](
	vec2( 0.6111618, 0.1050905 ),
	vec2( 0.1088336, 0.1127091 ),
	vec2( 0.3030421, -0.6292974 ),
	vec2( 0.4090526, 0.6716492 ),
	vec2( -0.1608387, -0.3867823 ),
	vec2( 0.7685862, -0.6118501 ),
	vec2( -0.1935026, -0.856501 ),
	vec2( -0.4028573, 0.07754025 ),
	vec2( -0.6411021, -0.4748057 ),
	vec2( -0.1314865, 0.8404058 ),
	vec2( -0.7005203, 0.4596822 ),
	vec2( -0.9713828, -0.06329931 ) );
	float shadow = 0.0;
	float numSamples = 12.0;
	float stepSize = 1.0 / numSamples;
	vec4 jitterTC = ( gl_FragCoord * rpScreenCorrectionFactor ) + rpJitterTexOffset;
	vec4 random = texture( samp6, jitterTC.xy ) * PI;
	vec2 rot;
	rot.x = cos( random.x );
	rot.y = sin( random.x );
	float shadowTexelSize = rpScreenCorrectionFactor.z * rpJitterTexScale.x;
	for( int i = 0; i < 12; i++ )
	{
		vec2 jitter = poissonDisk[i];
		vec2 jitterRotated;
		jitterRotated.x = jitter.x * rot.x - jitter.y * rot.y;
		jitterRotated.y = jitter.x * rot.y + jitter.y * rot.x;
		vec4 shadowTexcoordJittered = vec4( shadowTexcoord.xy + jitterRotated * shadowTexelSize, shadowTexcoord.z, shadowTexcoord.w );
		shadow += texture( samp5, shadowTexcoordJittered.xywz );
	}
	shadow *= stepSize;
	#elif 1
	#if 0
	vec2 poissonDisk[12] = vec2[](
	vec2( 0.6111618, 0.1050905 ),
	vec2( 0.1088336, 0.1127091 ),
	vec2( 0.3030421, -0.6292974 ),
	vec2( 0.4090526, 0.6716492 ),
	vec2( -0.1608387, -0.3867823 ),
	vec2( 0.7685862, -0.6118501 ),
	vec2( -0.1935026, -0.856501 ),
	vec2( -0.4028573, 0.07754025 ),
	vec2( -0.6411021, -0.4748057 ),
	vec2( -0.1314865, 0.8404058 ),
	vec2( -0.7005203, 0.4596822 ),
	vec2( -0.9713828, -0.06329931 ) );
	float shadow = 0.0;
	float numSamples = 6.0;
	float stepSize = 1.0 / numSamples;
	float random = BlueNoise( gl_FragCoord.xy, 1.0 );
	random *= PI;
	vec2 rot;
	rot.x = cos( random );
	rot.y = sin( random );
	float shadowTexelSize = rpScreenCorrectionFactor.z * rpJitterTexScale.x;
	for( int i = 0; i < 6; i++ )
	{
		vec2 jitter = poissonDisk[i];
		vec2 jitterRotated;
		jitterRotated.x = jitter.x * rot.x - jitter.y * rot.y;
		jitterRotated.y = jitter.x * rot.y + jitter.y * rot.x;
		vec4 shadowTexcoordJittered = vec4( shadowTexcoord.xy + jitterRotated * shadowTexelSize, shadowTexcoord.z, shadowTexcoord.w );
		shadow += texture( samp5, shadowTexcoordJittered.xywz );
	}
	shadow *= stepSize;
	#else 
	float shadow = 0.0;
	float numSamples = rpJitterTexScale.w;
	float stepSize = 1.0 / numSamples;
	float vogelPhi = BlueNoise( gl_FragCoord.xy, 1.0 );
	float shadowTexelSize = rpScreenCorrectionFactor.z * rpJitterTexScale.x;
	for( float i = 0.0; i < numSamples; i += 1.0 )
	{
		vec2 jitter = VogelDiskSample( i, numSamples, vogelPhi );
		vec4 shadowTexcoordJittered = vec4( shadowTexcoord.xy + jitter * shadowTexelSize, shadowTexcoord.z, shadowTexcoord.w );
		shadow += texture( samp5, shadowTexcoordJittered.xywz );
	}
	shadow *= stepSize;
	#endif 
	#else 
	float shadow = texture( samp5, shadowTexcoord.xywz );
	#endif 
	vec3 halfAngleVector = normalize( lightVector + viewVector );
	float hdotN = clamp( dot3( halfAngleVector, localNormal ), 0.0, 1.0 );
	#if defined( USE_PBR )
	float metallic = specMapSRGB.g;
	float roughness = specMapSRGB.r;
	float glossiness = 1.0 - roughness;
	vec3 dielectricColor = vec3( 0.04 );
	vec3 baseColor = diffuseMap;
	vec3 diffuseColor = baseColor * ( 1.0 - metallic );
	vec3 specularColor = mix( dielectricColor, baseColor, metallic );
	#else 
	float roughness = EstimateLegacyRoughness( specMapSRGB.rgb );
	vec3 diffuseColor = diffuseMap;
	vec3 specularColor = specMapSRGB.rgb;
	#endif 
	vec3 lightColor = sRGBToLinearRGB( lightProj.xyz * lightFalloff.xyz );
	float vdotN = clamp( dot3( viewVector, localNormal ), 0.0, 1.0 );
	float vdotH = clamp( dot3( viewVector, halfAngleVector ), 0.0, 1.0 );
	float ldotH = clamp( dot3( lightVector, halfAngleVector ), 0.0, 1.0 );
	vec3 reflectColor = specularColor * rpSpecularModifier.rgb * 1.0;
	float rr = roughness * roughness;
	float rrrr = rr * rr;
	float D = ( hdotN * hdotN ) * ( rrrr - 1.0 ) + 1.0;
	float VFapprox = ( ldotH * ldotH ) * ( roughness + 0.5 );
	vec3 specularLight = ( rrrr / ( 4.0 * PI * D * D * VFapprox ) ) * ldotN * reflectColor;
	#if 0
	fo_FragColor = vec4( _half3( VFapprox ), 1.0 );
	return;
	#endif 
	vec3 diffuseLight = diffuseColor * lambert * ( rpDiffuseModifier.xyz );
	vec3 color = ( diffuseLight + specularLight ) * lightColor * vofi_Color.rgb * shadow;
	fo_FragColor.rgb = color;
	fo_FragColor.a = 1.0;
}