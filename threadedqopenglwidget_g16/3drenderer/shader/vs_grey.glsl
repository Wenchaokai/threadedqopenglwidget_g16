#version 330 compatibility
layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 aTexCoord;

//out vec2 tex_uv;
out vec4 tex_uv;


void main()
{
	tex_uv = aTexCoord;
	gl_Position = gl_ModelViewProjectionMatrix * pos;

	//gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}