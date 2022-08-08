
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
layout( binding = 6 ) uniform sampler2D peeledColorBuffer;
layout( binding = 7 ) uniform sampler2D peeledNormalBuffer;
#else
layout( binding = 2 ) uniform sampler2D samp0;
layout( binding = 3 ) uniform sampler2D samp1;
layout( binding = 4 ) uniform sampler2D samp2;
layout( binding = 5 ) uniform sampler2D peeledColorBuffer;
layout( binding = 6 ) uniform sampler2D peeledNormalBuffer;
#endif // else

#include "renderprogs_src/global.glsl"
#define DIFFERENT_DEPTH_RESOLUTIONS 0
#define CS_Z_PACKED_TOGETHER 0
#define TEMPORALLY_VARY_TAPS 0
#define USE_OCT16 0
#define COMPUTE_PEELED_LAYER 0
#define USE_MIPMAPS 1
#define USE_TAP_NORMAL 1
#define HIGH_QUALITY 0
#if HIGH_QUALITY
#define NUM_SAMPLES 39
#define NUM_SPIRAL_TURNS 14
#else 
#define NUM_SAMPLES 11
#define NUM_SPIRAL_TURNS 7
#endif 
#define LOG_MAX_OFFSET (3)
#define MAX_MIP_LEVEL (5)
#define MIN_MIP_LEVEL 0
const float DOOM_TO_METERS = 0.0254;
const float METERS_TO_DOOM = ( 1.0 / DOOM_TO_METERS );
const float FAR_PLANE_Z = -4000.0;
const float radius = 1.0 * METERS_TO_DOOM;
const float radius2 = radius * radius;
const float invRadius2 = 1.0 / radius2;
const float bias = 0.01 * METERS_TO_DOOM;
const float projScale = 500.0;
#define normal_buffer samp0
#define CS_Z_buffer  samp1
#define colorBuffer  samp2

layout( location = 0 ) in vec2 vofi_TexCoord0;

layout( location = 0 ) out vec4 fo_FragColor;

