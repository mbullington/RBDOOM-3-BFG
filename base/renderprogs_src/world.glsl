vec2 vposToScreenPosTexCoord( vec2 vpos )
{
	return vpos.xy * rpWindowCoord.xy;
}