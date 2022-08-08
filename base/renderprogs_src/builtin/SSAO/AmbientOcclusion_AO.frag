#pragma static BRIGHTPASS _write

#if defined(USE_GPU_SKINNING)
layout( binding = 2 ) uniform UBOV {
	vec4 rpScreenCorrectionFactor;
	vec4 rpWindowCoord;
	vec4 rpModelMatrixX;
	vec4 rpModelMatrixY;
	vec4 rpModelMatrixZ;
	vec4 rpModelMatrixW;
	vec4 rpJitterTexOffset;
};
#else
layout( binding = 1 ) uniform UBOV {
	vec4 rpScreenCorrectionFactor;
	vec4 rpWindowCoord;
	vec4 rpModelMatrixX;
	vec4 rpModelMatrixY;
	vec4 rpModelMatrixZ;
	vec4 rpModelMatrixW;
	vec4 rpJitterTexOffset;
};
#endif
#if defined(USE_GPU_SKINNING)
layout( binding = 3 ) uniform sampler2D samp0;
layout( binding = 4 ) uniform sampler2D samp1;
layout( binding = 5 ) uniform sampler2D samp2;
#else
layout( binding = 2 ) uniform sampler2D samp0;
layout( binding = 3 ) uniform sampler2D samp1;
layout( binding = 4 ) uniform sampler2D samp2;
#endif // else

#include "renderprogs_src/global.glsl"
#define DIFFERENT_DEPTH_RESOLUTIONS 0
#define CS_Z_PACKED_TOGETHER 0
#define TEMPORALLY_VARY_TAPS 0
#define HIGH_QUALITY 1
#define USE_OCT16 0
#define USE_MIPMAPS 1
#define NUM_SAMPLES 11
#define NUM_SPIRAL_TURNS 7
#define LOG_MAX_OFFSET (3)
#define MAX_MIP_LEVEL (5)
#define MIN_MIP_LEVEL 0
const float DOOM_TO_METERS = 0.0254;
const float METERS_TO_DOOM = ( 1.0 / DOOM_TO_METERS );
const float radius = 1.0 * METERS_TO_DOOM;
const float radius2 = radius * radius;
const float invRadius2 = 1.0 / radius2;
const float bias = 0.01 * METERS_TO_DOOM;
const float intensity = 0.6;
const float intensityDivR6 = intensity / ( radius* radius* radius* radius* radius* radius );
const float projScale = 500.0;
#define CS_Z_buffer samp1

layout( location = 0 ) in vec2 vofi_TexCoord0;

layout( location = 0 ) out vec4 fo_FragColor;

