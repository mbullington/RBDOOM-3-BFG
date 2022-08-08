layout( binding = 1 ) uniform sampler2D samp0;

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec2 vofi_TexCoord0;
layout( location = 1 ) in vec4 vofi_TexCoord1;
layout( location = 2 ) in vec4 vofi_Color;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	vec4 color = ( texture( samp0, vofi_TexCoord0 ) * vofi_Color ) + vofi_TexCoord1;
	fo_FragColor.xyz = color.xyz * color.w;
	fo_FragColor.w = color.w;
}