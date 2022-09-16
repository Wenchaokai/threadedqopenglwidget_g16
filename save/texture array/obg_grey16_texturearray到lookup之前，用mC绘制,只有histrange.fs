#version 430 core
//enable the pointer data load
#extension GL_NV_shader_buffer_load : enable

in vec3 TexCoord;
out vec4 frag_color;

//volume channel, support maximum 6 channels 
uniform highp usampler2DArray channel0;
uniform highp usampler2DArray channel1;
uniform highp usampler2DArray channel2;

uniform highp usampler2D texHistRange; //channels * [channelOn, min, max, vrange, color]

struct RGBA16 {
	unsigned short r, g, b, a;
};
struct RGB8 {
	unsigned char r, g, b;
};

#define HistRangeCols 8 
#define FILL_CHANNEL 3

uint	mCRaw[FILL_CHANNEL];  //raw data for each channel
uint	mC[FILL_CHANNEL];	  //data after rescale and brighteness  	

int channel_index_On[FILL_CHANNEL]; //which channels are on 
int channel_flag_On[FILL_CHANNEL];

int channels_On = 0;

uint hist_min[FILL_CHANNEL];
uint hist_max[FILL_CHANNEL];
uint vrange[FILL_CHANNEL];

void createHist()
{
	//construct the 
	float FILL_CHANNEL_FACTOR = (FILL_CHANNEL - 1.0);
	float HistRangeCols_FACTOR = (HistRangeCols - 1.0);

	for (int ch = 0; ch < FILL_CHANNEL; ch++)
	{
		//
		float row = (float)(ch) / FILL_CHANNEL_FACTOR;
		float colIndex = 0.0; 
		float col = 0.0 / HistRangeCols_FACTOR;

		bool isOn = (bool)(texture(texHistRange, vec2(col, row)).r);
		channel_flag_On[ch] = 1; 

		channel_index_On[channels_On] = ch;
		channels_On++;
		//hist min
		colIndex++;
		col = colIndex / HistRangeCols_FACTOR;
		hist_min[ch] = (uint)((texture(texHistRange, vec2(col, row)).r));
		
		//hist max
		colIndex++;
		col = colIndex / HistRangeCols_FACTOR;
		hist_max[ch] = (uint)((texture(texHistRange, vec2(col, row)).r));

		//v range
		colIndex++;
		col = colIndex / HistRangeCols_FACTOR;
		vrange[ch] = (unsigned int)((texture(texHistRange, vec2(col, row)).r));
	}

}

void main()
{
	createHist();

	vec4 vColor;
	//test hist
	mCRaw[0] = (uint)(texture(channel0, TexCoord).r); 
	mCRaw[1] = (uint)(texture(channel1, TexCoord).r); 
	mCRaw[2] = (uint)(texture(channel2, TexCoord).r); 

	int k;
	uint C;
	float CC;

	for (int k_on = 0; k_on < channels_On; k_on++)
	{
		k = channel_index_On[k_on];
		C = mCRaw[k];
		C = clamp(C, hist_min[k], hist_max[k]);
		CC = (float)C; 
		//rewrite to the original
		C = (uint)CC;
		mC[k] =  ((C - hist_min[k]) << 8) / vrange[k];  // rescale to [0, 255]
	}
	
	vec4 oColor1 = vec4(mC[0], mC[1], mC[2], 0.3);
	vec4 oColor2 = vec4(mCRaw[0], mCRaw[1], mCRaw[2], 0.5);
	frag_color = oColor1 + oColor2;
}