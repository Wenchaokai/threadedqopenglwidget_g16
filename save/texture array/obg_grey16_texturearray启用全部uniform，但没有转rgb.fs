#version 430 core
//enable the pointer data load
#extension GL_NV_shader_buffer_load : enable

in vec3 TexCoord;
out vec4 frag_color;

//volume channel, support maximum 6 channels 
uniform highp usampler2DArray channel0;
uniform highp usampler2DArray channel1;
uniform highp usampler2DArray channel2;

uniform highp usampler2D texLuts;  //grey 16 rgba, channels*256
uniform highp usampler2D texHistRange; //channels * [channelOn, min, max, vrange, color]

uniform sampler2D colormap;

uniform vec3 channel;
uniform int blend_mode;

#define FILL_CHANNEL 3

unsigned int mCRaw[FILL_CHANNEL];  //raw data for each channel	

void main()
{	
	//test hist
	mCRaw[0] = (unsigned int)(texture(channel0, TexCoord).r); 
	mCRaw[1] = (unsigned int)(texture(channel1, TexCoord).r); 
	mCRaw[2] = (unsigned int)(texture(channel2, TexCoord).r); 

	vec4 vColor;

	vColor.r = ((float)(mCRaw[0])) / 1024;
	vColor.g = ((float)(mCRaw[1])) / 1024;
	vColor.b = ((float)(mCRaw[2])) / 1024;

	vColor.a = 1.0;

	vec4 mLut = texture(texLuts, vec2(1,1));
	vec4 hist = texture(texHistRange, vec2(1, 1));
	vec4 test = (mLut + hist)/10;

	vec4 C1 = texture2D(colormap, vec2(vColor.r, channel.x));
	vec4 C2 = texture2D(colormap, vec2(vColor.g, channel.y));
	vec4 C3 = texture2D(colormap, vec2(vColor.b, channel.z));

	vec3 aC1 = C1.rgb * C1.a;
	vec3 aC2 = C2.rgb * C2.a;
	vec3 aC3 = C3.rgb * C3.a;
	float Amean = (C1.a + C2.a + C3.a) / 3.0;
	float Amax = max(C1.a, max(C2.a, C3.a));

	vec4 oColor;
	float Amip = Amean;
	oColor.rgb = max(aC1.rgb, max(aC2.rgb, aC3.rgb));
	oColor.a = Amip;

	frag_color = oColor + test;
}