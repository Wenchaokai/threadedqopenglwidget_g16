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
uniform int format_bgra;

struct RGBA16 {
	unsigned short r, g, b, a;
};
struct RGB8 {
	unsigned char r, g, b;
};

#define HistRangeCols 8 
#define FILL_CHANNEL 3

unsigned int	mCRaw[FILL_CHANNEL];  //raw data for each channel
unsigned int	mC[FILL_CHANNEL];	  //data after rescale and brighteness  	

int channel_index_On[FILL_CHANNEL]; //which channels are on 
int channel_flag_On[FILL_CHANNEL];

int channels_On = 0;

unsigned int hist_min[FILL_CHANNEL];
unsigned int hist_max[FILL_CHANNEL];
unsigned int vrange[FILL_CHANNEL];

RGBA16  channel_color[FILL_CHANNEL];
RGBA16	chColor;

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
		hist_min[ch] = (unsigned int)((texture(texHistRange, vec2(col, row)).r));
		
		//hist max
		colIndex++;
		col = colIndex / HistRangeCols_FACTOR;
		hist_max[ch] = (unsigned int)((texture(texHistRange, vec2(col, row)).r));
		//v range
		colIndex++;
		col = colIndex / HistRangeCols_FACTOR;
		vrange[ch] = (unsigned int)((texture(texHistRange, vec2(col, row)).r));

		//channel color, r 
		colIndex++;
		col = colIndex / HistRangeCols_FACTOR;
		channel_color[ch].r = (unsigned short)((texture(texHistRange, vec2(col, row)).r));

		//channel color, g 
		colIndex++;
		col = colIndex / HistRangeCols_FACTOR;
		channel_color[ch].g = (unsigned short)((texture(texHistRange, vec2(col, row)).r));

		//channel color, b 
		colIndex++;
		col = colIndex / HistRangeCols_FACTOR;
		channel_color[ch].b = (unsigned short)((texture(texHistRange, vec2(col, row)).r));

		//channel color, a 
		colIndex++;
		col = colIndex / HistRangeCols_FACTOR;
		channel_color[ch].a = (unsigned short)((texture(texHistRange, vec2(col, row)).r));

		// 
		mCRaw[ch] = 0U;

	}

}
//////////////////////////////////////
// lookup and mix multi-channel to RGBA8
#define OP_MAX	0
#define OP_SUM	1
#define OP_MEAN	2
#define OP_OIT	3  //Order Independent Transparency
#define OP_INDEX	-1

struct MixOP
{
	int op;
	bool rescale;
	bool maskR, maskG, maskB;
	float brightness, contrast; //ratio
};

//[PM] can save 100 ms
//#define AR(k) (A(k)*R(k))
//#define AG(k) (A(k)*G(k))
//#define AB(k) (A(k)*B(k))
#define AR(k) (R(k))
#define AG(k) (G(k))
#define AB(k) (B(k))

vec2 pos; 

#define LOOKUP_CHANNEL(k)\
	pos.x = ((float)(mC[k])) / (255.0f);\
	pos.y = ((float)(k)) / (FILL_CHANNEL - (1.0f));\
	mLut[k] = texture(texLuts, pos);\

RGB8 lookup_mix_litone( int op, RGB8 mask)
{
//
// get the mC[k] from the lookup table
//

#define R(k) ( (uint)(mLut[k].r) )
#define G(k) ( (uint)(mLut[k].g) )
#define B(k) ( (uint)(mLut[k].b) )
#define A(k) ( (uint)(mLut[k].a) )

	vec4 mLut[FILL_CHANNEL];

	uint o1, o2, o3;

	int k, k0; 
	k = k0 = channel_index_On[0];
	//mC[k] is rescale to[0, 255] - unsigned int

	pos.x = ((float)(mC[k])) / (255.0f);
	pos.y = ((float)(k)) / (FILL_CHANNEL - (1.0f));
	mLut[k] = texture(texLuts, pos);
	
	o1 = R(k0);
	o2 = G(k0);
	o3 = B(k0);

	//===========test lutr
	vec4 vColor;

	//test lutr

	int N = channels_On;
	//end
	for (int k_on = 1; k_on<channels_On; k_on++)
	{
		k = channel_index_On[k_on];
		LOOKUP_CHANNEL(k);
		o1 = max(o1, AR(k));
		o2 = max(o2, AG(k));
		o3 = max(o3, AB(k));
	}

	// OP_INDEX ignored
	uint clamp_max = 65536U;

	o1 = clamp(o1, 0U, clamp_max) >> 8;
	o2 = clamp(o1, 0U, clamp_max) >> 8;
	o3 = clamp(o1, 0U, clamp_max) >> 8;

	RGB8 oC;

	oC.r = (unsigned char)o1;
	oC.g = (unsigned char)o2;
	oC.b = (unsigned char)o3;
	oC.r &= mask.r;
	oC.g &= mask.g;
	oC.b &= mask.b;

	return oC;
}

void main()
{
	createHist();

	vec4 vColor;
	//test hist
	mCRaw[0] = (unsigned int)(texture(channel0, TexCoord).r); 
	mCRaw[1] = (unsigned int)(texture(channel1, TexCoord).r); 
	mCRaw[2] = (unsigned int)(texture(channel2, TexCoord).r); 

	MixOP mixOp;

	//MixOP init
	mixOp.rescale = true;
	mixOp.op = OP_MAX;
	mixOp.maskR = mixOp.maskG = mixOp.maskB = true;
	mixOp.brightness = 0;
	mixOp.contrast = 1;

	//get MixOP  value
	bool bRescale = mixOp.rescale;
	float fb = mixOp.brightness;
	float fc = mixOp.contrast;
	int op = mixOp.op;

	RGB8 mask;
	mask.r = (mixOp.maskR) ? (unsigned char)255 : (unsigned char)0;
	mask.g = (mixOp.maskG) ? (unsigned char)255 : (unsigned char)0;
	mask.b = (mixOp.maskB) ? (unsigned char)255 : (unsigned char)0;

	int k;
	unsigned int C;
	float CC;

	for (int k_on = 0; k_on < channels_On; k_on++)
	{
		k = channel_index_On[k_on];
		C = mCRaw[k];
		C = clamp(C, hist_min[k], hist_max[k]);
		CC = (float)C; 
		//rewrite to the original
		C = (uint)CC;
		mC[k] = (!bRescale) ? C : ((C - hist_min[k]) << 8) / vrange[k];  // rescale to [0, 255]
	} //end of K

	RGB8 mixC = lookup_mix_litone(op, mask);

	//End of lookup_mix_litone 

	vColor.r = ((float)(mixC.r)) / 255;
	vColor.g = ((float)(mixC.g)) / 255;
	vColor.b = ((float)(mixC.b)) / 255;
	vColor.a = (vColor.r + vColor.g + vColor.b) / 3; 

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

	frag_color = oColor; // modulated by color_proxy
	
}