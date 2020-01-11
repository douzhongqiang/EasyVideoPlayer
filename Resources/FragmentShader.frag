varying vec4 M_Color;
varying vec2 M_Coord;

uniform sampler2D U_MainTexture;
uniform sampler2D Tex_y;
uniform sampler2D Tex_u;
uniform sampler2D Tex_v;
uniform mat4 YUVMat;

void main()
{
	// decodec YUV420
        if (YUVMat[3][3] > 0.5)
	{
		vec3 yuv;
		vec3 rgb;
		yuv.x = texture2D(Tex_y, M_Coord).r;
		yuv.y = texture2D(Tex_u, M_Coord).r - 0.5;
		yuv.z = texture2D(Tex_v, M_Coord).r - 0.5;
		rgb = mat3(1,       1,         1,
				   0,       -0.39465,  2.03211,
                   1.13983, -0.58060,  0) * yuv;
		gl_FragColor = vec4(rgb, 1);
	}
	else
	{
		gl_FragColor = texture2D(U_MainTexture, M_Coord);
	}
}
