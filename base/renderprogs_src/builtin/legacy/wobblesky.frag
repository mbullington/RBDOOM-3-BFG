layout( binding = 1 ) uniform samplerCube samp0;

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec3 vofi_TexCoord0;
layout( location = 1 ) in vec4 vofi_Color;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	fo_FragColor = sRGBAToLinearRGBA( texture( samp0, vofi_TexCoord0 ) ) * vofi_Color;
}