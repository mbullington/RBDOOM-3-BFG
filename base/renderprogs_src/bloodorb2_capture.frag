layout( binding = 1 ) uniform sampler2D samp0;
layout( binding = 2 ) uniform sampler2D samp1;
layout( binding = 3 ) uniform sampler2D samp2;

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec2 vofi_TexCoord0;
layout( location = 1 ) in vec2 vofi_TexCoord1;
layout( location = 2 ) in vec2 vofi_TexCoord2;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	vec4 redTint = vec4( 1, 0.98, 0.98, 1 );
	vec4 accumSample = texture( samp0, vofi_TexCoord0 ) * redTint;
	vec4 maskSample = texture( samp2, vofi_TexCoord1 );
	vec4 tint = vec4( 1.0, 0.8, 0.8, 1 );
	vec4 currentRenderSample = texture( samp1, vofi_TexCoord2 ) * tint;
	fo_FragColor = mix( accumSample, currentRenderSample, maskSample.a );
}