layout( binding = 1 ) uniform sampler2D samp0;
layout( binding = 2 ) uniform sampler2D samp1;

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec2 vofi_TexCoord0;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	if( fract( gl_FragCoord.y * 0.5 ) < 0.5 )
	{
		fo_FragColor = texture( samp0, vec2( vofi_TexCoord0 ) );
	}
	else
	{
		fo_FragColor = texture( samp1, vec2( vofi_TexCoord0 ) );
	}
}