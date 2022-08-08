
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
#else
layout( binding = 2 ) uniform sampler2D samp0;
layout( binding = 3 ) uniform sampler2D samp1;
#endif // else

#include "renderprogs_src/global.glsl"
#define SMAA_INCLUDE_VS 0
#define SMAA_INCLUDE_PS 1
#include "renderprogs_src/builtin/post/SMAA.glsl"

layout( location = 0 ) in vec2 vofi_TexCoord0;
layout( location = 1 ) in vec4 vofi_TexCoord1;
layout( location = 2 ) in vec4 vofi_TexCoord2;
layout( location = 3 ) in vec4 vofi_TexCoord3;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	vec2 tCoords = vofi_TexCoord0;
	vec4 offset[3];
	offset[0] = vofi_TexCoord1;
	offset[1] = vofi_TexCoord2;
	offset[2] = vofi_TexCoord3;
	vec4 color = vec4( 0.0 );
	color.rg = SMAALumaEdgeDetectionPS( tCoords,
	offset,
	samp0
	#if SMAA_PREDICATION
	, samp1
	#endif 
	);
	fo_FragColor = color;
}