#version 430 core
//enable the pointer data load
#extension GL_NV_shader_buffer_load : enable

in vec3 TexCoord;
out vec4 frag_color;

//volume channel, support maximum 6 channels 
uniform sampler2DArray volume;

uniform sampler2D colormap;

uniform vec3 channel;

void main()
{	

	vec4 vColor = texture(volume, TexCoord);

	vec4 C1 = texture2D(colormap, vec2(vColor.r, channel.x));
	vec4 C2 = texture2D(colormap, vec2(vColor.g, channel.y));
	vec4 C3 = texture2D(colormap, vec2(vColor.b, channel.z));

	vec3 aC1 = C1.rgb * C1.a;
	vec3 aC2 = C2.rgb * C2.a;
	vec3 aC3 = C3.rgb * C3.a;
	float Amean = (C1.a + C2.a + C3.a)/3.0;

	vec4 oColor;

	oColor.rgb = max(aC1, max(aC2, aC3));
	oColor.a = Amean;

	frag_color = oColor;
}