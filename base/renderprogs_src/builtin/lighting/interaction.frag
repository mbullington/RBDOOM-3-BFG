#pragma static USE_GPU_SKINNING _skinned

#if defined(USE_GPU_SKINNING)
layout( binding = 2 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpDiffuseModifier;
	vec4 rpSpecularModifier;
	vec4 rpJitterTexOffset;
};
#else
layout( binding = 1 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpDiffuseModifier;
	vec4 rpSpecularModifier;
	vec4 rpJitterTexOffset;
};
#endif
#if defined(USE_GPU_SKINNING)
layout( binding = 3 ) uniform sampler2D samp0;
layout( binding = 4 ) uniform sampler2D samp1;
layout( binding = 5 ) uniform sampler2D samp2;
layout( binding = 6 ) uniform sampler2D samp3;
layout( binding = 7 ) uniform sampler2D samp4;
#else
layout( binding = 2 ) uniform sampler2D samp0;
layout( binding = 3 ) uniform sampler2D samp1;
layout( binding = 4 ) uniform sampler2D samp2;
layout( binding = 5 ) uniform sampler2D samp3;
layout( binding = 6 ) uniform sampler2D samp4;
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
layout( location = 7 ) in vec4 vofi_Color;

layout( location = 0 ) out vec4 fo_FragColor;

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
	float lambert = ldotN;
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
	vec3 color = ( diffuseLight + specularLight ) * lightColor * vofi_Color.rgb;
	fo_FragColor.rgb = color;
	fo_FragColor.a = 1.0;
}