layout( binding = 1 ) uniform sampler2D samp0;

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec4 vofi_TexCoord0;

layout( location = 0 ) out vec4 fo_FragColor;

void main()
{
	float screenWarp_range = 1.45;
	vec2 warpCenter = vec2( 0.5, 0.5 );
	vec2 centeredTexcoord = vofi_TexCoord0.xy - warpCenter;
	float radialLength = length( centeredTexcoord );
	vec2 radialDir = normalize( centeredTexcoord );
	float range = screenWarp_range;
	float scaledRadialLength = radialLength * range;
	float tanScaled = tan( scaledRadialLength );
	float rescaleValue = tan( 0.5 * range );
	float rescaled = tanScaled / rescaleValue;
	vec2 warped = warpCenter + vec2( 0.5, 0.5 ) * radialDir * rescaled;
	fo_FragColor = texture( samp0, warped );
}