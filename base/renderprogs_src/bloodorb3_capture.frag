#include "renderprogs_src/global.glsl"

layout( binding = 1 ) uniform sampler2D samp0;
layout( binding = 2 ) uniform sampler2D samp1;
layout( binding = 3 ) uniform sampler2D samp2;

layout( location = 0 ) in vec2 vofi_TexCoord0;
layout( location = 1 ) in vec2 vofi_TexCoord1;
layout( location = 2 ) in vec2 vofi_TexCoord2;
layout( location = 3 ) in vec2 vofi_TexCoord3;
layout( location = 4 ) in vec2 vofi_TexCoord4;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	float colorFactor = vofi_TexCoord4.x;
	vec4 color0 = vec4( 1.0 - colorFactor, 1.0 - colorFactor, 1.0, 1.0 );
	vec4 color1 = vec4( 1.0, 0.95 - colorFactor, 0.95, 0.5 );
	vec4 color2 = vec4( 0.015, 0.015, 0.015, 0.01 );
	vec4 accumSample0 = texture( samp0, vofi_TexCoord0 ) * color0;
	vec4 accumSample1 = texture( samp0, vofi_TexCoord1 ) * color1;
	vec4 accumSample2 = texture( samp0, vofi_TexCoord2 ) * color2;
	vec4 maskSample = texture( samp2, vofi_TexCoord3 );
	vec4 tint = vec4( 0.8, 0.5, 0.5, 1 );
	vec4 currentRenderSample = texture( samp1, vofi_TexCoord3 ) * tint;
	vec4 accumColor = mix( accumSample0, accumSample1, 0.5 );
	accumColor += accumSample2;
	accumColor = mix( accumColor, currentRenderSample, maskSample.a );
	fo_FragColor = accumColor;
}