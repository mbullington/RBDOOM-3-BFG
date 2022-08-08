#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec4 vofi_Color;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	fo_FragColor = sRGBAToLinearRGBA( vofi_Color );
}