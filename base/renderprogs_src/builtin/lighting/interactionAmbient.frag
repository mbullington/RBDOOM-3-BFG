
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

layout( location = 0 ) in vec4 vofi_TexCoord1;
layout( location = 1 ) in vec4 vofi_TexCoord2;
layout( location = 2 ) in vec4 vofi_TexCoord3;
layout( location = 3 ) in vec4 vofi_TexCoord4;
layout( location = 4 ) in vec4 vofi_TexCoord5;
layout( location = 5 ) in vec4 vofi_TexCoord6;
layout( location = 6 ) in vec4 vofi_Color;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	vec4 bumpMap = texture( samp0, vofi_TexCoord1.xy );
	vec4 lightFalloff = idtex2Dproj( samp3, vofi_TexCoord2 );
	vec4 lightProj = idtex2Dproj( samp4, vofi_TexCoord3 );
	vec4 YCoCG = texture( samp2, vofi_TexCoord4.xy );
	vec4 specMapSRGB = texture( samp1, vofi_TexCoord5.xy );
	vec4 specMap = sRGBAToLinearRGBA( specMapSRGB );
	vec3 ambientLightVector = vec3( 0.5, 9.5 - 0.385, 0.8925 );
	vec3 lightVector = normalize( ambientLightVector );
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
	#if defined(USE_HALF_LAMBERT)
	float halfLdotN = dot3( localNormal, lightVector ) * 0.5 + 0.5;
	halfLdotN *= halfLdotN;
	float lambert = halfLdotN;
	#else 
	float lambert = ldotN;
	#endif 
	float specularPower = 10.0;
	float hDotN = dot3( normalize( vofi_TexCoord6.xyz ), localNormal );
	vec3 specularContribution = _half3( pow( abs( hDotN ), specularPower ) );
	vec3 diffuseColor = diffuseMap * ( rpDiffuseModifier.xyz );
	vec3 specularColor = specMap.xyz * specularContribution * ( rpSpecularModifier.xyz );
	vec3 lightColor = sRGBToLinearRGB( lightProj.xyz * lightFalloff.xyz );
	fo_FragColor.xyz = ( diffuseColor + specularColor ) * lightColor * vofi_Color.xyz;
	fo_FragColor.w = 1.0;
}