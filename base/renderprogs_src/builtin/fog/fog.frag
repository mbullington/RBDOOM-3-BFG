
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
#else
layout( binding = 2 ) uniform sampler2D samp0;
layout( binding = 3 ) uniform sampler2D samp1;
#endif // else

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec2 vofi_TexCoord0;
layout( location = 1 ) in vec2 vofi_TexCoord1;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	vec4 c = texture( samp0, vofi_TexCoord0 ) * texture( samp1, vofi_TexCoord1 ) * rpColor;
	#if defined( USE_LINEAR_RGB )
	c = clamp( c, 0.0, 1.0 );
	c = vec4( Linear1( c.r ), Linear1( c.g ), Linear1( c.b ), Linear1( c.a ) );
	#endif 
	fo_FragColor = c;
}