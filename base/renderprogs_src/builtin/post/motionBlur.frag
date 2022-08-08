
#if defined(USE_GPU_SKINNING)
layout( binding = 2 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpMVPmatrixX;
	vec4 rpMVPmatrixY;
	vec4 rpMVPmatrixZ;
	vec4 rpMVPmatrixW;
	vec4 rpProjectionMatrixZ;
	vec4 rpOverbright;
	vec4 rpJitterTexOffset;
};
#else
layout( binding = 1 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpMVPmatrixX;
	vec4 rpMVPmatrixY;
	vec4 rpMVPmatrixZ;
	vec4 rpMVPmatrixW;
	vec4 rpProjectionMatrixZ;
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

layout( location = 0 ) in vec2 vofi_TexCoord0;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	#if 0
	if( vofi_TexCoord0.x < 0.5 )
	{
		discard;
	}
	#endif 
	if( texture( samp0, vofi_TexCoord0 ).w == 0.0 )
	{
		discard;
	}
	float windowZ = texture( samp1, vofi_TexCoord0 ).x;
	vec3 ndc = vec3( vofi_TexCoord0 * 2.0 - 1.0, windowZ * 2.0 - 1.0 );
	float clipW = -rpProjectionMatrixZ.w / ( -rpProjectionMatrixZ.z - ndc.z );
	vec4 clip = vec4( ndc * clipW, clipW );
	vec4 reClip;
	reClip.x = dot( rpMVPmatrixX, clip );
	reClip.y = dot( rpMVPmatrixY, clip );
	reClip.z = dot( rpMVPmatrixZ, clip );
	reClip.w = dot( rpMVPmatrixW, clip );
	vec2 prevTexCoord;
	prevTexCoord.x = ( reClip.x / reClip.w ) * 0.5 + 0.5;
	prevTexCoord.y = ( reClip.y / reClip.w ) * 0.5 + 0.5;
	vec2 texCoord = prevTexCoord;
	vec2 delta = ( vofi_TexCoord0 - prevTexCoord );
	vec3 sum = vec3( 0.0 );
	float goodSamples = 0.0;
	float samples = rpOverbright.x;
	for( float i = 0.0 ; i < samples ; i = i + 1.0 )
	{
		vec2 pos = vofi_TexCoord0 + delta * ( ( i / ( samples - 1.0 ) ) - 0.5 );
		vec4 color = texture( samp0, pos );
		sum += color.xyz * color.w;
		goodSamples += color.w;
	}
	float invScale = 1.0 / goodSamples;
	fo_FragColor = vec4( sum * invScale, 1.0 );
}