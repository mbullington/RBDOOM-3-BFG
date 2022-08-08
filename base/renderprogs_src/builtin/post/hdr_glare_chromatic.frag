layout( binding = 1 ) uniform UBOV {
	vec4 rpWindowCoord;
};
layout( binding = 2 ) uniform sampler2D samp0;

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec2 vofi_TexCoord0;

layout( location = 0 ) out vec4 fo_FragColor;

float linterp( float t )
{
	return saturate( 1.0 - abs( 2.0 * t - 1.0 ) );
}
float remap( float t, float a, float b )
{
	return saturate( ( t - a ) / ( b - a ) );
}
vec3 spectrumoffset( float t )
{
	float lo = step( t, 0.5 );
	float hi = 1.0 - lo;
	float w = linterp( remap( t, 1.0 / 6.0, 5.0 / 6.0 ) );
	vec3 ret = vec3( lo, 1.0, hi ) * vec3( 1.0 - w, w, 1.0 - w );
	return ret;
}
void main()
{
	vec2 st = vofi_TexCoord0;
	vec4 color = texture( samp0, st );
	float gaussFact[9] = float[9]( 0.13298076, 0.12579441, 0.10648267, 0.08065691, 0.05467002, 0.03315905, 0.01799699, 0.00874063, 0.00379866 );
	vec3 chromaticOffsets[9] = vec3[](
	vec3( 0.5, 0.5, 0.5 ),
	vec3( 0.8, 0.3, 0.3 ),
	vec3( 0.5, 0.2, 0.8 ),
	vec3( 0.2, 0.2, 1.0 ),
	vec3( 0.2, 0.3, 0.9 ),
	vec3( 0.2, 0.9, 0.2 ),
	vec3( 0.3, 0.5, 0.3 ),
	vec3( 0.3, 0.5, 0.3 ),
	vec3( 0.3, 0.5, 0.3 )
	);
	vec3 sumColor = vec3( 0.0 );
	vec3 sumSpectrum = vec3( 0.0 );
	int tap = 4;
	int samples = 9;
	float scale = 13.0;
	float weightScale = 2.3;
	for( int i = 0; i < samples; i++ )
	{
		vec3 so = chromaticOffsets[ i ];
		vec4 color = texture( samp0, st + vec2( float( i ), 0 ) * rpWindowCoord.xy * scale );
		float weight = gaussFact[ i ];
		sumColor += color.rgb * ( so.rgb * weight * weightScale );
	}
	#if 1
	for( int i = 1; i < samples; i++ )
	{
		vec3 so = chromaticOffsets[ i ];
		vec4 color = texture( samp0, st + vec2( float( -i ), 0 ) * rpWindowCoord.xy * scale );
		float weight = gaussFact[ i ];
		sumColor += color.rgb * ( so.rgb * weight * weightScale );
	}
	#endif 
	fo_FragColor = vec4( sumColor, 1.0 );
}