float BlueNoise( vec2 n, float x )
{
	float noise = texture( samp2, n.xy * rpJitterTexOffset.xy ).r;
	#if TEMPORALLY_VARY_TAPS
	noise = fract( noise + 0.61803398875 * rpJitterTexOffset.z * x );
	#else 
	noise = fract( noise );
	#endif 
	return noise;
}
void packKey( float key, out vec2 p )
{
	float temp = floor( key * 256.0 );
	p.x = temp * ( 1.0 / 256.0 );
	p.y = key * 256.0 - temp;
}
vec3 reconstructCSFaceNormal( vec3 C )
{
	return normalize( cross( dFdy( C ), dFdx( C ) ) );
}
vec3 reconstructNonUnitCSFaceNormal( vec3 C )
{
	vec3 n = cross( dFdy( C ), dFdx( C ) );
	return n;
}
vec3 reconstructCSPosition( vec2 S, float z )
{
	vec4 P;
	P.z = z * 2.0 - 1.0;
	P.xy = ( S * rpWindowCoord.xy ) * 2.0 - 1.0;
	P.w = 1.0;
	vec4 csP;
	csP.x = dot4( P, rpModelMatrixX );
	csP.y = dot4( P, rpModelMatrixY );
	csP.z = dot4( P, rpModelMatrixZ );
	csP.w = dot4( P, rpModelMatrixW );
	csP.xyz /= csP.w;
	return csP.xyz;
}
vec3 sampleNormal( sampler2D normalBuffer, ivec2 ssC, int mipLevel )
{
	#if USE_OCT16
	return decode16( texelFetch( normalBuffer, ssC, mipLevel ).xy * 2.0 - 1.0 );
	#else 
	return texelFetch( normalBuffer, ssC, mipLevel ).xyz * 2.0 - 1.0;
	#endif 
}
vec2 tapLocation( int sampleNumber, float spinAngle, out float ssR )
{
	float alpha = ( float( sampleNumber ) + 0.5 ) * ( 1.0 / float( NUM_SAMPLES ) );
	float angle = alpha * ( float( NUM_SPIRAL_TURNS ) * 6.28 ) + spinAngle;
	ssR = alpha;
	return vec2( cos( angle ), sin( angle ) );
}
vec3 getPosition( ivec2 ssP, sampler2D cszBuffer )
{
	vec3 P;
	P.z = texelFetch( cszBuffer, ssP, 0 ).r;
	P = reconstructCSPosition( vec2( ssP ) + vec2( 0.5 ), P.z );
	return P;
}
void computeMipInfo( float ssR, ivec2 ssP, sampler2D cszBuffer, out int mipLevel, out ivec2 mipP )
{
	#ifdef GL_EXT_gpu_shader5
	mipLevel = clamp( findMSB( int( ssR ) ) - LOG_MAX_OFFSET, 0, MAX_MIP_LEVEL );
	#else 
	mipLevel = clamp( int( floor( log2( ssR ) ) ) - LOG_MAX_OFFSET, 0, MAX_MIP_LEVEL );
	#endif 
	mipP = clamp( ssP >> mipLevel, ivec2( 0 ), textureSize( cszBuffer, mipLevel ) - ivec2( 1 ) );
}
vec3 getOffsetPosition( ivec2 issC, vec2 unitOffset, float ssR, sampler2D cszBuffer, float invCszBufferScale )
{
	ivec2 ssP = ivec2( ssR * unitOffset ) + issC;
	vec3 P;
	int mipLevel;
	ivec2 mipP;
	computeMipInfo( ssR, ssP, cszBuffer, mipLevel, mipP );
	#if USE_MIPMAPS
	P.z = texelFetch( cszBuffer, mipP, mipLevel ).r;
	#else 
	P.z = texelFetch( cszBuffer, ssP, 0 ).r;
	#endif 
	P = reconstructCSPosition( vec2( ssP ) + vec2( 0.5 ), P.z );
	return P;
}
float fallOffFunction( float vv, float vn, float epsilon )
{
	#if HIGH_QUALITY
	float f = max( 1.0 - vv * invRadius2, 0.0 );
	return f * max( ( vn - bias ) * inversesqrt( epsilon + vv ), 0.0 );
	#else 
	float f = max( radius2 - vv, 0.0 );
	return f * f * f * max( ( vn - bias ) / ( epsilon + vv ), 0.0 );
	#endif 
}
float aoValueFromPositionsAndNormal( vec3 C, vec3 n_C, vec3 Q )
{
	vec3 v = Q - C;
	float vv = dot( v, v );
	float vn = dot( v, n_C );
	const float epsilon = 0.001;
	return fallOffFunction( vv, vn, epsilon ) * mix( 1.0, max( 0.0, 1.5 * n_C.z ), 0.35 );
}
float sampleAO( ivec2 issC, in vec3 C, in vec3 n_C, in float ssDiskRadius, in int tapIndex, in float randomPatternRotationAngle, in sampler2D cszBuffer, in float invCszBufferScale )
{
	float ssR;
	vec2 unitOffset = tapLocation( tapIndex, randomPatternRotationAngle, ssR );
	ssR = max( 0.75, ssR * ssDiskRadius );
	#if (CS_Z_PACKED_TOGETHER != 0)
	vec3 Q0, Q1;
	getOffsetPositions( ssC, unitOffset, ssR, cszBuffer, Q0, Q1 );
	return max( aoValueFromPositionsAndNormal( C, n_C, Q0 ), aoValueFromPositionsAndNormal( C, n_C, Q1 ) );
	#else 
	vec3 Q = getOffsetPosition( issC, unitOffset, ssR, cszBuffer, invCszBufferScale );
	return aoValueFromPositionsAndNormal( C, n_C, Q );
	#endif 
}
const float MIN_RADIUS = 3.0;
#define visibility      fo_FragColor.r
#define bilateralKey    fo_FragColor.gb
void main()
{
	fo_FragColor = vec4( 1.0, 0.0, 0.0, 1.0 );
	#if 0
	if( vofi_TexCoord0.x < 0.5 )
	{
		discard;
	}
	#endif 
	vec2 ssC = vofi_TexCoord0 * rpScreenCorrectionFactor.xy;
	ivec2 ssP = ivec2( ssC.x * rpWindowCoord.z, ssC.y * rpWindowCoord.w );
	vec3 C = getPosition( ssP, CS_Z_buffer );
	#if 0
	if( key >= 1.0 )
	{
		visibility = 0.0;
		return;
	}
	#endif 
	visibility = 0.0;
	#if 1
	vec3 n_C = sampleNormal( samp0, ssP, 0 );
	if( length( n_C ) < 0.01 )
	{
		visibility = 1.0;
		return;
	}
	n_C = normalize( n_C );
	#else 
	vec3 n_C = reconstructNonUnitCSFaceNormal( C );
	if( dot( n_C, n_C ) > ( square( C.z * C.z * 0.00006 ) ) )
	{
		visibility = 1.0;
		return;
	}
	else
	{
		n_C = normalize( -n_C );
	}
	#endif 
	#if 1
	float randomPatternRotationAngle = BlueNoise( vec2( ssP.xy ), 10.0 ) * 10.0;
	#else 
	float randomPatternRotationAngle = float( ( ( 3 * ssP.x ) ^ ( ssP.y + ssP.x * ssP.y ) )
	#if TEMPORALLY_VARY_TAPS
	+ rpJitterTexOffset.z
	#endif 
	) * 10.0;
	#endif 
	float ssDiskRadius = -projScale * radius / C.z;
	#if 1
	if( ssDiskRadius <= MIN_RADIUS )
	{
		visibility = 1.0;
		return;
	}
	#endif 
	float sum = 0.0;
	for( int i = 0; i < NUM_SAMPLES; ++i )
	{
		sum += sampleAO( ssP, C, n_C, ssDiskRadius, i, randomPatternRotationAngle, CS_Z_buffer, 1.0 );
	}
	#if HIGH_QUALITY
	float A = pow( max( 0.0, 1.0 - sqrt( sum * ( 3.0 / float( NUM_SAMPLES ) ) ) ), intensity );
	#else 
	float A = max( 0.0, 1.0 - sum * intensityDivR6 * ( 5.0 / float( NUM_SAMPLES ) ) );
	#endif 
	visibility = mix( 1.0, A, saturate( ssDiskRadius - MIN_RADIUS ) );
	#if defined(BRIGHTPASS)
	fo_FragColor = vec4( visibility, visibility, visibility, 1.0 );
	#endif 
}