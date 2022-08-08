
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
layout( binding = 3 ) uniform sampler2DArray samp0;
#else
layout( binding = 2 ) uniform sampler2DArray samp0;
#endif // else

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec2 vofi_TexCoord0;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	vec3 tc;
	tc.xy = vofi_TexCoord0.xy;
	tc.z = rpScreenCorrectionFactor.x;
	fo_FragColor = texture( samp0, tc );
}