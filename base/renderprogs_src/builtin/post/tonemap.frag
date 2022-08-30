#pragma static BRIGHTPASS _brightpass
#pragma static HDR_DEBUG _debug

#include "renderprogs_src/global.glsl"

layout( binding = 0 ) uniform UBOV {
	vec4 rpScreenCorrectionFactor;
};

layout( binding = 1 ) uniform sampler2D samp0;

layout( location = 0 ) in vec2 vofi_TexCoord0;
layout( location = 0 ) out vec4 fo_FragColor;

vec3 ACESFilm( vec3 x )
{
	float a = 2.51;
	float b = 0.03;
	float c = 2.43;
	float d = 0.59;
	float e = 0.14;
	return saturate( ( x * ( a * x + b ) ) / ( x * ( c * x + d ) + e ) );
}

void main()
{
	vec2 tCoords = vofi_TexCoord0;
	vec4 color = texture( samp0, tCoords );
	float Y = dot( LUMINANCE_SRGB, color );
	float hdrGamma = 2.2;
	float gamma = hdrGamma;

	float hdrKey = rpScreenCorrectionFactor.x;
	float hdrAverageLuminance = rpScreenCorrectionFactor.y;
	float hdrMaxLuminance = rpScreenCorrectionFactor.z;
	float Yr = ( hdrKey * Y ) / hdrAverageLuminance;
	float Ymax = hdrMaxLuminance;

	float avgLuminance = max( hdrAverageLuminance, 0.001 );
	float linearExposure = ( hdrKey / avgLuminance );
	float exposure = log2( max( linearExposure, 0.0001 ) );
	vec3 exposedColor = exp2( exposure ) * color.rgb;
	color.rgb = ACESFilm( exposedColor );

	gamma = 1.0 / hdrGamma;
	color.r = pow( color.r, gamma );
	color.g = pow( color.g, gamma );
	color.b = pow( color.b, gamma );

	fo_FragColor = color;
}