layout( binding = 1 ) uniform sampler2D samp0;

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec2 vofi_TexCoord0;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	vec2 tCoords = vofi_TexCoord0;
	vec4 color = texture( samp0, tCoords );
	fo_FragColor = color;
}