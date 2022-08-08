layout( binding = 2 ) uniform UBOF {
	vec4 rpWindowCoord;
};
layout( binding = 3 ) uniform sampler2D samp0;
layout( binding = 4 ) uniform sampler2D samp1;

#include "renderprogs_src/global.glsl"
#include "renderprogs_src/world.glsl"

layout( location = 0 ) in vec4 vofi_TexCoord0;
layout( location = 1 ) in vec4 vofi_TexCoord1;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	vec4 bumpMap = ( texture( samp1, vofi_TexCoord0.xy ) * 2.0 ) - 1.0;
	vec2 localNormal = bumpMap.wy;
	vec2 screenTexCoord = vposToScreenPosTexCoord( gl_FragCoord.xy );
	screenTexCoord += ( localNormal * vofi_TexCoord1.xy );
	screenTexCoord = saturate( screenTexCoord );
	fo_FragColor = ( texture( samp0, screenTexCoord.xy ) );
}