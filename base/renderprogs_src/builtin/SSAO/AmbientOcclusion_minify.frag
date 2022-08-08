#pragma static BRIGHTPASS _mip0

#if defined(USE_GPU_SKINNING)
layout( binding = 2 ) uniform UBOV {
	vec4 rpScreenCorrectionFactor;
	vec4 rpWindowCoord;
	vec4 rpJitterTexScale;
	vec4 rpJitterTexOffset;
};
#else
layout( binding = 1 ) uniform UBOV {
	vec4 rpScreenCorrectionFactor;
	vec4 rpWindowCoord;
	vec4 rpJitterTexScale;
	vec4 rpJitterTexOffset;
};
#endif
#if defined(USE_GPU_SKINNING)
layout( binding = 3 ) uniform sampler2D samp0;
#else
layout( binding = 2 ) uniform sampler2D samp0;
#endif // else

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec2 vofi_TexCoord0;

layout( location = 0 ) out vec4 fo_FragColor;

#if 0 //( USE_PEELED_DEPTH_BUFFER != 0 )
#define mask rg
#else 
#define mask r
#endif 
float reconstructCSZ( float d )
{
	return -3.0 / ( -1.0 * d + 1.0 );
}
void main()
{
	#if defined(BRIGHTPASS)
	vec2 ssC = vofi_TexCoord0;
	float depth = texture( samp0, ssC ).r;
	fo_FragColor.mask = depth;
	#else 
	ivec2 ssP = ivec2( vofi_TexCoord0 * rpScreenCorrectionFactor.zw );
	int previousMIPNumber = int( rpJitterTexScale.x );
	fo_FragColor.mask = texelFetch( samp0, clamp( ssP * 2 + ivec2( ssP.y & 1, ssP.x & 1 ), ivec2( 0 ), textureSize( samp0, previousMIPNumber ) - ivec2( 1 ) ), previousMIPNumber ).mask;
	#endif 
}