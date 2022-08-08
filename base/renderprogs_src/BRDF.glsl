
float Distribution_GGX( float hdotN, float alpha )
{
	float a2 = alpha * alpha;
	float tmp = ( hdotN * a2 - hdotN ) * hdotN + 1.0;
	return ( a2 / ( PI * tmp * tmp ) );
}
float Distribution_GGX_Disney( float hdotN, float alphaG )
{
	float a2 = alphaG * alphaG;
	float tmp = ( hdotN * hdotN ) * ( a2 - 1.0 ) + 1.0;
	return ( a2 / ( PI * tmp ) );
}
float Distribution_GGX_1886( float hdotN, float alpha )
{
	return ( alpha / ( PI * pow( hdotN * hdotN * ( alpha - 1.0 ) + 1.0, 2.0 ) ) );
}
vec3 Fresnel_Schlick( vec3 specularColor, float vDotN )
{
	return specularColor + ( 1.0 - specularColor ) * pow( 1.0 - vDotN, 5.0 );
}
vec3 Fresnel_SchlickRoughness( vec3 specularColor, float vDotN, float roughness )
{
	return specularColor + ( max( vec3( 1.0 - roughness ), specularColor ) - specularColor ) * pow( 1.0 - vDotN, 5.0 );
}
float ComputeSpecularAO( float vDotN, float ao, float roughness )
{
	return clamp( pow( vDotN + ao, exp2( -16.0 * roughness - 1.0 ) ) - 1.0 + ao, 0.0, 1.0 );
}
float Visibility_Schlick( float vdotN, float ldotN, float alpha )
{
	float k = alpha * 0.5;
	float schlickL = ( ldotN * ( 1.0 - k ) + k );
	float schlickV = ( vdotN * ( 1.0 - k ) + k );
	return ( 0.25 / ( schlickL * schlickV ) );
}
float Visibility_SmithGGX( float vdotN, float ldotN, float alpha )
{
	float V1 = ldotN + sqrt( alpha + ( 1.0 - alpha ) * ldotN * ldotN );
	float V2 = vdotN + sqrt( alpha + ( 1.0 - alpha ) * vdotN * vdotN );
	return ( 1.0 / max( V1 * V2, 0.15 ) );
}
float EstimateLegacyRoughness( vec3 specMapSRGB )
{
	float Y = dot( LUMINANCE_SRGB.rgb, specMapSRGB );
	float glossiness = clamp( pow( Y, 1.0 / 2.0 ), 0.0, 0.98 );
	float roughness = 1.0 - glossiness;
	return roughness;
}