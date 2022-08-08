
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

layout( location = 0 ) in vec4 in_Position;


void main()
{
	vec4 vPos = in_Position - rpLocalLightOrigin;
	vPos = ( vPos.wwww * rpLocalLightOrigin ) + vPos;
	gl_Position.x = dot4( vPos, rpMVPmatrixX );
	gl_Position.y = dot4( vPos, rpMVPmatrixY );
	gl_Position.z = dot4( vPos, rpMVPmatrixZ );
	gl_Position.w = dot4( vPos, rpMVPmatrixW );
}