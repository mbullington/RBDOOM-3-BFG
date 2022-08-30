
float dot2( vec2 a, vec2 b ) { return dot( a, b ); }
float dot3( vec3 a, vec3 b ) { return dot( a, b ); }
float dot3( vec3 a, vec4 b ) { return dot( a, b.xyz ); }
float dot3( vec4 a, vec3 b ) { return dot( a.xyz, b ); }
float dot3( vec4 a, vec4 b ) { return dot( a.xyz, b.xyz ); }
float dot4( vec4 a, vec4 b ) { return dot( a, b ); }
float dot4( vec2 a, vec4 b ) { return dot( vec4( a, 0, 1 ), b ); }
#ifndef PI
#define PI 3.14159265358979323846
#endif 
#define DEG2RAD( a )    ( ( a ) * PI / 180.0f )
#define RAD2DEG( a )    ( ( a ) * 180.0f / PI )
float Linear1( float c )
{
	return ( c <= 0.04045 ) ? c / 12.92 : pow( ( c + 0.055 ) / 1.055, 2.4 );
}
vec3 Linear3( vec3 c )
{
	return vec3( Linear1( c.r ), Linear1( c.g ), Linear1( c.b ) );
}
float Srgb1( float c )
{
	return ( c < 0.0031308 ? c * 12.92 : 1.055 * pow( c, 0.41666 ) - 0.055 );
}
vec3 Srgb3( vec3 c )
{
	return vec3( Srgb1( c.r ), Srgb1( c.g ), Srgb1( c.b ) );
}
const vec3 photoLuma = vec3( 0.2126, 0.7152, 0.0722 );
float PhotoLuma( vec3 c )
{
	return dot( c, photoLuma );
}
vec3 sRGBToLinearRGB( vec3 c )
{
	#if !defined( USE_SRGB )
	c = clamp( c, 0.0, 1.0 );
	return Linear3( c );
	#else 
	return c;
	#endif 
}
vec4 sRGBAToLinearRGBA( vec4 c )
{
	#if !defined( USE_SRGB )
	c = clamp( c, 0.0, 1.0 );
	return vec4( Linear1( c.r ), Linear1( c.g ), Linear1( c.b ), Linear1( c.a ) );
	#else 
	return c;
	#endif 
}
vec3 LinearRGBToSRGB( vec3 c )
{
	#if !defined( USE_SRGB )
	c = clamp( c, 0.0, 1.0 );
	return Srgb3( c );
	#else 
	return c;
	#endif 
}
vec4 LinearRGBToSRGB( vec4 c )
{
	#if !defined( USE_SRGB )
	c = clamp( c, 0.0, 1.0 );
	return vec4( Srgb1( c.r ), Srgb1( c.g ), Srgb1( c.b ), c.a );
	#else 
	return c;
	#endif 
}
float signNotZeroFloat( float k )
{
	return ( k >= 0.0 ) ? 1.0 : -1.0;
}
vec2 signNotZero( vec2 v )
{
	return vec2( signNotZeroFloat( v.x ), signNotZeroFloat( v.y ) );
}
vec2 octEncode( vec3 v )
{
	float l1norm = abs( v.x ) + abs( v.y ) + abs( v.z );
	vec2 oct = v.xy * ( 1.0 / l1norm );
	if( v.z < 0.0 )
	{
		oct = ( 1.0 - abs( oct.yx ) ) * signNotZero( oct.xy );
	}
	return oct;
}
vec3 octDecode( vec2 o )
{
	vec3 v = vec3( o.x, o.y, 1.0 - abs( o.x ) - abs( o.y ) );
	if( v.z < 0.0 )
	{
		v.xy = ( 1.0 - abs( v.yx ) ) * signNotZero( v.xy );
	}
	return normalize( v );
}
const vec4 matrixRGB1toCoCg1YX = vec4( 0.50, 0.0, -0.50, 0.50196078 );
const vec4 matrixRGB1toCoCg1YY = vec4( -0.25, 0.5, -0.25, 0.50196078 );
const vec4 matrixRGB1toCoCg1YZ = vec4( 0.0, 0.0, 0.0, 1.0 );
const vec4 matrixRGB1toCoCg1YW = vec4( 0.25, 0.5, 0.25, 0.0 );
const vec4 matrixCoCg1YtoRGB1X = vec4( 1.0, -1.0, 0.0, 1.0 );
const vec4 matrixCoCg1YtoRGB1Y = vec4( 0.0, 1.0, -0.50196078, 1.0 );
const vec4 matrixCoCg1YtoRGB1Z = vec4( -1.0, -1.0, 1.00392156, 1.0 );
vec3 ConvertYCoCgToRGB( vec4 YCoCg )
{
	vec3 rgbColor;
	YCoCg.z = ( YCoCg.z * 31.875 ) + 1.0;
	YCoCg.z = 1.0 / YCoCg.z;
	YCoCg.xy *= YCoCg.z;
	rgbColor.x = dot4( YCoCg, matrixCoCg1YtoRGB1X );
	rgbColor.y = dot4( YCoCg, matrixCoCg1YtoRGB1Y );
	rgbColor.z = dot4( YCoCg, matrixCoCg1YtoRGB1Z );
	return rgbColor;
}
vec2 CenterScale( vec2 inTC, vec2 centerScale )
{
	float scaleX = centerScale.x;
	float scaleY = centerScale.y;
	vec4 tc0 = vec4( scaleX, 0, 0, 0.5 - ( 0.5 * scaleX ) );
	vec4 tc1 = vec4( 0, scaleY, 0, 0.5 - ( 0.5 * scaleY ) );
	vec2 finalTC;
	finalTC.x = dot4( inTC, tc0 );
	finalTC.y = dot4( inTC, tc1 );
	return finalTC;
}
vec2 Rotate2D( vec2 inTC, vec2 cs )
{
	float sinValue = cs.y;
	float cosValue = cs.x;
	vec4 tc0 = vec4( cosValue, -sinValue, 0, ( -0.5 * cosValue ) + ( 0.5 * sinValue ) + 0.5 );
	vec4 tc1 = vec4( sinValue, cosValue, 0, ( -0.5 * sinValue ) + ( -0.5 * cosValue ) + 0.5 );
	vec2 finalTC;
	finalTC.x = dot4( inTC, tc0 );
	finalTC.y = dot4( inTC, tc1 );
	return finalTC;
}
float rand( vec2 co )
{
	return fract( sin( dot( co.xy, vec2( 12.9898, 78.233 ) ) ) * 43758.5453 );
}
#define square( x )  ( x * x )
const vec4 LUMINANCE_SRGB = vec4( 0.2125, 0.7154, 0.0721, 0.0 );
const vec4 LUMINANCE_LINEAR = vec4( 0.299, 0.587, 0.144, 0.0 );
#define _half2( x )  vec2( x )
#define _half3( x )  vec3( x )
#define _half4( x )  vec4( x )
#define _float2( x ) vec2( x )
#define _float3( x ) vec3( x )
#define _float4( x ) vec4( x )
vec4 idtex2Dproj( sampler2D samp, vec4 texCoords )
{
	return textureProj( samp, texCoords.xyw );
}
vec4 swizzleColor( vec4 c )
{
	return c;
}
#define BRANCH
#define IFANY
float RemapNoiseTriErp( const float v )
{
	float r2 = 0.5 * v;
	float f1 = sqrt( r2 );
	float f2 = 1.0 - sqrt( r2 - 0.25 );
	return ( v < 0.5 ) ? f1 : f2;
}
float InterleavedGradientNoise( vec2 uv )
{
	const vec3 magic = vec3( 0.06711056, 0.00583715, 52.9829189 );
	float rnd = fract( magic.z * fract( dot( uv, magic.xy ) ) );
	return rnd;
}
float InterleavedGradientNoiseAnim( vec2 uv, float frameIndex )
{
	uv += ( frameIndex * 5.588238 );
	const vec3 magic = vec3( 0.06711056, 0.00583715, 52.9829189 );
	float rnd = fract( magic.z * fract( dot( uv, magic.xy ) ) );
	return rnd;
}
const float c_goldenRatioConjugate = 0.61803398875;
#define HASHSCALE3 vec3(443.897, 441.423, 437.195)
vec3 Hash33( vec3 p3 )
{
	p3 = fract( p3 * HASHSCALE3 );
	p3 += dot( p3, p3.yxz + 19.19 );
	return fract( ( p3.xxy + p3.yxx ) * p3.zyx );
}
