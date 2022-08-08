
#if defined(USE_GPU_SKINNING)
layout( binding = 2 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpDiffuseModifier;
	vec4 rpSpecularModifier;
	vec4 rpAmbientColor;
	vec4 rpJitterTexOffset;
};
#else
layout( binding = 1 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpDiffuseModifier;
	vec4 rpSpecularModifier;
	vec4 rpAmbientColor;
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

layout( location = 0 ) in vec4 vofi_TexCoord0;
layout( location = 1 ) in vec4 vofi_TexCoord1;
layout( location = 2 ) in vec4 vofi_TexCoord4;
layout( location = 3 ) in vec4 vofi_TexCoord5;
layout( location = 4 ) in vec4 vofi_TexCoord6;
layout( location = 5 ) in vec4 vofi_Color;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	vec4 bumpMap = texture( samp0, vofi_TexCoord1.xy );
	vec4 YCoCG = texture( samp2, vofi_TexCoord4.xy );
	vec4 specMap = sRGBAToLinearRGBA( texture( samp1, vofi_TexCoord5.xy ) );
	vec3 lightVector = normalize( vofi_TexCoord0.xyz );
	vec3 diffuseMap = sRGBToLinearRGB( ConvertYCoCgToRGB( YCoCG ) );
	vec3 localNormal;
	#if defined(USE_NORMAL_FMT_RGB8)
	localNormal.xy = bumpMap.rg - 0.5;
	#else 
	localNormal.xy = bumpMap.wy - 0.5;
	#endif 
	localNormal.z = sqrt( abs( dot( localNormal.xy, localNormal.xy ) - 0.25 ) );
	localNormal = normalize( localNormal );
	float specularPower = 10.0;
	float hDotN = dot3( normalize( vofi_TexCoord6.xyz ), localNormal );
	vec3 specularContribution = _half3( pow( abs( hDotN ), specularPower ) );
	vec3 diffuseColor = diffuseMap * ( rpDiffuseModifier.xyz * 0.5 );
	vec3 specularColor = specMap.xyz * specularContribution * ( rpSpecularModifier.xyz );
	float halfLdotN = dot3( localNormal, lightVector ) * 0.5 + 0.5;
	halfLdotN *= halfLdotN;
	float ldotN = dot3( localNormal, lightVector );
	vec3 lightColor = sRGBToLinearRGB( rpAmbientColor.rgb );
	float rim = 1.0 - saturate( hDotN );
	float rimPower = 8.0;
	vec3 rimColor = sRGBToLinearRGB( vec3( 0.125 ) * 1.2 ) * lightColor * pow( rim, rimPower );
	fo_FragColor.xyz = ( ( diffuseColor + specularColor ) * halfLdotN * lightColor + rimColor ) * vofi_Color.rgb;
	fo_FragColor.w = vofi_Color.a;
}