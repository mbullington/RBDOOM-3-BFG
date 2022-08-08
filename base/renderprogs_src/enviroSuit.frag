#include "renderprogs_src/global.glsl"

layout( binding = 1 ) uniform sampler2D samp0;
layout( binding = 2 ) uniform sampler2D samp1;

layout( location = 0 ) in vec2 vofi_TexCoord0;
layout( location = 1 ) in vec4 vofi_Color;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	vec2 screenTexCoord = vofi_TexCoord0;
	vec4 warpFactor = 1.0 - ( texture( samp1, screenTexCoord.xy ) * vofi_Color );
	screenTexCoord -= vec2( 0.5, 0.5 );
	screenTexCoord *= warpFactor.xy;
	screenTexCoord += vec2( 0.5, 0.5 );
	fo_FragColor = texture( samp0, screenTexCoord );
}