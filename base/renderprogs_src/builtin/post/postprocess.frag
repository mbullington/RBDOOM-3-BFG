layout( binding = 1 ) uniform UBOV {
 	vec4 rpJitterTexOffset;
};
layout( binding = 2 ) uniform sampler2D samp0;
layout( binding = 3 ) uniform sampler2D samp1;

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec2 vofi_TexCoord0;

layout( location = 0 ) out vec4 fo_FragColor;

#define USE_CHROMATIC_ABERRATION   1
#define Chromatic_Amount     0.075
#define USE_TECHNICOLOR      0  // [0 or 1]
#define Technicolor_Amount     1.0  // [0.00 to 1.00]
#define Technicolor_Power     4.0  // [0.00 to 8.00]
#define Technicolor_RedNegativeAmount  0.88 // [0.00 to 1.00]
#define Technicolor_GreenNegativeAmount  0.88 // [0.00 to 1.00]
#define Technicolor_BlueNegativeAmount  0.88 // [0.00 to 1.00]
#define USE_VIBRANCE      0
#define Vibrance       0.5 // [-1.00 to 1.00]
#define Vibrance_RGB_Balance    vec3( 1.0, 1.0, 1.0 )
#define USE_CAS                             0
#define USE_DITHERING       1
#define Dithering_QuantizationSteps         16.0 // 8.0 = 2 ^ 3 quantization bits
#define Dithering_NoiseBoost                1.0
#define Dithering_Wide                      1.0
#define DITHER_IN_LINEAR_SPACE              0
#define DITHER_GENERATE_NOISE               0
vec3 overlay( vec3 a, vec3 b )
{
	return vec3(
	b.x < 0.5 ? ( 2.0 * a.x * b.x ) : ( 1.0 - 2.0 * ( 1.0 - a.x ) * ( 1.0 - b.x ) ),
	b.y < 0.5 ? ( 2.0 * a.y * b.y ) : ( 1.0 - 2.0 * ( 1.0 - a.y ) * ( 1.0 - b.y ) ),
	b.z < 0.5 ? ( 2.0 * a.z * b.z ) : ( 1.0 - 2.0 * ( 1.0 - a.z ) * ( 1.0 - b.z ) ) );
}
void TechnicolorPass( inout vec4 color )
{
	const vec3 cyanFilter = vec3( 0.0, 1.30, 1.0 );
	const vec3 magentaFilter = vec3( 1.0, 0.0, 1.05 );
	const vec3 yellowFilter = vec3( 1.6, 1.6, 0.05 );
	const vec3 redOrangeFilter = vec3( 1.05, 0.62, 0.0 );
	const vec3 greenFilter = vec3( 0.3, 1.0, 0.0 );
	vec2 redNegativeMul = color.rg * ( 1.0 / ( Technicolor_RedNegativeAmount * Technicolor_Power ) );
	vec2 greenNegativeMul = color.rg * ( 1.0 / ( Technicolor_GreenNegativeAmount * Technicolor_Power ) );
	vec2 blueNegativeMul = color.rb * ( 1.0 / ( Technicolor_BlueNegativeAmount * Technicolor_Power ) );
	float redNegative = dot( redOrangeFilter.rg, redNegativeMul );
	float greenNegative = dot( greenFilter.rg, greenNegativeMul );
	float blueNegative = dot( magentaFilter.rb, blueNegativeMul );
	vec3 redOutput = vec3( redNegative ) + cyanFilter;
	vec3 greenOutput = vec3( greenNegative ) + magentaFilter;
	vec3 blueOutput = vec3( blueNegative ) + yellowFilter;
	vec3 result = redOutput * greenOutput * blueOutput;
	color.rgb = mix( color.rgb, result, Technicolor_Amount );
}
void VibrancePass( inout vec4 color )
{
	const vec3 vibrance = Vibrance_RGB_Balance * Vibrance;
	float Y = dot( LUMINANCE_SRGB, color );
	float minColor = min( color.r, min( color.g, color.b ) );
	float maxColor = max( color.r, max( color.g, color.b ) );
	float colorSat = maxColor - minColor;
	color.rgb = mix( vec3( Y ), color.rgb, ( 1.0 + ( vibrance * ( 1.0 - ( sign( vibrance ) * colorSat ) ) ) ) );
}
vec2 BarrelDistortion( vec2 xy, float amount )
{
	vec2 cc = xy - 0.5;
	float dist = dot2( cc, cc );
	return xy + cc * dist * amount;
}
float Linterp( float t )
{
	return saturate( 1.0 - abs( 2.0 * t - 1.0 ) );
}
float Remap( float t, float a, float b )
{
	return saturate( ( t - a ) / ( b - a ) );
}
vec3 SpectrumOffset( float t )
{
	float lo = step( t, 0.5 );
	float hi = 1.0 - lo;
	float w = Linterp( Remap( t, 1.0 / 6.0, 5.0 / 6.0 ) );
	vec3 ret = vec3( lo, 1.0, hi ) * vec3( 1.0 - w, w, 1.0 - w );
	return pow( ret, vec3( 1.0 / 2.2 ) );
}
void ChromaticAberrationPass( inout vec4 color )
{
	float amount = Chromatic_Amount;
	vec3 sum = vec3( 0.0 );
	vec3 sumColor = vec3( 0.0 );
	float samples = 12.0;
	for( float i = 0.0; i < samples; i = i + 1.0 )
	{
		float t = ( i / ( samples - 1.0 ) );
		vec3 so = SpectrumOffset( t );
		sum += so.xyz;
		sumColor += so * texture( samp0, BarrelDistortion( vofi_TexCoord0, ( 0.5 * amount * t ) ) ).rgb;
	}
	vec3 outColor = ( sumColor / sum );
	color.rgb = mix( color.rgb, outColor, Technicolor_Amount );
}
void ChromaticAberrationPass2( inout vec4 color )
{
	float amount = 6.0;
	vec2 uv = vofi_TexCoord0;
	vec2 texel = 1.0 / vec2( 1920.0, 1080.0 );
	vec2 coords = ( uv - 0.5 ) * 2.0;
	float coordDot = dot( coords, coords );
	vec2 precompute = amount * coordDot * coords;
	vec2 uvR = uv - texel.xy * precompute;
	vec2 uvB = uv + texel.xy * precompute;
	vec3 outColor;
	outColor.r = texture( samp0, uvR ).r;
	outColor.g = texture( samp0, uv ).g;
	outColor.b = texture( samp0, uvB ).b;
	color.rgb = mix( color.rgb, outColor, Technicolor_Amount );
}
float BlueNoise( vec2 n, float x )
{
	float noise = texture( samp1, ( n.xy * rpJitterTexOffset.xy ) * 1.0 ).r;
	noise = fract( noise + 0.61803398875 * rpJitterTexOffset.z * x );
	noise = RemapNoiseTriErp( noise );
	noise = noise * 2.0 - 1.0;
	return noise;
}
vec3 BlueNoise3( vec2 n, float x )
{
	vec2 uv = n.xy * rpJitterTexOffset.xy;
	vec3 noise = texture( samp1, uv ).rgb;
	noise = fract( noise + c_goldenRatioConjugate * rpJitterTexOffset.w * x );
	return noise;
}
float Noise( vec2 n, float x )
{
	n += x;
	#if 1
	return fract( sin( dot( n.xy, vec2( 12.9898, 78.233 ) ) ) * 43758.5453 ) * 2.0 - 1.0;
	#else 
	return BlueNoise( n, 1.0 );
	#endif 
}
float Step1( vec2 uv, float n )
{
	float a = 1.0, b = 2.0, c = -12.0, t = 1.0;
	return ( 1.0 / ( a * 4.0 + b * 4.0 - c ) ) * (
	Noise( uv + vec2( -1.0, -1.0 ) * t, n ) * a +
	Noise( uv + vec2( 0.0, -1.0 ) * t, n ) * b +
	Noise( uv + vec2( 1.0, -1.0 ) * t, n ) * a +
	Noise( uv + vec2( -1.0, 0.0 ) * t, n ) * b +
	Noise( uv + vec2( 0.0, 0.0 ) * t, n ) * c +
	Noise( uv + vec2( 1.0, 0.0 ) * t, n ) * b +
	Noise( uv + vec2( -1.0, 1.0 ) * t, n ) * a +
	Noise( uv + vec2( 0.0, 1.0 ) * t, n ) * b +
	Noise( uv + vec2( 1.0, 1.0 ) * t, n ) * a +
	0.0 );
}
float Step2( vec2 uv, float n )
{
	float a = 1.0, b = 2.0, c = -2.0, t = 1.0;
	#if DITHER_IN_LINEAR_SPACE
	return ( 1.0 / ( a * 4.0 + b * 4.0 - c ) ) * (
	#else 
	return ( 4.0 / ( a * 4.0 + b * 4.0 - c ) ) * (
	#endif 
	Step1( uv + vec2( -1.0, -1.0 ) * t, n ) * a +
	Step1( uv + vec2( 0.0, -1.0 ) * t, n ) * b +
	Step1( uv + vec2( 1.0, -1.0 ) * t, n ) * a +
	Step1( uv + vec2( -1.0, 0.0 ) * t, n ) * b +
	Step1( uv + vec2( 0.0, 0.0 ) * t, n ) * c +
	Step1( uv + vec2( 1.0, 0.0 ) * t, n ) * b +
	Step1( uv + vec2( -1.0, 1.0 ) * t, n ) * a +
	Step1( uv + vec2( 0.0, 1.0 ) * t, n ) * b +
	Step1( uv + vec2( 1.0, 1.0 ) * t, n ) * a +
	0.0 );
}
vec3 Step3( vec2 uv )
{
	#if DITHER_GENERATE_NOISE
	float a = Step2( uv, 0.07 );
	float b = Step2( uv, 0.11 );
	float c = Step2( uv, 0.13 );
	return vec3( a, b, c );
	#else 
	vec3 noise = BlueNoise3( uv, 0.0 );
	#if 1
	noise.x = RemapNoiseTriErp( noise.x );
	noise.y = RemapNoiseTriErp( noise.y );
	noise.z = RemapNoiseTriErp( noise.z );
	noise = noise * 2.0 - 1.0;
	#endif 
	return noise;
	#endif 
}
vec3 Step3T( vec2 uv )
{
	#if DITHER_GENERATE_NOISE
	float a = Step2( uv, 0.07 * fract( rpJitterTexOffset.z ) );
	float b = Step2( uv, 0.11 * fract( rpJitterTexOffset.z ) );
	float c = Step2( uv, 0.13 * fract( rpJitterTexOffset.z ) );
	return vec3( a, b, c );
	#else 
	vec3 noise = BlueNoise3( uv, 1.0 );
	#if 1
	noise.x = RemapNoiseTriErp( noise.x );
	noise.y = RemapNoiseTriErp( noise.y );
	noise.z = RemapNoiseTriErp( noise.z );
	noise = noise * 2.0 - 1.0;
	#endif 
	return noise;
	#endif 
}
void DitheringPass( inout vec4 fragColor )
{
	vec2 uv = gl_FragCoord.xy * 1.0;
	vec2 uv2 = vofi_TexCoord0;
	vec3 color = fragColor.rgb;
	#if 0
	if( uv2.y >= 0.975 )
	{
		color = vec3( uv2.x );
	}
	else if( uv2.y >= 0.95 )
	{
		color = vec3( uv2.x );
		color = floor( color * Dithering_QuantizationSteps ) * ( 1.0 / ( Dithering_QuantizationSteps - 1.0 ) );
	}
	else if( uv2.y >= 0.925 )
	{
		color = vec3( uv2.x );
		color = floor( color * Dithering_QuantizationSteps + Step3( uv ) * Dithering_NoiseBoost ) * ( 1.0 / ( Dithering_QuantizationSteps - 1.0 ) );
	}
	else if( uv2.y >= 0.9 )
	{
		color = Step3( uv ) * ( 0.25 * Dithering_NoiseBoost ) + 0.5;
	}
	else
	#endif 
	{
		#if DITHER_IN_LINEAR_SPACE
		color = Linear3( color );
		#if 0
		#if 1
		vec3 amount = Linear3( Srgb3( color ) + ( Dithering_NoiseBoost / Dithering_QuantizationSteps ) ) - color;
		#else 
		float luma = PhotoLuma( color );
		float amount = Linear1( Srgb1( luma ) + ( Dithering_NoiseBoost / Dithering_QuantizationSteps ) ) - luma;
		#endif 
		#else 
		#if 1
		float luma = PhotoLuma( color );
		float amount = mix(
		Linear1( Dithering_NoiseBoost / Dithering_QuantizationSteps ),
		Linear1( ( Dithering_NoiseBoost / Dithering_QuantizationSteps ) + 1.0 ) - 1.0,
		luma );
		#else 
		vec3 amount = mix(
		vec3( Linear1( Dithering_NoiseBoost / Dithering_QuantizationSteps ) ),
		vec3( Linear1( ( Dithering_NoiseBoost / Dithering_QuantizationSteps ) + 1.0 ) - 1.0 ),
		color );
		#endif 
		#endif 
		color += Step3T( uv ) * amount;
		color = max( vec3( 0.0 ), color );
		color = Srgb3( color );
		color = floor( color * Dithering_QuantizationSteps ) * ( 1.0 / ( Dithering_QuantizationSteps - 1.0 ) );
		#else 
		#if 0
		if( uv2.x <= 0.5 )
		{
			color = floor( 0.5 + color * ( Dithering_QuantizationSteps + Dithering_Wide - 1.0 ) + ( -Dithering_Wide * 0.5 ) ) * ( 1.0 / ( Dithering_QuantizationSteps - 1.0 ) );
		}
		else
		#endif 
		{
			color = floor( 0.5 + color * ( Dithering_QuantizationSteps + Dithering_Wide - 1.0 ) + ( -Dithering_Wide * 0.5 ) + Step3T( uv ) * ( Dithering_Wide ) ) * ( 1.0 / ( Dithering_QuantizationSteps - 1.0 ) );
		}
		#endif 
	}
	fragColor.rgb = color;
}
float Min3( float x, float y, float z )
{
	return min( x, min( y, z ) );
}
float Max3( float x, float y, float z )
{
	return max( x, max( y, z ) );
}
float rcp( float v )
{
	return 1.0 / v;
}
void ContrastAdaptiveSharpeningPass( inout vec4 color )
{
	vec2 texcoord = vofi_TexCoord0;
	float Sharpness = 1;
	ivec2 bufferSize = textureSize( samp0, 0 );
	float pixelX = ( 1.0 / bufferSize.x );
	float pixelY = ( 1.0 / bufferSize.y );
	vec3 a = texture( samp0, texcoord + vec2( -pixelX, -pixelY ) ).rgb;
	vec3 b = texture( samp0, texcoord + vec2( 0.0, -pixelY ) ).rgb;
	vec3 c = texture( samp0, texcoord + vec2( pixelX, -pixelY ) ).rgb;
	vec3 d = texture( samp0, texcoord + vec2( -pixelX, 0.0 ) ).rgb;
	vec3 e = texture( samp0, texcoord ).rgb;
	vec3 f = texture( samp0, texcoord + vec2( pixelX, 0.0 ) ).rgb;
	vec3 g = texture( samp0, texcoord + vec2( -pixelX, pixelY ) ).rgb;
	vec3 h = texture( samp0, texcoord + vec2( 0.0, pixelY ) ).rgb;
	vec3 i = texture( samp0, texcoord + vec2( pixelX, pixelY ) ).rgb;
	float mnR = Min3( Min3( d.r, e.r, f.r ), b.r, h.r );
	float mnG = Min3( Min3( d.g, e.g, f.g ), b.g, h.g );
	float mnB = Min3( Min3( d.b, e.b, f.b ), b.b, h.b );
	float mnR2 = Min3( Min3( mnR, a.r, c.r ), g.r, i.r );
	float mnG2 = Min3( Min3( mnG, a.g, c.g ), g.g, i.g );
	float mnB2 = Min3( Min3( mnB, a.b, c.b ), g.b, i.b );
	mnR = mnR + mnR2;
	mnG = mnG + mnG2;
	mnB = mnB + mnB2;
	float mxR = Max3( Max3( d.r, e.r, f.r ), b.r, h.r );
	float mxG = Max3( Max3( d.g, e.g, f.g ), b.g, h.g );
	float mxB = Max3( Max3( d.b, e.b, f.b ), b.b, h.b );
	float mxR2 = Max3( Max3( mxR, a.r, c.r ), g.r, i.r );
	float mxG2 = Max3( Max3( mxG, a.g, c.g ), g.g, i.g );
	float mxB2 = Max3( Max3( mxB, a.b, c.b ), g.b, i.b );
	mxR = mxR + mxR2;
	mxG = mxG + mxG2;
	mxB = mxB + mxB2;
	float rcpMR = rcp( mxR );
	float rcpMG = rcp( mxG );
	float rcpMB = rcp( mxB );
	float ampR = saturate( min( mnR, 2.0 - mxR ) * rcpMR );
	float ampG = saturate( min( mnG, 2.0 - mxG ) * rcpMG );
	float ampB = saturate( min( mnB, 2.0 - mxB ) * rcpMB );
	ampR = sqrt( ampR );
	ampG = sqrt( ampG );
	ampB = sqrt( ampB );
	float peak = -rcp( mix( 8.0, 5.0, saturate( Sharpness ) ) );
	float wR = ampR * peak;
	float wG = ampG * peak;
	float wB = ampB * peak;
	float rcpWeightR = rcp( 1.0 + 4.0 * wR );
	float rcpWeightG = rcp( 1.0 + 4.0 * wG );
	float rcpWeightB = rcp( 1.0 + 4.0 * wB );
	vec3 outColor = vec3( saturate( ( b.r * wR + d.r * wR + f.r * wR + h.r * wR + e.r ) * rcpWeightR ),
	saturate( ( b.g * wG + d.g * wG + f.g * wG + h.g * wG + e.g ) * rcpWeightG ),
	saturate( ( b.b * wB + d.b * wB + f.b * wB + h.b * wB + e.b ) * rcpWeightB ) );
	color.rgb = outColor;
}
void main()
{
	vec2 tCoords = vofi_TexCoord0;
	vec4 color = texture( samp0, tCoords );
	#if USE_CAS
	ContrastAdaptiveSharpeningPass( color );
	#endif 
	#if USE_CHROMATIC_ABERRATION
	ChromaticAberrationPass( color );
	#endif 
	#if USE_TECHNICOLOR
	TechnicolorPass( color );
	#endif 
	#if USE_VIBRANCE
	VibrancePass( color );
	#endif 
	#if USE_DITHERING
	DitheringPass( color );
	#endif 
	fo_FragColor = color;
}