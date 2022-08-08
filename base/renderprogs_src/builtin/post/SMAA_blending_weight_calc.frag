
#if defined(USE_GPU_SKINNING)
layout( binding = 2 ) uniform UBOV {
	vec4 rpScreenCorrectionFactor;
	vec4 rpWindowCoord;
	vec4 rpJitterTexOffset;
};
#else
layout( binding = 1 ) uniform UBOV {
	vec4 rpScreenCorrectionFactor;
	vec4 rpWindowCoord;
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
#define SMAA_INCLUDE_VS 0
#define SMAA_INCLUDE_PS 1
#include "renderprogs_src/builtin/post/SMAA.glsl"

layout( location = 0 ) in vec2 vofi_TexCoord0;
layout( location = 1 ) in vec4 vofi_TexCoord1;
layout( location = 2 ) in vec4 vofi_TexCoord2;
layout( location = 3 ) in vec4 vofi_TexCoord3;
layout( location = 4 ) in vec4 vofi_TexCoord4;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	vec2 texcoord = vofi_TexCoord0;
	vec4 offset[3];
	offset[0] = vofi_TexCoord1;
	offset[1] = vofi_TexCoord2;
	offset[2] = vofi_TexCoord3;
	vec2 pixcoord = vofi_TexCoord4.st;
	vec4 subsampleIndices = vec4( 0.0, 0.0, 0.0, 0.0 );
	vec4 color = SMAABlendingWeightCalculationPS( texcoord,
	pixcoord,
	offset,
	samp0,
	samp1,
	samp2,
	subsampleIndices );
	fo_FragColor = color;
}