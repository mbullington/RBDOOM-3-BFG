#define SMAA_GLSL_3 1
#define SMAA_PRESET_HIGH 1

#if defined(SMAA_PRESET_LOW)
#define SMAA_THRESHOLD 0.15
#define SMAA_MAX_SEARCH_STEPS 4
#define SMAA_DISABLE_DIAG_DETECTION
#define SMAA_DISABLE_CORNER_DETECTION
#elif defined(SMAA_PRESET_MEDIUM)
#define SMAA_THRESHOLD 0.1
#define SMAA_MAX_SEARCH_STEPS 8
#define SMAA_DISABLE_DIAG_DETECTION
#define SMAA_DISABLE_CORNER_DETECTION
#elif defined(SMAA_PRESET_HIGH)
#define SMAA_THRESHOLD 0.1
#define SMAA_MAX_SEARCH_STEPS 16
#define SMAA_MAX_SEARCH_STEPS_DIAG 8
#define SMAA_CORNER_ROUNDING 25
#elif defined(SMAA_PRESET_ULTRA)
#define SMAA_THRESHOLD 0.05
#define SMAA_MAX_SEARCH_STEPS 32
#define SMAA_MAX_SEARCH_STEPS_DIAG 16
#define SMAA_CORNER_ROUNDING 25
#endif 
#ifndef SMAA_THRESHOLD
#define SMAA_THRESHOLD 0.1
#endif 
#ifndef SMAA_DEPTH_THRESHOLD
#define SMAA_DEPTH_THRESHOLD (0.1 * SMAA_THRESHOLD)
#endif 
#ifndef SMAA_MAX_SEARCH_STEPS
#define SMAA_MAX_SEARCH_STEPS 16
#endif 
#ifndef SMAA_MAX_SEARCH_STEPS_DIAG
#define SMAA_MAX_SEARCH_STEPS_DIAG 8
#endif 
#ifndef SMAA_CORNER_ROUNDING
#define SMAA_CORNER_ROUNDING 25
#endif 
#ifndef SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR
#define SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR 2.0
#endif 
#ifndef SMAA_PREDICATION
#define SMAA_PREDICATION 0
#endif 
#ifndef SMAA_PREDICATION_THRESHOLD
#define SMAA_PREDICATION_THRESHOLD 0.01
#endif 
#ifndef SMAA_PREDICATION_SCALE
#define SMAA_PREDICATION_SCALE 2.0
#endif 
#ifndef SMAA_PREDICATION_STRENGTH
#define SMAA_PREDICATION_STRENGTH 0.4
#endif 
#ifndef SMAA_REPROJECTION
#define SMAA_REPROJECTION 0
#endif 
#ifndef SMAA_REPROJECTION_WEIGHT_SCALE
#define SMAA_REPROJECTION_WEIGHT_SCALE 30.0
#endif 
#ifndef SMAA_INCLUDE_VS
#define SMAA_INCLUDE_VS 1
#endif 
#ifndef SMAA_INCLUDE_PS
#define SMAA_INCLUDE_PS 1
#endif 
#ifndef SMAA_AREATEX_SELECT
#if defined(SMAA_HLSL_3)
#define SMAA_AREATEX_SELECT(sample) sample.ra
#else 
#define SMAA_AREATEX_SELECT(sample) sample.rg
#endif 
#endif 
#ifndef SMAA_SEARCHTEX_SELECT
#define SMAA_SEARCHTEX_SELECT(sample) sample.r
#endif 
#ifndef SMAA_DECODE_VELOCITY
#define SMAA_DECODE_VELOCITY(sample) sample.rg
#endif 
#define SMAA_AREATEX_MAX_DISTANCE 16
#define SMAA_AREATEX_MAX_DISTANCE_DIAG 20
#define SMAA_AREATEX_PIXEL_SIZE (1.0 / vec2(160.0, 560.0))
#define SMAA_AREATEX_SUBTEX_SIZE (1.0 / 7.0)
#define SMAA_SEARCHTEX_SIZE vec2(66.0, 33.0)
#define SMAA_SEARCHTEX_PACKED_SIZE vec2(64.0, 16.0)
#define SMAA_CORNER_ROUNDING_NORM (float(SMAA_CORNER_ROUNDING) / 100.0)
#if defined(SMAA_HLSL_3)
#define API_V_DIR(v) v
#define API_V_COORD(v) v
#define API_V_BELOW(v1, v2) v1 > v2
#define API_V_ABOVE(v1, v2) v1 < v2
#define SMAATexture2D(tex) sampler2D tex
#define SMAATexturePass2D(tex) tex
#define SMAASampleLevelZero(tex, coord) tex2Dlod(tex, float4(coord, 0.0, 0.0))
#define SMAASampleLevelZeroPoint(tex, coord) tex2Dlod(tex, float4(coord, 0.0, 0.0))
#define SMAASampleLevelZeroOffset(tex, coord, offset) tex2Dlod(tex, float4(coord + offset * rpScreenCorrectionFactor.xy, 0.0, 0.0))
#define SMAASample(tex, coord) tex2D(tex, coord)
#define SMAASamplePoint(tex, coord) tex2D(tex, coord)
#define SMAASampleOffset(tex, coord, offset) tex2D(tex, coord + offset * rpScreenCorrectionFactor.xy)
#define SMAA_FLATTEN [flatten]
#define SMAA_BRANCH [branch]
#endif 
#if defined(SMAA_HLSL_4) || defined(SMAA_HLSL_4_1)
#define API_V_DIR(v) v
#define API_V_COORD(v) v
#define API_V_BELOW(v1, v2) v1 > v2
#define API_V_ABOVE(v1, v2) v1 < v2
SamplerState LinearSampler { Filter = MIN_MAG_LINEAR_MIP_POINT; AddressU = Clamp; AddressV = Clamp; };
SamplerState PointSampler { Filter = MIN_MAG_MIP_POINT; AddressU = Clamp; AddressV = Clamp; };
#define SMAATexture2D(tex) Texture2D tex
#define SMAATexturePass2D(tex) tex
#define SMAASampleLevelZero(tex, coord) tex.SampleLevel(LinearSampler, coord, 0)
#define SMAASampleLevelZeroPoint(tex, coord) tex.SampleLevel(PointSampler, coord, 0)
#define SMAASampleLevelZeroOffset(tex, coord, offset) tex.SampleLevel(LinearSampler, coord, 0, offset)
#define SMAASample(tex, coord) tex.Sample(LinearSampler, coord)
#define SMAASamplePoint(tex, coord) tex.Sample(PointSampler, coord)
#define SMAASampleOffset(tex, coord, offset) tex.Sample(LinearSampler, coord, offset)
#define SMAA_FLATTEN [flatten]
#define SMAA_BRANCH [branch]
#define SMAATexture2DMS2(tex) Texture2DMS<float4, 2> tex
#define SMAALoad(tex, pos, sample) tex.Load(pos, sample)
#if defined(SMAA_HLSL_4_1)
#define SMAAGather(tex, coord) tex.Gather(LinearSampler, coord, 0)
#endif 
#endif 
#if defined(SMAA_GLSL_3) || defined(SMAA_GLSL_4)
#define API_V_DIR(v) v
#define API_V_COORD(v) v
#define API_V_BELOW(v1, v2) v1 > v2
#define API_V_ABOVE(v1, v2) v1 < v2
#define SMAATexture2D(tex) sampler2D tex
#define SMAATexturePass2D(tex) tex
#define SMAASampleLevelZero(tex, coord) textureLod(tex, coord, 0.0)
#define SMAASampleLevelZeroPoint(tex, coord) textureLod(tex, coord, 0.0)
#define SMAASampleLevelZeroOffset(tex, coord, offset) textureLodOffset(tex, coord, 0.0, offset)
#define SMAASample(tex, coord) texture(tex, coord)
#define SMAASamplePoint(tex, coord) texture(tex, coord)
#define SMAASampleOffset(tex, coord, offset) texture(tex, coord, offset)
#define SMAA_FLATTEN
#define SMAA_BRANCH
#define lerp(a, b, t) mix(a, b, t)
#define saturate(a) clamp(a, 0.0, 10.0)
#if defined(SMAA_GLSL_4)
#define mad(a, b, c) fma(a, b, c)
#define SMAAGather(tex, coord) textureGather(tex, coord)
#else 
#define mad(a, b, c) (a * b + c)
#endif 
#endif 
#if !defined(SMAA_HLSL_3) && !defined(SMAA_HLSL_4) && !defined(SMAA_HLSL_4_1) && !defined(SMAA_GLSL_3) && !defined(SMAA_GLSL_4) && !defined(SMAA_CUSTOM_SL)
#error you must define the shading language: SMAA_HLSL_*, SMAA_GLSL_* or SMAA_CUSTOM_SL
#endif 
vec3 SMAAGatherNeighbours( vec2 texcoord,
vec4 offset[3],
SMAATexture2D( tex ) )
{
	#ifdef SMAAGather
	return SMAAGather( tex, texcoord + rpScreenCorrectionFactor.xy * vec2( -0.5, -0.5 ) ).grb;
	#else 
	float P = SMAASamplePoint( tex, texcoord ).r;
	float Pleft = SMAASamplePoint( tex, offset[0].xy ).r;
	float Ptop = SMAASamplePoint( tex, offset[0].zw ).r;
	return vec3( P, Pleft, Ptop );
	#endif 
}
vec2 SMAACalculatePredicatedThreshold( vec2 texcoord,
vec4 offset[3],
SMAATexture2D( predicationTex ) )
{
	vec3 neighbours = SMAAGatherNeighbours( texcoord, offset, SMAATexturePass2D( predicationTex ) );
	vec2 delta = abs( neighbours.xx - neighbours.yz );
	vec2 edges = step( SMAA_PREDICATION_THRESHOLD, delta );
	return SMAA_PREDICATION_SCALE * SMAA_THRESHOLD * ( 1.0 - SMAA_PREDICATION_STRENGTH * edges );
}
void SMAAMovc( bvec2 cond, inout vec2 variable, vec2 value )
{
	SMAA_FLATTEN if( cond.x )
	{
		variable.x = value.x;
	}
	SMAA_FLATTEN if( cond.y )
	{
		variable.y = value.y;
	}
}
void SMAAMovc( bvec4 cond, inout vec4 variable, vec4 value )
{
	SMAAMovc( cond.xy, variable.xy, value.xy );
	SMAAMovc( cond.zw, variable.zw, value.zw );
}
#if SMAA_INCLUDE_VS
void SMAAEdgeDetectionVS( vec2 texcoord,
out vec4 offset[3] )
{
	offset[0] = mad( rpScreenCorrectionFactor.xyxy, vec4( -1.0, 0.0, 0.0, API_V_DIR( -1.0 ) ), texcoord.xyxy );
	offset[1] = mad( rpScreenCorrectionFactor.xyxy, vec4( 1.0, 0.0, 0.0, API_V_DIR( 1.0 ) ), texcoord.xyxy );
	offset[2] = mad( rpScreenCorrectionFactor.xyxy, vec4( -2.0, 0.0, 0.0, API_V_DIR( -2.0 ) ), texcoord.xyxy );
}
void SMAABlendingWeightCalculationVS( vec2 texcoord,
out vec2 pixcoord,
out vec4 offset[3] )
{
	pixcoord = texcoord * rpScreenCorrectionFactor.zw;
	offset[0] = mad( rpScreenCorrectionFactor.xyxy, vec4( -0.25, API_V_DIR( -0.125 ), 1.25, API_V_DIR( -0.125 ) ), texcoord.xyxy );
	offset[1] = mad( rpScreenCorrectionFactor.xyxy, vec4( -0.125, API_V_DIR( -0.25 ), -0.125, API_V_DIR( 1.25 ) ), texcoord.xyxy );
	offset[2] = mad( rpScreenCorrectionFactor.xxyy,
	vec4( -2.0, 2.0, API_V_DIR( -2.0 ), API_V_DIR( 2.0 ) ) * float( SMAA_MAX_SEARCH_STEPS ),
	vec4( offset[0].xz, offset[1].yw ) );
}
void SMAANeighborhoodBlendingVS( vec2 texcoord,
out vec4 offset )
{
	offset = mad( rpScreenCorrectionFactor.xyxy, vec4( 1.0, 0.0, 0.0, API_V_DIR( 1.0 ) ), texcoord.xyxy );
}
#endif // SMAA_INCLUDE_VS
#if SMAA_INCLUDE_PS
vec2 SMAALumaEdgeDetectionPS( vec2 texcoord,
vec4 offset[3],
SMAATexture2D( colorTex )
#if SMAA_PREDICATION
, SMAATexture2D( predicationTex )
#endif 
)
{
	#if SMAA_PREDICATION
	vec2 threshold = SMAACalculatePredicatedThreshold( texcoord, offset, SMAATexturePass2D( predicationTex ) );
	#else 
	vec2 threshold = vec2( SMAA_THRESHOLD, SMAA_THRESHOLD );
	#endif 
	vec3 weights = vec3( 0.2126, 0.7152, 0.0722 );
	float L = dot( SMAASamplePoint( colorTex, texcoord ).rgb, weights );
	float Lleft = dot( SMAASamplePoint( colorTex, offset[0].xy ).rgb, weights );
	float Ltop = dot( SMAASamplePoint( colorTex, offset[0].zw ).rgb, weights );
	vec4 delta;
	delta.xy = abs( L - vec2( Lleft, Ltop ) );
	vec2 edges = step( threshold, delta.xy );
	if( dot( edges, vec2( 1.0, 1.0 ) ) == 0.0 )
	{
		discard;
	}
	float Lright = dot( SMAASamplePoint( colorTex, offset[1].xy ).rgb, weights );
	float Lbottom = dot( SMAASamplePoint( colorTex, offset[1].zw ).rgb, weights );
	delta.zw = abs( L - vec2( Lright, Lbottom ) );
	vec2 maxDelta = max( delta.xy, delta.zw );
	float Lleftleft = dot( SMAASamplePoint( colorTex, offset[2].xy ).rgb, weights );
	float Ltoptop = dot( SMAASamplePoint( colorTex, offset[2].zw ).rgb, weights );
	delta.zw = abs( vec2( Lleft, Ltop ) - vec2( Lleftleft, Ltoptop ) );
	maxDelta = max( maxDelta.xy, delta.zw );
	float finalDelta = max( maxDelta.x, maxDelta.y );
	edges.xy *= step( finalDelta, SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR * delta.xy );
	return edges;
}
vec2 SMAAColorEdgeDetectionPS( vec2 texcoord,
vec4 offset[3],
SMAATexture2D( colorTex )
#if SMAA_PREDICATION
, SMAATexture2D( predicationTex )
#endif 
)
{
	#if SMAA_PREDICATION
	vec2 threshold = SMAACalculatePredicatedThreshold( texcoord, offset, predicationTex );
	#else 
	vec2 threshold = vec2( SMAA_THRESHOLD, SMAA_THRESHOLD );
	#endif 
	vec4 delta;
	vec3 C = SMAASamplePoint( colorTex, texcoord ).rgb;
	vec3 Cleft = SMAASamplePoint( colorTex, offset[0].xy ).rgb;
	vec3 t = abs( C - Cleft );
	delta.x = max( max( t.r, t.g ), t.b );
	vec3 Ctop = SMAASamplePoint( colorTex, offset[0].zw ).rgb;
	t = abs( C - Ctop );
	delta.y = max( max( t.r, t.g ), t.b );
	vec2 edges = step( threshold, delta.xy );
	if( dot( edges, vec2( 1.0, 1.0 ) ) == 0.0 )
	{
		discard;
	}
	vec3 Cright = SMAASamplePoint( colorTex, offset[1].xy ).rgb;
	t = abs( C - Cright );
	delta.z = max( max( t.r, t.g ), t.b );
	vec3 Cbottom = SMAASamplePoint( colorTex, offset[1].zw ).rgb;
	t = abs( C - Cbottom );
	delta.w = max( max( t.r, t.g ), t.b );
	vec2 maxDelta = max( delta.xy, delta.zw );
	vec3 Cleftleft = SMAASamplePoint( colorTex, offset[2].xy ).rgb;
	t = abs( C - Cleftleft );
	delta.z = max( max( t.r, t.g ), t.b );
	vec3 Ctoptop = SMAASamplePoint( colorTex, offset[2].zw ).rgb;
	t = abs( C - Ctoptop );
	delta.w = max( max( t.r, t.g ), t.b );
	maxDelta = max( maxDelta.xy, delta.zw );
	float finalDelta = max( maxDelta.x, maxDelta.y );
	edges.xy *= step( finalDelta, SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR * delta.xy );
	return edges;
}
vec2 SMAADepthEdgeDetectionPS( vec2 texcoord,
vec4 offset[3],
SMAATexture2D( depthTex ) )
{
	vec3 neighbours = SMAAGatherNeighbours( texcoord, offset, SMAATexturePass2D( depthTex ) );
	vec2 delta = abs( neighbours.xx - vec2( neighbours.y, neighbours.z ) );
	vec2 edges = step( SMAA_DEPTH_THRESHOLD, delta );
	if( dot( edges, vec2( 1.0, 1.0 ) ) == 0.0 )
	{
		discard;
	}
	return edges;
}
#if !defined(SMAA_DISABLE_DIAG_DETECTION)
vec2 SMAADecodeDiagBilinearAccess( vec2 e )
{
	e.r = e.r * abs( 5.0 * e.r - 5.0 * 0.75 );
	return round( e );
}
vec4 SMAADecodeDiagBilinearAccess( vec4 e )
{
	e.rb = e.rb * abs( 5.0 * e.rb - 5.0 * 0.75 );
	return round( e );
}
vec2 SMAASearchDiag1( SMAATexture2D( edgesTex ), vec2 texcoord, vec2 dir, out vec2 e )
{
	dir.y = API_V_DIR( dir.y );
	vec4 coord = vec4( texcoord, -1.0, 1.0 );
	vec3 t = vec3( rpScreenCorrectionFactor.xy, 1.0 );
	while( coord.z < float( SMAA_MAX_SEARCH_STEPS_DIAG - 1 ) &&
	coord.w > 0.9 )
	{
		coord.xyz = mad( t, vec3( dir, 1.0 ), coord.xyz );
		e = SMAASampleLevelZero( edgesTex, coord.xy ).rg;
		coord.w = dot( e, vec2( 0.5, 0.5 ) );
	}
	return coord.zw;
}
vec2 SMAASearchDiag2( SMAATexture2D( edgesTex ), vec2 texcoord, vec2 dir, out vec2 e )
{
	dir.y = API_V_DIR( dir.y );
	vec4 coord = vec4( texcoord, -1.0, 1.0 );
	coord.x += 0.25 * rpScreenCorrectionFactor.x;
	vec3 t = vec3( rpScreenCorrectionFactor.xy, 1.0 );
	while( coord.z < float( SMAA_MAX_SEARCH_STEPS_DIAG - 1 ) &&
	coord.w > 0.9 )
	{
		coord.xyz = mad( t, vec3( dir, 1.0 ), coord.xyz );
		e = SMAASampleLevelZero( edgesTex, coord.xy ).rg;
		e = SMAADecodeDiagBilinearAccess( e );
		coord.w = dot( e, vec2( 0.5, 0.5 ) );
	}
	return coord.zw;
}
vec2 SMAAAreaDiag( SMAATexture2D( areaTex ), vec2 dist, vec2 e, float offset )
{
	vec2 texcoord = mad( vec2( SMAA_AREATEX_MAX_DISTANCE_DIAG, SMAA_AREATEX_MAX_DISTANCE_DIAG ), e, dist );
	texcoord = mad( SMAA_AREATEX_PIXEL_SIZE, texcoord, 0.5 * SMAA_AREATEX_PIXEL_SIZE );
	texcoord.x += 0.5;
	texcoord.y += SMAA_AREATEX_SUBTEX_SIZE * offset;
	texcoord.y = API_V_COORD( texcoord.y );
	return SMAA_AREATEX_SELECT( SMAASampleLevelZero( areaTex, texcoord ) );
}
vec2 SMAACalculateDiagWeights( SMAATexture2D( edgesTex ), SMAATexture2D( areaTex ), vec2 texcoord, vec2 e, vec4 subsampleIndices )
{
	vec2 weights = vec2( 0.0, 0.0 );
	vec4 d;
	vec2 end;
	if( e.r > 0.0 )
	{
		d.xz = SMAASearchDiag1( SMAATexturePass2D( edgesTex ), texcoord, vec2( -1.0, 1.0 ), end );
		d.x += float( end.y > 0.9 );
	}
	else
	{
		d.xz = vec2( 0.0, 0.0 );
	}
	d.yw = SMAASearchDiag1( SMAATexturePass2D( edgesTex ), texcoord, vec2( 1.0, -1.0 ), end );
	SMAA_BRANCH
	if( d.x + d.y > 2.0 )
	{
		vec4 coords = mad( vec4( -d.x + 0.25, API_V_DIR( d.x ), d.y, API_V_DIR( -d.y - 0.25 ) ), rpScreenCorrectionFactor.xyxy, texcoord.xyxy );
		vec4 c;
		c.xy = SMAASampleLevelZeroOffset( edgesTex, coords.xy, ivec2( -1, 0 ) ).rg;
		c.zw = SMAASampleLevelZeroOffset( edgesTex, coords.zw, ivec2( 1, 0 ) ).rg;
		c.yxwz = SMAADecodeDiagBilinearAccess( c.xyzw );
		vec2 cc = mad( vec2( 2.0, 2.0 ), c.xz, c.yw );
		SMAAMovc( bvec2( step( 0.9, d.zw ) ), cc, vec2( 0.0, 0.0 ) );
		weights += SMAAAreaDiag( SMAATexturePass2D( areaTex ), d.xy, cc, subsampleIndices.z );
	}
	d.xz = SMAASearchDiag2( SMAATexturePass2D( edgesTex ), texcoord, vec2( -1.0, -1.0 ), end );
	if( SMAASampleLevelZeroOffset( edgesTex, texcoord, ivec2( 1, 0 ) ).r > 0.0 )
	{
		d.yw = SMAASearchDiag2( SMAATexturePass2D( edgesTex ), texcoord, vec2( 1.0, 1.0 ), end );
		d.y += float( end.y > 0.9 );
	}
	else
	{
		d.yw = vec2( 0.0, 0.0 );
	}
	SMAA_BRANCH
	if( d.x + d.y > 2.0 )
	{
		vec4 coords = mad( vec4( -d.x, API_V_DIR( -d.x ), d.y, API_V_DIR( d.y ) ), rpScreenCorrectionFactor.xyxy, texcoord.xyxy );
		vec4 c;
		c.x = SMAASampleLevelZeroOffset( edgesTex, coords.xy, ivec2( -1, 0 ) ).g;
		c.y = SMAASampleLevelZeroOffset( edgesTex, coords.xy, ivec2( 0, API_V_DIR( -1 ) ) ).r;
		c.zw = SMAASampleLevelZeroOffset( edgesTex, coords.zw, ivec2( 1, 0 ) ).gr;
		vec2 cc = mad( vec2( 2.0, 2.0 ), c.xz, c.yw );
		SMAAMovc( bvec2( step( 0.9, d.zw ) ), cc, vec2( 0.0, 0.0 ) );
		weights += SMAAAreaDiag( SMAATexturePass2D( areaTex ), d.xy, cc, subsampleIndices.w ).gr;
	}
	return weights;
}
#endif 
float SMAASearchLength( SMAATexture2D( searchTex ), vec2 e, float offset )
{
	vec2 scale = SMAA_SEARCHTEX_SIZE * vec2( 0.5, -1.0 );
	vec2 bias = SMAA_SEARCHTEX_SIZE * vec2( offset, 1.0 );
	scale += vec2( -1.0, 1.0 );
	bias += vec2( 0.5, -0.5 );
	scale *= 1.0 / SMAA_SEARCHTEX_PACKED_SIZE;
	bias *= 1.0 / SMAA_SEARCHTEX_PACKED_SIZE;
	vec2 coord = mad( scale, e, bias );
	coord.y = API_V_COORD( coord.y );
	return SMAA_SEARCHTEX_SELECT( SMAASampleLevelZero( searchTex, coord ) );
}
float SMAASearchXLeft( SMAATexture2D( edgesTex ), SMAATexture2D( searchTex ), vec2 texcoord, float end )
{
	vec2 e = vec2( 0.0, 1.0 );
	while( texcoord.x > end &&
	e.g > 0.8281 &&
	e.r == 0.0 )
	{
		e = SMAASampleLevelZero( edgesTex, texcoord ).rg;
		texcoord = mad( -vec2( 2.0, 0.0 ), rpScreenCorrectionFactor.xy, texcoord );
	}
	float offset = mad( -( 255.0 / 127.0 ), SMAASearchLength( SMAATexturePass2D( searchTex ), e, 0.0 ), 3.25 );
	return mad( rpScreenCorrectionFactor.x, offset, texcoord.x );
}
float SMAASearchXRight( SMAATexture2D( edgesTex ), SMAATexture2D( searchTex ), vec2 texcoord, float end )
{
	vec2 e = vec2( 0.0, 1.0 );
	while( texcoord.x < end &&
	e.g > 0.8281 &&
	e.r == 0.0 )
	{
		e = SMAASampleLevelZero( edgesTex, texcoord ).rg;
		texcoord = mad( vec2( 2.0, 0.0 ), rpScreenCorrectionFactor.xy, texcoord );
	}
	float offset = mad( -( 255.0 / 127.0 ), SMAASearchLength( SMAATexturePass2D( searchTex ), e, 0.5 ), 3.25 );
	return mad( -rpScreenCorrectionFactor.x, offset, texcoord.x );
}
float SMAASearchYUp( SMAATexture2D( edgesTex ), SMAATexture2D( searchTex ), vec2 texcoord, float end )
{
	vec2 e = vec2( 1.0, 0.0 );
	while( API_V_BELOW( texcoord.y, end ) &&
	e.r > 0.8281 &&
	e.g == 0.0 )
	{
		e = SMAASampleLevelZero( edgesTex, texcoord ).rg;
		texcoord = mad( -vec2( 0.0, API_V_DIR( 2.0 ) ), rpScreenCorrectionFactor.xy, texcoord );
	}
	float offset = mad( -( 255.0 / 127.0 ), SMAASearchLength( SMAATexturePass2D( searchTex ), e.gr, 0.0 ), 3.25 );
	return mad( rpScreenCorrectionFactor.y, API_V_DIR( offset ), texcoord.y );
}
float SMAASearchYDown( SMAATexture2D( edgesTex ), SMAATexture2D( searchTex ), vec2 texcoord, float end )
{
	vec2 e = vec2( 1.0, 0.0 );
	while( API_V_ABOVE( texcoord.y, end ) &&
	e.r > 0.8281 &&
	e.g == 0.0 )
	{
		e = SMAASampleLevelZero( edgesTex, texcoord ).rg;
		texcoord = mad( vec2( 0.0, API_V_DIR( 2.0 ) ), rpScreenCorrectionFactor.xy, texcoord );
	}
	float offset = mad( -( 255.0 / 127.0 ), SMAASearchLength( SMAATexturePass2D( searchTex ), e.gr, 0.5 ), 3.25 );
	return mad( -rpScreenCorrectionFactor.y, API_V_DIR( offset ), texcoord.y );
}
vec2 SMAAArea( SMAATexture2D( areaTex ), vec2 dist, float e1, float e2, float offset )
{
	vec2 texcoord = mad( vec2( SMAA_AREATEX_MAX_DISTANCE, SMAA_AREATEX_MAX_DISTANCE ), round( 4.0 * vec2( e1, e2 ) ), dist );
	texcoord = mad( SMAA_AREATEX_PIXEL_SIZE, texcoord, 0.5 * SMAA_AREATEX_PIXEL_SIZE );
	texcoord.y = mad( SMAA_AREATEX_SUBTEX_SIZE, offset, texcoord.y );
	texcoord.y = API_V_COORD( texcoord.y );
	return SMAA_AREATEX_SELECT( SMAASampleLevelZero( areaTex, texcoord ) );
}
void SMAADetectHorizontalCornerPattern( SMAATexture2D( edgesTex ), inout vec2 weights, vec4 texcoord, vec2 d )
{
	#if !defined(SMAA_DISABLE_CORNER_DETECTION)
	vec2 leftRight = step( d.xy, d.yx );
	vec2 rounding = ( 1.0 - SMAA_CORNER_ROUNDING_NORM ) * leftRight;
	rounding /= leftRight.x + leftRight.y;
	vec2 factor = vec2( 1.0, 1.0 );
	factor.x -= rounding.x * SMAASampleLevelZeroOffset( edgesTex, texcoord.xy, ivec2( 0, API_V_DIR( 1 ) ) ).r;
	factor.x -= rounding.y * SMAASampleLevelZeroOffset( edgesTex, texcoord.zw, ivec2( 1, API_V_DIR( 1 ) ) ).r;
	factor.y -= rounding.x * SMAASampleLevelZeroOffset( edgesTex, texcoord.xy, ivec2( 0, API_V_DIR( -2 ) ) ).r;
	factor.y -= rounding.y * SMAASampleLevelZeroOffset( edgesTex, texcoord.zw, ivec2( 1, API_V_DIR( -2 ) ) ).r;
	weights *= saturate( factor );
	#endif 
}
void SMAADetectVerticalCornerPattern( SMAATexture2D( edgesTex ), inout vec2 weights, vec4 texcoord, vec2 d )
{
	#if !defined(SMAA_DISABLE_CORNER_DETECTION)
	vec2 leftRight = step( d.xy, d.yx );
	vec2 rounding = ( 1.0 - SMAA_CORNER_ROUNDING_NORM ) * leftRight;
	rounding /= leftRight.x + leftRight.y;
	vec2 factor = vec2( 1.0, 1.0 );
	factor.x -= rounding.x * SMAASampleLevelZeroOffset( edgesTex, texcoord.xy, ivec2( 1, 0 ) ).g;
	factor.x -= rounding.y * SMAASampleLevelZeroOffset( edgesTex, texcoord.zw, ivec2( 1, API_V_DIR( 1 ) ) ).g;
	factor.y -= rounding.x * SMAASampleLevelZeroOffset( edgesTex, texcoord.xy, ivec2( -2, 0 ) ).g;
	factor.y -= rounding.y * SMAASampleLevelZeroOffset( edgesTex, texcoord.zw, ivec2( -2, API_V_DIR( 1 ) ) ).g;
	weights *= saturate( factor );
	#endif 
}
vec4 SMAABlendingWeightCalculationPS( vec2 texcoord,
vec2 pixcoord,
vec4 offset[3],
SMAATexture2D( edgesTex ),
SMAATexture2D( areaTex ),
SMAATexture2D( searchTex ),
vec4 subsampleIndices )
{
	vec4 weights = vec4( 0.0, 0.0, 0.0, 0.0 );
	vec2 e = SMAASample( edgesTex, texcoord ).rg;
	SMAA_BRANCH
	if( e.g > 0.0 )
	{
		#if !defined(SMAA_DISABLE_DIAG_DETECTION)
		weights.rg = SMAACalculateDiagWeights( SMAATexturePass2D( edgesTex ), SMAATexturePass2D( areaTex ), texcoord, e, subsampleIndices );
		SMAA_BRANCH
		if( weights.r == -weights.g )
		{
			#endif 
			vec2 d;
			vec3 coords;
			coords.x = SMAASearchXLeft( SMAATexturePass2D( edgesTex ), SMAATexturePass2D( searchTex ), offset[0].xy, offset[2].x );
			coords.y = offset[1].y;
			d.x = coords.x;
			float e1 = SMAASampleLevelZero( edgesTex, coords.xy ).r;
			coords.z = SMAASearchXRight( SMAATexturePass2D( edgesTex ), SMAATexturePass2D( searchTex ), offset[0].zw, offset[2].y );
			d.y = coords.z;
			d = abs( round( mad( rpScreenCorrectionFactor.zz, d, -pixcoord.xx ) ) );
			vec2 sqrt_d = sqrt( d );
			float e2 = SMAASampleLevelZeroOffset( edgesTex, coords.zy, ivec2( 1, 0 ) ).r;
			weights.rg = SMAAArea( SMAATexturePass2D( areaTex ), sqrt_d, e1, e2, subsampleIndices.y );
			coords.y = texcoord.y;
			SMAADetectHorizontalCornerPattern( SMAATexturePass2D( edgesTex ), weights.rg, coords.xyzy, d );
			#if !defined(SMAA_DISABLE_DIAG_DETECTION)
		}
		else
		{
			e.r = 0.0;
		}
		#endif 
	}
	SMAA_BRANCH
	if( e.r > 0.0 )
	{
		vec2 d;
		vec3 coords;
		coords.y = SMAASearchYUp( SMAATexturePass2D( edgesTex ), SMAATexturePass2D( searchTex ), offset[1].xy, offset[2].z );
		coords.x = offset[0].x;
		d.x = coords.y;
		float e1 = SMAASampleLevelZero( edgesTex, coords.xy ).g;
		coords.z = SMAASearchYDown( SMAATexturePass2D( edgesTex ), SMAATexturePass2D( searchTex ), offset[1].zw, offset[2].w );
		d.y = coords.z;
		d = abs( round( mad( rpScreenCorrectionFactor.ww, d, -pixcoord.yy ) ) );
		vec2 sqrt_d = sqrt( d );
		float e2 = SMAASampleLevelZeroOffset( edgesTex, coords.xz, ivec2( 0, API_V_DIR( 1 ) ) ).g;
		weights.ba = SMAAArea( SMAATexturePass2D( areaTex ), sqrt_d, e1, e2, subsampleIndices.x );
		coords.x = texcoord.x;
		SMAADetectVerticalCornerPattern( SMAATexturePass2D( edgesTex ), weights.ba, coords.xyxz, d );
	}
	return weights;
}
vec4 SMAANeighborhoodBlendingPS( vec2 texcoord,
vec4 offset,
SMAATexture2D( colorTex ),
SMAATexture2D( blendTex )
#if SMAA_REPROJECTION
, SMAATexture2D( velocityTex )
#endif 
)
{
	vec4 a;
	a.x = SMAASample( blendTex, offset.xy ).a;
	a.y = SMAASample( blendTex, offset.zw ).g;
	a.wz = SMAASample( blendTex, texcoord ).xz;
	SMAA_BRANCH
	if( dot( a, vec4( 1.0, 1.0, 1.0, 1.0 ) ) < 1e-5 )
	{
		vec4 color = SMAASampleLevelZero( colorTex, texcoord );
		#if SMAA_REPROJECTION
		vec2 velocity = SMAA_DECODE_VELOCITY( SMAASampleLevelZero( velocityTex, texcoord ) );
		color.a = sqrt( 5.0 * length( velocity ) );
		#endif 
		return color;
	}
	else
	{
		bool h = max( a.x, a.z ) > max( a.y, a.w );
		vec4 blendingOffset = vec4( 0.0, API_V_DIR( a.y ), 0.0, API_V_DIR( a.w ) );
		vec2 blendingWeight = a.yw;
		SMAAMovc( bvec4( h, h, h, h ), blendingOffset, vec4( a.x, 0.0, a.z, 0.0 ) );
		SMAAMovc( bvec2( h, h ), blendingWeight, a.xz );
		blendingWeight /= dot( blendingWeight, vec2( 1.0, 1.0 ) );
		vec4 blendingCoord = mad( blendingOffset, vec4( rpScreenCorrectionFactor.xy, -rpScreenCorrectionFactor.xy ), texcoord.xyxy );
		vec4 color = blendingWeight.x * SMAASampleLevelZero( colorTex, blendingCoord.xy );
		color += blendingWeight.y * SMAASampleLevelZero( colorTex, blendingCoord.zw );
		#if SMAA_REPROJECTION
		vec2 velocity = blendingWeight.x * SMAA_DECODE_VELOCITY( SMAASampleLevelZero( velocityTex, blendingCoord.xy ) );
		velocity += blendingWeight.y * SMAA_DECODE_VELOCITY( SMAASampleLevelZero( velocityTex, blendingCoord.zw ) );
		color.a = sqrt( 5.0 * length( velocity ) );
		#endif 
		return color;
	}
}
vec4 SMAAResolvePS( vec2 texcoord,
SMAATexture2D( currentColorTex ),
SMAATexture2D( previousColorTex )
#if SMAA_REPROJECTION
, SMAATexture2D( velocityTex )
#endif 
)
{
	#if SMAA_REPROJECTION
	vec2 velocity = -SMAA_DECODE_VELOCITY( SMAASamplePoint( velocityTex, texcoord ).rg );
	vec4 current = SMAASamplePoint( currentColorTex, texcoord );
	vec4 previous = SMAASamplePoint( previousColorTex, texcoord + velocity );
	float delta = abs( current.a * current.a - previous.a * previous.a ) / 5.0;
	float weight = 0.5 * saturate( 1.0 - sqrt( delta ) * SMAA_REPROJECTION_WEIGHT_SCALE );
	return mix( current, previous, weight );
	#else 
	vec4 current = SMAASamplePoint( currentColorTex, texcoord );
	vec4 previous = SMAASamplePoint( previousColorTex, texcoord );
	return mix( current, previous, 0.5 );
	#endif 
}
#ifdef SMAALoad
void SMAASeparatePS( vec4 position,
vec2 texcoord,
out vec4 target0,
out vec4 target1,
SMAATexture2DMS2( colorTexMS ) )
{
	ivec2 pos = ivec2( position.xy );
	target0 = SMAALoad( colorTexMS, pos, 0 );
	target1 = SMAALoad( colorTexMS, pos, 1 );
}
#endif 
#endif // SMAA_INCLUDE_PS