
vec4 modelPosition = vec4( in_Position, 1.0 );
BRANCH if( rpEnableSkinning.x > 0.0 )
{
	const float w0 = in_Color2.x;
	const float w1 = in_Color2.y;
	const float w2 = in_Color2.z;
	const float w3 = in_Color2.w;
	vec4 matX, matY, matZ;
	int joint = int( in_Color.x * 255.1 * 3.0 );
	matX = matrices[int( joint + 0 )] * w0;
	matY = matrices[int( joint + 1 )] * w0;
	matZ = matrices[int( joint + 2 )] * w0;
	joint = int( in_Color.y * 255.1 * 3.0 );
	matX += matrices[int( joint + 0 )] * w1;
	matY += matrices[int( joint + 1 )] * w1;
	matZ += matrices[int( joint + 2 )] * w1;
	joint = int( in_Color.z * 255.1 * 3.0 );
	matX += matrices[int( joint + 0 )] * w2;
	matY += matrices[int( joint + 1 )] * w2;
	matZ += matrices[int( joint + 2 )] * w2;
	joint = int( in_Color.w * 255.1 * 3.0 );
	matX += matrices[int( joint + 0 )] * w3;
	matY += matrices[int( joint + 1 )] * w3;
	matZ += matrices[int( joint + 2 )] * w3;
	modelPosition.x = dot4( matX, vec4( in_Position, 1.0 ) );
	modelPosition.y = dot4( matY, vec4( in_Position, 1.0 ) );
	modelPosition.z = dot4( matZ, vec4( in_Position, 1.0 ) );
	modelPosition.w = 1.0;
}
gl_Position.x = dot4( modelPosition, rpMVPmatrixX );
gl_Position.y = dot4( modelPosition, rpMVPmatrixY );
gl_Position.z = dot4( modelPosition, rpMVPmatrixZ );
gl_Position.w = dot4( modelPosition, rpMVPmatrixW );