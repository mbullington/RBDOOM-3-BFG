
layout( binding = 1 ) uniform UBO_MAT {
	 vec4 matrices[ 408 ];
};

layout( binding = 0 ) uniform UBOV {
	vec4 rpWindowCoord;
	vec4 rpLocalLightOrigin;
	vec4 rpMVPmatrixX;
	vec4 rpMVPmatrixY;
	vec4 rpMVPmatrixZ;
	vec4 rpMVPmatrixW;
	vec4 rpJitterTexOffset;
};

#include "renderprogs_src/global.glsl"

layout( location = 0 ) in vec3 in_Position;
layout( location = 1 ) in vec4 in_Color;
layout( location = 2 ) in vec4 in_Color2;


void main()
{
	vec4 position4 = vec4( in_Position, 1.0 );

	float w0 = in_Color2.x;
	float w1 = in_Color2.y;
	float w2 = in_Color2.z;
	float w3 = in_Color2.w;
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
	vec4 vertexPosition = position4;
	vertexPosition.w = 1.0;
	vec4 modelPosition;
	modelPosition.x = dot4( matX, vertexPosition );
	modelPosition.y = dot4( matY, vertexPosition );
	modelPosition.z = dot4( matZ, vertexPosition );
	modelPosition.w = position4.w;
	vec4 vPos = modelPosition - rpLocalLightOrigin;
	vPos = ( vPos.wwww * rpLocalLightOrigin ) + vPos;
	gl_Position.x = dot4( vPos, rpMVPmatrixX );
	gl_Position.y = dot4( vPos, rpMVPmatrixY );
	gl_Position.z = dot4( vPos, rpMVPmatrixZ );
	gl_Position.w = dot4( vPos, rpMVPmatrixW );
}