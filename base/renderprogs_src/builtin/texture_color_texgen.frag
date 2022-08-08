layout( binding = 1 ) uniform sampler2D samp0;

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec4 vofi_TexCoord0;
layout( location = 1 ) in vec4 vofi_Color;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	vec4 texSample = idtex2Dproj( samp0, vofi_TexCoord0 );
	fo_FragColor = sRGBAToLinearRGBA( texSample ) * vofi_Color;
}