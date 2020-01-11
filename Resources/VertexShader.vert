attribute vec4 color;
attribute vec3 pos;
attribute vec2 coord;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

varying vec4 M_Color;
varying vec2 M_Coord;

void main()
{	
	gl_Position = P * V * M * vec4(pos, 1.0);
	
	M_Color = color;
	M_Coord = coord;
}