#define USE_DEPTH_PEEL 0
#define indirectColor   fo_FragColor.rgb
#define visibility      fo_FragColor.a
float reconstructCSZ( float d )
{
	return -3.0 / ( -1.0 * d + 1.0 );
}
vec3 reconstructCSPosition( vec2 S, float z )
{
	vec4 P;
	P.z = z * 2.0 - 1.0;
	P.xy = ( S * rpScreenCorrectionFactor.xy ) * 2.0 - 1.0;
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
#if USE_OCT16
void sampleBothNormals( sampler2D normalBuffer, ivec2 ssC, int mipLevel, out vec3 n_tap0, out vec3 n_tap1 )
{
	vec4 encodedNormals = texelFetch( normalBuffer, ssC, mipLevel ) * 2.0 - 1.0;
	n_tap0 = decode16( encodedNormals.xy );
	n_tap1 = decode16( encodedNormals.zw );
}
#endif
vec2 tapLocation( int sampleNumber, float spinAngle, float radialJitter, out float ssR )
{
	float alpha = ( float( sampleNumber ) + radialJitter ) * ( 1.0 / float( NUM_SAMPLES ) );
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
void getPositions( ivec2 ssP, sampler2D cszBuffer, out vec3 P0, out vec3 P1 )
{
	vec2 Zs = texelFetch( cszBuffer, ssP, 0 ).rg;
	P0 = reconstructCSPosition( vec2( ssP ) + vec2( 0.5 ), Zs.x );
	P1 = reconstructCSPosition( vec2( ssP ) + vec2( 0.5 ), Zs.y );
}
void computeMipInfo( float ssR, ivec2 ssP, sampler2D cszBuffer, inout int mipLevel, inout ivec2 mipP )
{
	#if USE_MIPMAPS
	#ifdef GL_EXT_gpu_shader5
	mipLevel = clamp( findMSB( int( ssR ) ) - LOG_MAX_OFFSET, MIN_MIP_LEVEL, MAX_MIP_LEVEL );
	#else 
	mipLevel = clamp( int( floor( log2( ssR ) ) ) - LOG_MAX_OFFSET, MIN_MIP_LEVEL, MAX_MIP_LEVEL );
	#endif 
	mipP = clamp( ssP >> mipLevel, ivec2( 0 ), textureSize( cszBuffer, mipLevel ) - ivec2( 1 ) );
	#else 
	mipLevel = 0;
	mipP = ssP;
	#endif 
}
// vec3 getOffsetPosition( ivec2 ssC, vec2 unitOffset, float ssR, sampler2D cszBuffer, float invCszBufferScale )
// {
// 	ivec2 ssP = clamp( ivec2( ssR * unitOffset ) + ssC, ivec2( 0 ), ivec2( g3d_sz2D_colorBuffer.xy - 1 ) );
// 	int mipLevel;
// 	ivec2 mipP;
// 	computeMipInfo( ssR, ssP, cszBuffer, mipLevel, mipP );
// 	vec3 P;
// 	P.z = texelFetch( cszBuffer, mipP, mipLevel ).r;
// 	P = reconstructCSPosition( ( vec2( ssP ) + vec2( 0.5 ) ) * invCszBufferScale, P.z );
// 	return P;
// }
// void getOffsetPositions( ivec2 ssC, vec2 unitOffset, float ssR, sampler2D cszBuffer, out vec3 P0, out vec3 P1 )
// {
// 	ivec2 ssP = clamp( ivec2( ssR * unitOffset ) + ssC, ivec2( 0 ), ivec2( g3d_sz2D_colorBuffer.xy - 1 ) );
// 	int mipLevel;
// 	ivec2 mipP;
// 	computeMipInfo( ssR, ssP, cszBuffer, mipLevel, mipP );
// 	vec2 Zs = texelFetch( cszBuffer, mipP, mipLevel ).rg;
// 	P0 = reconstructCSPosition( ( vec2( ssP ) + vec2( 0.5 ) ), Zs.x );
// 	P1 = reconstructCSPosition( ( vec2( ssP ) + vec2( 0.5 ) ), Zs.y );
// }
void getOffsetPositionNormalAndLambertian
( ivec2 ssP,
float ssR,
sampler2D cszBuffer,
sampler2D bounceBuffer,
sampler2D normalBuffer,
inout vec3 Q,
inout vec3 lambertian_tap,
inout vec3 n_tap )
{
	#if USE_MIPMAPS
	int mipLevel;
	ivec2 texel;
	computeMipInfo( ssR, ssP, cszBuffer, mipLevel, texel );
	#else 
	int mipLevel = 0;
	ivec2 texel = ssP;
	#endif 
	float z = texelFetch( cszBuffer, texel, mipLevel ).r;
	#if 0
	vec3 n = sampleNormal( normalBuffer, texel, mipLevel );
	lambertian_tap = texelFetch( bounceBuffer, texel, mipLevel ).rgb;
	#else 
	vec3 n = sampleNormal( normalBuffer, ssP, 0 );
	lambertian_tap = texelFetch( bounceBuffer, ssP, 0 ).rgb;
	#endif 
	n_tap = n;
	Q = reconstructCSPosition( ( vec2( ssP ) + vec2( 0.5 ) ), z );
}
void getOffsetPositionsNormalsAndLambertians
( ivec2 ssP,
float ssR,
sampler2D cszBuffer,
sampler2D bounceBuffer,
sampler2D peeledBounceBuffer,
sampler2D normalBuffer,
sampler2D peeledNormalBuffer,
out vec3 Q0,
out vec3 Q1,
out vec3 lambertian_tap0,
out vec3 lambertian_tap1,
out vec3 n_tap0,
out vec3 n_tap1 )
{
	#if USE_MIPMAPS
	int mipLevel;
	ivec2 texel;
	computeMipInfo( ssR, ssP, cszBuffer, mipLevel, texel );
	#else 
	int mipLevel = 0;
	ivec2 texel = ssP;
	#endif 
	vec2 Zs = texelFetch( cszBuffer, texel, mipLevel ).rg;
	#if USE_OCT16
	sampleBothNormals( normalBuffer, texel, mipLevel, n_tap0, n_tap1 );
	#else 
	n_tap0 = sampleNormal( normalBuffer, texel, mipLevel );
	n_tap1 = sampleNormal( peeledNormalBuffer, texel, mipLevel );
	#endif 
	lambertian_tap0 = texelFetch( bounceBuffer, texel, mipLevel ).rgb;
	lambertian_tap1 = texelFetch( peeledBounceBuffer, texel, mipLevel ).rgb;
	Q0 = reconstructCSPosition( ( vec2( ssP ) + vec2( 0.5 ) ), Zs.x ); // had projInfo
	Q1 = reconstructCSPosition( ( vec2( ssP ) + vec2( 0.5 ) ), Zs.y ); // had projInfo
}
void iiValueFromPositionsAndNormalsAndLambertian( ivec2 ssP, vec3 X, vec3 n_X, vec3 Y, vec3 n_Y, vec3 radiosity_Y, inout vec3 E, inout float weight_Y, inout float visibilityWeight_Y )
{
	vec3 YminusX = Y - X;
	vec3 w_i = normalize( YminusX );
	weight_Y = ( ( dot( w_i, n_X ) > 0.0 )
	#if USE_TAP_NORMAL
	&& ( dot( -w_i, n_Y ) > 0.01 )
	#endif 
	) ? 1.0 : 0.0;
	if( ( dot( YminusX, YminusX ) < radius2 ) &&
	( weight_Y > 0.0 ) )
	{
		E = radiosity_Y * dot( w_i, n_X );
	}
	else
	{
		#if USE_TAP_NORMAL == 0
		weight_Y = 0;
		#endif 
		E = vec3( 0 );
	}
}
void sampleIndirectLight
( in ivec2 ssC,
in vec3 C,
in vec3 n_C,
in vec3 C_peeled,
in vec3 n_C_peeled,
in float ssDiskRadius,
in int tapIndex,
in float randomPatternRotationAngle,
in float radialJitter,
in sampler2D cszBuffer,
in sampler2D nBuffer,
in sampler2D bounceBuffer,
inout vec3 irradianceSum,
inout float numSamplesUsed,
inout vec3 iiPeeled,
inout float weightSumPeeled )
{
	float visibilityWeightPeeled0, visibilityWeightPeeled1;
	float ssR;
	vec2 unitOffset = tapLocation( tapIndex, randomPatternRotationAngle, radialJitter, ssR );
	ssR *= ssDiskRadius;
	ivec2 ssP = ivec2( ssR * unitOffset ) + ssC;
	#if USE_DEPTH_PEEL
	vec3 E, ii_tap0, ii_tap1;
	float weight, weight0, weight1;
	float visibilityWeight0, visibilityWeight1;
	vec3 Q0, lambertian_tap0, n_tap0, Q1, lambertian_tap1, n_tap1;
	getOffsetPositionsNormalsAndLambertians( ssP, ssR, cszBuffer, bounceBuffer, peeledColorBuffer, nBuffer, peeledNormalBuffer, Q0, Q1, lambertian_tap0, lambertian_tap1, n_tap0, n_tap1 );
	iiValueFromPositionsAndNormalsAndLambertian( ssP, C, n_C, Q0, n_tap0, lambertian_tap0, ii_tap0, weight0, visibilityWeight0 );
	float adjustedWeight0 = weight0 * dot( ii_tap0, ii_tap0 ) + weight0;
	iiValueFromPositionsAndNormalsAndLambertian( ssP, C, n_C, Q1, n_tap1, lambertian_tap1, ii_tap1, weight1, visibilityWeight1 );
	float adjustedWeight1 = weight1 * dot( ii_tap1, ii_tap1 ) + weight1;
	weight = ( adjustedWeight0 > adjustedWeight1 ) ? weight0 : weight1;
	E = ( adjustedWeight0 > adjustedWeight1 ) ? ii_tap0 : ii_tap1;
	#if COMPUTE_PEELED_LAYER
	float weightPeeled0, weightPeeled1;
	vec3 ii_tapPeeled0, ii_tapPeeled1;
	iiValueFromPositionsAndNormalsAndLambertian( ssP, C_peeled, n_C_peeled, Q0, n_tap0, lambertian_tap0, ii_tapPeeled0, weightPeeled0, visibilityWeightPeeled0 );
	iiValueFromPositionsAndNormalsAndLambertian( ssP, C_peeled, n_C_peeled, Q1, n_tap1, lambertian_tap1, ii_tapPeeled1, weightPeeled1, visibilityWeightPeeled1 );
	float iiMag0 = dot( ii_tapPeeled0, ii_tapPeeled0 );
	float iiMag1 = dot( ii_tapPeeled1, ii_tapPeeled1 );
	weightSumPeeled += iiMag0 > iiMag1 ? weightPeeled0 : weightPeeled1;
	iiPeeled += iiMag0 > iiMag1 ? ii_tapPeeled0 : ii_tapPeeled1;
	#endif 
	numSamplesUsed += weight;
	#else 
	vec3 E;
	float visibilityWeight;
	float weight_Y;
	vec3 Q, lambertian_tap, n_tap;
	getOffsetPositionNormalAndLambertian( ssP, ssR, cszBuffer, bounceBuffer, nBuffer, Q, lambertian_tap, n_tap );
	iiValueFromPositionsAndNormalsAndLambertian( ssP, C, n_C, Q, n_tap, lambertian_tap, E, weight_Y, visibilityWeight );
	numSamplesUsed += weight_Y;
	#endif 
	irradianceSum += E;
}
void main()
{
	fo_FragColor = vec4( 0.0, 0.0, 0.0, 1.0 );
	#if 0
	if( vofi_TexCoord0.x < 0.5 )
	{
		discard;
	}
	#endif 
	ivec2 ssC = ivec2( gl_FragCoord.xy );
	#if COMPUTE_PEELED_LAYER
	vec3 C, C_peeled;
	getPositions( ssC, CS_Z_buffer, C, C_peeled );
	vec3 n_C_peeled = sampleNormal( peeledNormalBuffer, ssC, 0 );
	#else 
	vec3 C = getPosition( ssC, CS_Z_buffer );
	vec3 C_peeled = vec3( 0 );
	vec3 n_C_peeled = vec3( 0 );
	#endif 
	vec3 n_C = sampleNormal( normal_buffer, ssC, 0 );
	float ssDiskRadius = -projScale * radius / C.z;
	float randomPatternRotationAngle = float( 3 * ssC.x ^ ssC.y + ssC.x * ssC.y ) * 10.0;
	#if TEMPORALLY_VARY_TAPS
	randomPatternRotationAngle += rpJitterTexOffset.x;
	#endif 
	float radialJitter = fract( sin( gl_FragCoord.x * 1e2 +
	#if TEMPORALLY_VARY_TAPS
	rpJitterTexOffset.x +
	#endif 
	gl_FragCoord.y ) * 1e5 + sin( gl_FragCoord.y * 1e3 ) * 1e3 ) * 0.8 + 0.1;
	float numSamplesUsed = 0.0;
	vec3 irradianceSum = vec3( 0 );
	vec3 ii_peeled = vec3( 0 );
	float peeledSum = 0.0;
	for( int i = 0; i < NUM_SAMPLES; ++i )
	{
		sampleIndirectLight( ssC, C, n_C, C_peeled, n_C_peeled, ssDiskRadius, i, randomPatternRotationAngle, radialJitter, CS_Z_buffer, normal_buffer, colorBuffer, irradianceSum, numSamplesUsed, ii_peeled, peeledSum );
	}
	float solidAngleHemisphere = 2.0 * PI;
	vec3 E_X = irradianceSum * solidAngleHemisphere / ( numSamplesUsed + 0.00001 );
	indirectColor = E_X;
	visibility = 1.0 - numSamplesUsed / float( NUM_SAMPLES );
	#if COMPUTE_PEELED_LAYER
	float A_peeled = 1.0 - peeledSum / float( NUM_SAMPLES );
	vec3 E_X_peeled = ii_peeled * solidAngleHemisphere / ( peeledSum + 0.00001 );
	indirectPeeledResult = E_X_peeled;
	peeledVisibility = A_peeled;
	#endif 
}