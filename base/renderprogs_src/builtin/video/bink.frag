#pragma static USE_SRGB _sRGB

#if defined(USE_GPU_SKINNING)
layout( binding = 2 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpColor;
	vec4 rpJitterTexOffset;
};
#else
layout( binding = 1 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpColor;
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

layout( location = 0 ) in vec2 vofi_TexCoord0;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	vec3 crc = vec3( 1.595794678, -0.813476563, 0 );
	vec3 crb = vec3( 0, -0.391448975, 2.017822266 );
	vec3 adj = vec3( -0.87065506, 0.529705048, -1.081668854 );
	vec3 YScalar = vec3( 1.164123535, 1.164123535, 1.164123535 );
	float Y = texture( samp0, vofi_TexCoord0.xy ).x;
	float Cr = texture( samp1, vofi_TexCoord0.xy ).x;
	float Cb = texture( samp2, vofi_TexCoord0.xy ).x;
	vec3 p = ( YScalar * Y );
	p += ( crc * Cr ) + ( crb * Cb ) + adj;
	vec4 color;
	color.xyz = p;
	color.w = 1.0;
	color *= rpColor;
	fo_FragColor = sRGBAToLinearRGBA( color );
}