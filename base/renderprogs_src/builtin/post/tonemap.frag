#pragma static BRIGHTPASS _brightpass
#pragma static HDR_DEBUG _debug

#if defined(USE_GPU_SKINNING)
layout( binding = 2 ) uniform UBOV {
	vec4 rpScreenCorrectionFactor;
	vec4 rpWindowCoord;
	vec4 rpOverbright;
	vec4 rpJitterTexOffset;
};
#else
layout( binding = 1 ) uniform UBOV {
	vec4 rpScreenCorrectionFactor;
	vec4 rpWindowCoord;
	vec4 rpOverbright;
	vec4 rpJitterTexOffset;
};
#endif
#if defined(USE_GPU_SKINNING)
layout( binding = 3 ) uniform sampler2D samp0;
layout( binding = 4 ) uniform sampler2D samp1;
#else
layout( binding = 2 ) uniform sampler2D samp0;
layout( binding = 3 ) uniform sampler2D samp1;
#endif // else

#include "renderprogs_src/global.glsl"

vec3 ditherRGB( vec3 color, vec2 uvSeed, float quantSteps )
{
	vec3 noise = vec3( InterleavedGradientNoiseAnim( uvSeed, rpJitterTexOffset.w ) );
	#if 1
	noise.x = RemapNoiseTriErp( noise.x );
	noise = noise * 2.0 - 0.5;
	#endif 
	noise = vec3( noise.x );
	float scale = quantSteps - 1.0;
	color += noise / ( quantSteps );
	color = floor( color * scale ) / scale;
	#if defined( USE_LINEAR_RGB )
	#endif 
	return color;
}

layout( location = 0 ) in vec2 vofi_TexCoord0;

layout( location = 0 ) out vec4 fo_FragColor;

vec3 Uncharted2Tonemap( vec3 x )
{
	float A = 0.22;
	float B = 0.3;
	float C = 0.10;
	float D = 0.20;
	float E = 0.01;
	float F = 0.30;
	float W = 11.2;
	return ( ( x * ( A * x + C * B ) + D * E ) / ( x * ( A * x + B ) + D * F ) ) - E / F;
}
vec3 ACESFilm( vec3 x )
{
	float a = 2.51;
	float b = 0.03;
	float c = 2.43;
	float d = 0.59;
	float e = 0.14;
	return saturate( ( x * ( a * x + b ) ) / ( x * ( c * x + d ) + e ) );
}
#define USE_DITHERING 0
void main()
{
	vec2 tCoords = vofi_TexCoord0;
	#if defined(BRIGHTPASS_FILTER)
	tCoords *= vec2( 4.0, 4.0 );
	#endif 
	vec4 color = texture( samp0, tCoords );
	float Y = dot( LUMINANCE_SRGB, color );
	float hdrGamma = 2.2;
	float gamma = hdrGamma;
	#if 0
	color.r = pow( color.r, gamma );
	color.g = pow( color.g, gamma );
	color.b = pow( color.b, gamma );
	#endif 
	#if USE_DITHERING
	float quantSteps = 256.0;
	color.rgb = ditherRGB( color.rgb, gl_FragCoord.xy, quantSteps );
	#endif 
	#if defined(BRIGHTPASS)
	if( Y < 0.1 )
	{
		fo_FragColor = vec4( 0.0, 0.0, 0.0, 1.0 );
		return;
	}
	#endif 
	float hdrKey = rpScreenCorrectionFactor.x;
	float hdrAverageLuminance = rpScreenCorrectionFactor.y;
	float hdrMaxLuminance = rpScreenCorrectionFactor.z;
	float Yr = ( hdrKey * Y ) / hdrAverageLuminance;
	float Ymax = hdrMaxLuminance;
	#define OPERATOR 2
	#if OPERATOR == 0
	float L = 1.0 - exp( -Yr );
	color.rgb *= L;
	#elif OPERATOR == 1
	float exposure = 1.0;
	float L = ( 1.0 - exp( -Yr * exposure ) );
	color.rgb *= L;
	#elif OPERATOR == 2
	float avgLuminance = max( hdrAverageLuminance, 0.001 );
	float linearExposure = ( hdrKey / avgLuminance );
	float exposure = log2( max( linearExposure, 0.0001 ) );
	vec3 exposedColor = exp2( exposure ) * color.rgb;
	color.rgb = ACESFilm( exposedColor );
	#elif OPERATOR == 3
	float exposure = rpScreenCorrectionFactor.w;
	vec3 exposedColor = exp2( exposure ) * color.rgb;
	vec3 curr = ACESFilm( exposedColor );
	vec3 whiteScale = 1.0 / ACESFilm( vec3( Ymax ) );
	color.rgb = curr * whiteScale;
	#elif OPERATOR == 4
	float exposure = rpScreenCorrectionFactor.w;
	vec3 exposedColor = exposure * color.rgb;
	vec3 curr = Uncharted2Tonemap( exposedColor );
	vec3 whiteScale = 1.0 / Uncharted2Tonemap( vec3( Ymax ) );
	color.rgb = curr * whiteScale;
	#endif 
	#if defined(BRIGHTPASS)
	float hdrContrastThreshold = rpOverbright.x;
	float hdrContrastOffset = rpOverbright.y;
	float T = max( Yr - hdrContrastThreshold, 0.0 );
	float B = T > 0.0 ? T / ( hdrContrastOffset + T ) : T;
	color.rgb *= clamp( B, 0.0, 1.0 );
	#endif 
	#if USE_DITHERING
	color.rgb = max( vec3( 0.0 ), color.rgb );
	color.rgb = Srgb3( color.rgb );
	color.rgb = floor( color.rgb * quantSteps ) * ( 1.0 / ( quantSteps - 1.0 ) );
	#else 
	gamma = 1.0 / hdrGamma;
	color.r = pow( color.r, gamma );
	color.g = pow( color.g, gamma );
	color.b = pow( color.b, gamma );
	#endif 
	#if defined(HDR_DEBUG)
	vec3 debugColors[16] = vec3[](
	vec3( 0.0, 0.0, 0.0 ),
	vec3( 0.0, 0.0, 0.1647 ),
	vec3( 0.0, 0.0, 0.3647 ),
	vec3( 0.0, 0.0, 0.6647 ),
	vec3( 0.0, 0.0, 0.9647 ),
	vec3( 0.0, 0.9255, 0.9255 ),
	vec3( 0.0, 0.5647, 0.0 ),
	vec3( 0.0, 0.7843, 0.0 ),
	vec3( 1.0, 1.0, 0.0 ),
	vec3( 0.90588, 0.75294, 0.0 ),
	vec3( 1.0, 0.5647, 0.0 ),
	vec3( 1.0, 0.0, 0.0 ),
	vec3( 0.8392, 0.0, 0.0 ),
	vec3( 1.0, 0.0, 1.0 ),
	vec3( 0.6, 0.3333, 0.7882 ),
	vec3( 1.0, 1.0, 1.0 )
	);
	float v = log2( Y / 0.18 );
	v = clamp( v + 5.0, 0.0, 15.0 );
	int index = int( floor( v ) );
	color.rgb = mix( debugColors[index], debugColors[ min( 15, index + 1 ) ], fract( v ) );
	#endif 
	fo_FragColor = color;
	#if 0
	fo_FragColor = vec4( L, L, L, 1.0 );
	#endif 
}