#version 330 compatibility
//enable the pointer data load
#extension GL_NV_shader_buffer_load : enable

in vec3 TexCoord;
out vec4 frag_color;

//volume channel, support maximum 6 channels 
uniform highp usampler2D channel0;
uniform highp usampler2D channel1;
uniform highp usampler2D channel2;

vec4 tex_uv; // texture coordination

#define FILL_CHANNEL 3

unsigned int	mCRaw[FILL_CHANNEL];  //raw data for each channel	

void main()
{
	tex_uv = vec4(TexCoord, 1.0);
	
	//test hist

	mCRaw[0] = (unsigned int)(texture(channel0, tex_uv.xy).r); 
	mCRaw[1] = (unsigned int)(texture(channel1, tex_uv.xy).r); 
	mCRaw[2] = (unsigned int)(texture(channel2, tex_uv.xy).r); 

	vec4 vColor;

	vColor.r = ((float)(mCRaw[0])) / 1024;
	vColor.g = ((float)(mCRaw[1])) / 1024;
	vColor.b = ((float)(mCRaw[2])) / 1024;

	vColor.a = 255;

	frag_color = vColor;
}