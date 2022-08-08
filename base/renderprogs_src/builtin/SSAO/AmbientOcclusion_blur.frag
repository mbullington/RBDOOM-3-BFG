#pragma static BRIGHTPASS _write

#if defined(USE_GPU_SKINNING)
layout( binding = 2 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpModelMatrixX;
	vec4 rpModelMatrixY;
	vec4 rpModelMatrixZ;
	vec4 rpModelMatrixW;
	vec4 rpJitterTexScale;
	vec4 rpJitterTexOffset;
};
#else
layout( binding = 1 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpModelMatrixX;
	vec4 rpModelMatrixY;
	vec4 rpModelMatrixZ;
	vec4 rpModelMatrixW;
	vec4 rpJitterTexScale;
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
#define normal_buffer samp0
#define cszBuffer  samp1
#define source   samp2

layout( location = 0 ) in vec2 vofi_TexCoord0;

layout( location = 0 ) out vec4 fo_FragColor;

#define PEELED_LAYER 0
#define USE_OCT16 0
#define USE_NORMALS 1
#define EDGE_SHARPNESS     (1.0)
#define SCALE               (2)
#define R                   (4)
#define MDB_WEIGHTS  0
#define VALUE_TYPE        float
#define VALUE_COMPONENTS   r
#define VALUE_IS_KEY       0
#if USE_OCT16
#include oct.glsl>
#endif 
vec3 sampleNormal( sampler2D normalBuffer, ivec2 ssC, int mipLevel )
{
	#if USE_OCT16
	return decode16( texelFetch( normalBuffer, ssC, mipLevel ).xy * 2.0 - 1.0 );
	#else 
	return normalize( texelFetch( normalBuffer, ssC, mipLevel ).xyz * 2.0 - 1.0 );
	#endif 
}
#define aoResult       fo_FragColor.VALUE_COMPONENTS
#define keyPassThrough fo_FragColor.KEY_COMPONENTS
const float FAR_PLANE_Z = -16000.0;
float CSZToKey( float z )
{
	return clamp( z * ( 1.0 / FAR_PLANE_Z ), 0.0, 1.0 );
}
float reconstructCSZ( float d )
{
	return -3.0 / ( -1.0 * d + 1.0 );
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
float getKey( ivec2 ssP )
{
	#if PEELED_LAYER
	float key = texelFetch( cszBuffer, ssP, 0 ).g;
	#else 
	float key = texelFetch( cszBuffer, ssP, 0 ).r;
	#endif 
	#if 0
	key = reconstructCSZ( key );
	#else 
	vec3 P = reconstructCSPosition( vec2( ssP ) + vec2( 0.5 ), key );
	key = P.z;
	#endif 
	key = clamp( key * ( 1.0 / FAR_PLANE_Z ), 0.0, 1.0 );
	return key;
}
vec3 positionFromKey( float key, ivec2 ssC )
{
	float z = key * FAR_PLANE_Z;
	vec3 C = reconstructCSPosition( vec2( ssC ) + vec2( 0.5 ), z );
	return C;
}
vec3 getPosition( ivec2 ssP, sampler2D cszBuffer )
{
	vec3 P;
	P.z = texelFetch( cszBuffer, ssP, 0 ).r;
	P = reconstructCSPosition( vec2( ssP ) + vec2( 0.5 ), P.z );
	return P;
}
float calculateBilateralWeight( float key, float tapKey, ivec2 tapLoc, vec3 n_C, vec3 C )
{
	float depthWeight = max( 0.0, 1.0 - ( EDGE_SHARPNESS * 2000.0 ) * abs( tapKey - key ) );
	float k_normal = 1.0;
	float k_plane = 1.0;
	float normalWeight = 1.0;
	float planeWeight = 1.0;
	#if USE_NORMALS
	vec3 tapN_C = sampleNormal( normal_buffer, tapLoc, 0 );
	depthWeight = 1.0;
	float normalError = 1.0 - dot( tapN_C, n_C ) * k_normal;
	normalWeight = max( ( 1.0 - EDGE_SHARPNESS * normalError ), 0.00 );
	float lowDistanceThreshold2 = 0.001;
	vec3 tapC = getPosition( tapLoc, cszBuffer );
	vec3 dq = C - tapC;
	float distance2 = dot( dq, dq );
	float planeError = max( abs( dot( dq, tapN_C ) ), abs( dot( dq, n_C ) ) );
	planeWeight = ( distance2 < lowDistanceThreshold2 ) ? 1.0 :
	pow( max( 0.0, 1.0 - EDGE_SHARPNESS * 2.0 * k_plane * planeError / sqrt( distance2 ) ), 2.0 );
	#endif 
	return depthWeight * normalWeight * planeWeight;
}
void main()
{
	float kernel[R + 1];
	#if R == 1
	kernel[0] = 0.5;
	kernel[1] = 0.25;
	#elif R == 2
	kernel[0] = 0.153170;
	kernel[1] = 0.144893;
	kernel[2] = 0.122649;
	#elif R == 3
	kernel[0] = 0.153170;
	kernel[1] = 0.144893;
	kernel[2] = 0.122649;
	kernel[3] = 0.092902;
	#elif R == 4
	kernel[0] = 0.153170;
	kernel[1] = 0.144893;
	kernel[2] = 0.122649;
	kernel[3] = 0.092902;
	kernel[4] = 0.062970;
	#elif R == 5
	kernel[0] = 0.111220;
	kernel[1] = 0.107798;
	kernel[2] = 0.098151;
	kernel[3] = 0.083953;
	kernel[4] = 0.067458;
	kernel[5] = 0.050920;
	#elif R == 6
	kernel[0] = 0.111220;
	kernel[1] = 0.107798;
	kernel[2] = 0.098151;
	kernel[3] = 0.083953;
	kernel[4] = 0.067458;
	kernel[5] = 0.050920;
	kernel[6] = 0.036108;
	#endif 
	ivec2 ssC = ivec2( gl_FragCoord.xy );
	vec4 temp = texelFetch( source, ssC, 0 );
	#if 0
	if( vofi_TexCoord0.x < 0.75 )
	{
		fo_FragColor = temp;
		return;
	}
	#endif 
	#if 0
	float key = getKey( ssC );
	vec3 C = positionFromKey( key, ssC );
	#else 
	vec3 C = getPosition( ssC, cszBuffer );
	float key = CSZToKey( C.z );
	#endif 
	VALUE_TYPE sum = temp.VALUE_COMPONENTS;
	if( key == 1.0 )
	{
		aoResult = sum;
		#if defined(BRIGHTPASS)
		fo_FragColor = vec4( aoResult, aoResult, aoResult, 1.0 );
		#endif 
		return;
	}
	float BASE = kernel[0];
	float totalWeight = BASE;
	sum *= totalWeight;
	vec3 n_C;
	#if USE_NORMALS
	n_C = sampleNormal( normal_buffer, ssC, 0 );
	#endif 
	#if MDB_WEIGHTS == 0
	for( int r = -R; r <= R; ++r )
	{
		if( r != 0 )
		{
			ivec2 tapLoc = ssC + ivec2( rpJitterTexScale.xy ) * ( r * SCALE );
			temp = texelFetch( source, tapLoc, 0 );
			float tapKey = getKey( tapLoc );
			VALUE_TYPE value = temp.VALUE_COMPONENTS;
			float weight = 0.3 + kernel[abs( r )];
			float bilateralWeight = calculateBilateralWeight( key, tapKey, tapLoc, n_C, C );
			weight *= bilateralWeight;
			sum += value * weight;
			totalWeight += weight;
		}
	}
	#else 
	float lastBilateralWeight = 9999.0;
	for( int r = -1; r >= -R; --r )
	{
		ivec2 tapLoc = ssC + ivec2( rpJitterTexScale.xy ) * ( r * SCALE );
		temp = texelFetch( source, tapLoc, 0 );
		float tapKey = getKey( tapLoc );
		VALUE_TYPE value = temp.VALUE_COMPONENTS;
		float weight = 0.3 + kernel[abs( r )];
		float bilateralWeight = calculateBilateralWeight( key, tapKey, tapLoc, n_C, C );
		bilateralWeight = min( lastBilateralWeight, bilateralWeight );
		lastBilateralWeight = bilateralWeight;
		weight *= bilateralWeight;
		sum += value * weight;
		totalWeight += weight;
	}
	lastBilateralWeight = 9999.0;
	for( int r = 1; r <= R; ++r )
	{
		ivec2 tapLoc = ssC + ivec2( rpJitterTexScale.xy ) * ( r * SCALE );
		temp = texelFetch( source, tapLoc, 0 );
		float tapKey = getKey( tapLoc );
		VALUE_TYPE value = temp.VALUE_COMPONENTS;
		float weight = 0.3 + kernel[abs( r )];
		float bilateralWeight = calculateBilateralWeight( key, tapKey, tapLoc, n_C, C );
		bilateralWeight = min( lastBilateralWeight, bilateralWeight );
		lastBilateralWeight = bilateralWeight;
		weight *= bilateralWeight;
		sum += value * weight;
		totalWeight += weight;
	}
	#endif 
	float epsilon = 0.0001;
	aoResult = sum / ( totalWeight + epsilon );
	#if defined(BRIGHTPASS)
	fo_FragColor = vec4( aoResult, aoResult, aoResult, 1.0 );
	#endif 
}