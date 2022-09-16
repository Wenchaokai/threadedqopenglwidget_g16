#version 460 compatibility
//enable the pointer data load
#extension GL_NV_shader_buffer_load : enable

//in vec4 tex_uv;
out vec4 frag_color;

//volume channel, support maximum 6 channels 
uniform highp usampler2D channel0;
uniform highp usampler2D channel1;
uniform highp usampler2D channel2;
uniform highp usampler2D channel3;
uniform highp usampler2D channel4;
uniform highp usampler2D channel5;

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

struct texHistRange16 {
	unsigned short channelOn;  //1: enable, 0: disable
	unsigned short hist_min;
	unsigned short hist_max;
	unsigned short v_range;
	RGBA16 color;
};

#define ushort unsigned short
#define uchar unsigned char

uniform int pLutsTest[6];

//
//uniform RGBA16 pLuts[3][2];

#define FILL_CHANNEL 4



// lookup and mix multi-channel to RGBA8
#define OP_MAX	0
#define OP_SUM	1
#define OP_MEAN	2
#define OP_OIT	3  //Order Independent Transparency
#define OP_INDEX	-1

#ifndef ABS
#define ABS(a)  ( ((a)>0)? (a) : -(a) )
#endif
#ifndef MIN
#define MIN(a, b)  ( ((a)<(b))? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a, b)  ( ((a)>(b))? (a) : (b) )
#endif
#ifndef ABSMIN
#define ABSMIN(a, b)  ( (ABS(a)<ABS(b))? (a) : (b) )
#endif
#ifndef ABSMAX
#define ABSMAX(a, b)  ( (ABS(a)>ABS(b))? (a) : (b) )
#endif
#ifndef CLAMP
#define CLAMP(a, b, x)  MIN( MAX(MIN(a,b), x), MAX(a,b))
#endif
#ifndef BETWEEN
#define BETWEEN(a, b, x)  (MIN(a,b)<(x) && (x)<MAX(a,b))
#endif
#ifndef BETWEENEQ
#define BETWEENEQ(a, b, x)  (MIN(a,b)<=(x) && (x)<=MAX(a,b))
#endif
#ifndef IROUND
#define IROUND(x)  int(x + .5)
#endif

struct MixOP
{
	int op;
	bool rescale;
	bool maskR, maskG, maskB;
	float brightness, contrast; //ratio
	/*MixOP() {
		op = OP_MAX;  rescale = true;
		maskR = maskG = maskB = true;
		brightness = 0; contrast = 1;
	}*/
};


//
// glsl is column first for multi-dimesio  
//
//RGB8 lookup_mix_litone(unsigned char mC[6], int N, int channel_index_On[6], int channels_On, RGBA16 mLut[6][256], int op, RGB8 mask)
//{
//
//#define R(k) (mLut[k][ mC[k] ].r)
//#define G(k) (mLut[k][ mC[k] ].g)
//#define B(k) (mLut[k][ mC[k] ].b)
//#define A(k) (mLut[k][ mC[k] ].a)
//	//[PM] can save 100 ms
//	//#define AR(k) (A(k)*R(k))
//	//#define AG(k) (A(k)*G(k))
//	//#define AB(k) (A(k)*B(k))
//#define AR(k) (R(k))
//#define AG(k) (G(k))
//#define AB(k) (B(k))
//	//end
//
//	//int N = mC.size();
//	//assert(N <= mLut.size());
//	//printf("lookup_mix:N:%d, mLut.size():%d\n", N, mLut.size()); 
//	uint o1, o2, o3;
//
//	int k0 = channel_index_On[0];
//
//	o1 = mLut[k0][mC[k0]].r;
//	o2 = mLut[k0][mC[k0]].g;
//	o3 = mLut[k0][mC[k0]].b;
//
//	//
//	if (N == 1)
//	{
//		uint clamp_max = 65535U;
//
//		/*o1 = CLAMP(0, clamp_max, o1)/255;
//		o2 = CLAMP(0, clamp_max, o2)/255;
//		o3 = CLAMP(0, clamp_max, o3)/255;*/
//
//		o1 = (uint)(CLAMP(0U, clamp_max, o1) >> 8U );
//		o2 = (uint)(CLAMP(0U, clamp_max, o2) >> 8U);
//		o3 = (uint)(CLAMP(0U, clamp_max, o3) >> 8U);
//
//		RGB8 oC;
//
//		oC.r = (unsigned char)o1;
//		oC.g = (unsigned char)o2;
//		oC.b = (unsigned char)o3;
//		oC.r &= mask.r;
//		oC.g &= mask.g;
//		oC.b &= mask.b;
//		return oC;
//
//	}
//	//end
//	if (op == OP_MAX)
//	{
//		//for (int k = 0; k<N; k++)
//		for (int k_on = 1; k_on<channels_On; k_on++)
//		{
//			int k = channel_index_On[k_on];
//			o1 = MAX(o1, AR(k));
//			o2 = MAX(o2, AG(k));
//			o3 = MAX(o3, AB(k));
//		}
//	}
//	else if (op == OP_SUM)
//	{
//		//for (int k = 0; k<N; k++)
//		for (int k_on = 1; k_on<channels_On; k_on++)
//		{
//			int k = channel_index_On[k_on];
//			o1 += AR(k);
//			o2 += AG(k);
//			o3 += AB(k);
//		}
//	}
//	else if (op == OP_MEAN)
//	{
//		//for (int k = 0; k<N; k++)
//		for (int k_on = 1; k_on<channels_On; k_on++)
//		{
//			int k = channel_index_On[k_on];
//			o1 += AR(k);
//			o2 += AG(k);
//			o3 += AB(k);
//		}
//		o1 /= N;
//		o2 /= N;
//		o3 /= N;
//	}
//	else if (op == OP_OIT)
//	{
//		float avg_1, avg_2, avg_3, avg_a1, avg_a2, avg_a3, avg_a;
//		avg_1 = avg_2 = avg_3 = avg_a1 = avg_a2 = avg_a3 = avg_a = 0;
//		//for (int k = 0; k<N; k++)
//		for (int k_on = 0; k_on<channels_On; k_on++)
//		{
//			int k = channel_index_On[k_on];
//
//			o1 = AR(k) >> 8;
//			o2 = AG(k) >> 8;
//			o3 = AB(k) >> 8;
//			//			avg_a1 += o1;
//			//			avg_a2 += o2;
//			//			avg_a3 += o3;
//			//			avg_1 += o1*o1;
//			//			avg_2 += o2*o2;
//			//			avg_3 += o3*o3;
//			float a = (float)(MAX(o1, MAX(o2, o3)));
//			//(o1+o2+o3)/3;
//			avg_a += a;
//			avg_1 += o1 *a;
//			avg_2 += o2 *a;
//			avg_3 += o3 *a;
//		}
//		//avg_alpha
//		//		avg_a1 /=N;
//		//		avg_a2 /=N;
//		//		avg_a3 /=N;
//		avg_a /= N;	avg_a1 = avg_a2 = avg_a3 = avg_a;
//
//		//avg_color
//		avg_1 /= N;
//		avg_2 /= N;
//		avg_3 /= N;
//		//(1-avg_alpha)^n
//		float bg_a1 = pow(1 - avg_a1, N);
//		float bg_a2 = pow(1 - avg_a2, N);
//		float bg_a3 = pow(1 - avg_a3, N);
//		float bg_color = 1;
//		//0.5;
//
//		// dst_color = avg_color * (1-(1-avg_alpha)^n) + bg_color * (1-avg_alpha)^n
//		o1 = (uint)(avg_1*(1 - bg_a1) + bg_color*bg_a1);
//		o2 = (uint)(avg_2*(1 - bg_a2) + bg_color*bg_a2);
//		o3 = (uint)(avg_3*(1 - bg_a3) + bg_color*bg_a3);
//
//		o1 = CLAMP(0U, 1U, o1);
//		o2 = CLAMP(0U, 1U, o2);
//		o3 = CLAMP(0U, 1U, o3);
//
//		RGB8 oC;
//		oC.r = (unsigned char)(((float)(o1))*255.0 + 0.5);
//		oC.g = (unsigned char)(((float)(o2))*255.0 + 0.5);
//		oC.b = (unsigned char)(((float)(o3))*255.0 + 0.5);
//		oC.r &= mask.r;
//		oC.g &= mask.g;
//		oC.b &= mask.b;
//		return oC;
//
//	}
//	// OP_INDEX ignored
//	uint clamp_max = 65536U;
//
//	/*o1 = CLAMP(0, clamp_max, o1)/255;
//	o2 = CLAMP(0, clamp_max, o2)/255;
//	o3 = CLAMP(0, clamp_max, o3)/255;*/
//
//	o1 = CLAMP(0U, clamp_max, o1) >> 8;
//	o2 = CLAMP(0U, clamp_max, o2) >> 8;
//	o3 = CLAMP(0U, clamp_max, o3) >> 8;
//
//	RGB8 oC;
//
//	oC.r = (unsigned char)o1;
//	oC.g = (unsigned char)o2;
//	oC.b = (unsigned char)o3;
//	oC.r &= mask.r;
//	oC.g &= mask.g;
//	oC.b &= mask.b;
//	return oC;
//
////#undef R(k)
////#undef G(k)
////#undef B(k)
////#undef A(k)
////#undef AR(k)
////#undef AG(k)
////#undef AB(k)
//}

#define HIST_RESCALE(k,C)\
					if(C<hist_min[k])\
					{\
						C = hist_min[k];\
					}\
					else if(C>hist_max[k])\
					{\
						C=hist_max[k];\
					}


//#define BRIGHTEN_TRANSFORM( C ) \
//					if (fc != 1 || fb != 0) \
//					{ \
//						unsigned int CC=C;\
//						C = C*(fc_128) + (fb_128*vrange[k]); \
//						C = C >> 7; \
//						if(C<(CC+vrange[k])) \
//						{ \
//							C = hist_min[k]; \
//						} \
//						else \
//						{ \
//							C = C - CC - vrange[k]; \
//							C = CLAMP(hist_min[k], hist_max[k], C); \
//						} \
//					}

//
// glsl is column first for multi-dimesio  
//
//RGB8 lookup_mix_litone(unsigned char mC[6], int N, int channel_index_On[6], int channels_On, RGBA16 mLut[6][256], int op, RGB8 mask)
//{
//
//#define R(k) (mLut[k][ mC[k] ].r)
//#define G(k) (mLut[k][ mC[k] ].g)
//#define B(k) (mLut[k][ mC[k] ].b)
//#define A(k) (mLut[k][ mC[k] ].a)
//	//[PM] can save 100 ms
//	//#define AR(k) (A(k)*R(k))
//	//#define AG(k) (A(k)*G(k))
//	//#define AB(k) (A(k)*B(k))
//#define AR(k) (R(k))
//#define AG(k) (G(k))
//#define AB(k) (B(k))
//	//end
//
//	//int N = mC.size();
//	//assert(N <= mLut.size());
//	//printf("lookup_mix:N:%d, mLut.size():%d\n", N, mLut.size()); 
//	uint o1, o2, o3;
//
//	int k0 = channel_index_On[0];
//
//	o1 = mLut[k0][mC[k0]].r;
//	o2 = mLut[k0][mC[k0]].g;
//	o3 = mLut[k0][mC[k0]].b;
//
//	//
//	if (N == 1)
//	{
//		uint clamp_max = 65535U;
//
//		/*o1 = CLAMP(0, clamp_max, o1)/255;
//		o2 = CLAMP(0, clamp_max, o2)/255;
//		o3 = CLAMP(0, clamp_max, o3)/255;*/
//
//		o1 = (uint)(CLAMP(0U, clamp_max, o1) >> 8U );
//		o2 = (uint)(CLAMP(0U, clamp_max, o2) >> 8U);
//		o3 = (uint)(CLAMP(0U, clamp_max, o3) >> 8U);
//
//		RGB8 oC;
//
//		oC.r = (unsigned char)o1;
//		oC.g = (unsigned char)o2;
//		oC.b = (unsigned char)o3;
//		oC.r &= mask.r;
//		oC.g &= mask.g;
//		oC.b &= mask.b;
//		return oC;
//
//	}
//	//end
//	if (op == OP_MAX)
//	{
//		//for (int k = 0; k<N; k++)
//		for (int k_on = 1; k_on<channels_On; k_on++)
//		{
//			int k = channel_index_On[k_on];
//			o1 = MAX(o1, AR(k));
//			o2 = MAX(o2, AG(k));
//			o3 = MAX(o3, AB(k));
//		}
//	}
//	else if (op == OP_SUM)
//	{
//		//for (int k = 0; k<N; k++)
//		for (int k_on = 1; k_on<channels_On; k_on++)
//		{
//			int k = channel_index_On[k_on];
//			o1 += AR(k);
//			o2 += AG(k);
//			o3 += AB(k);
//		}
//	}
//	else if (op == OP_MEAN)
//	{
//		//for (int k = 0; k<N; k++)
//		for (int k_on = 1; k_on<channels_On; k_on++)
//		{
//			int k = channel_index_On[k_on];
//			o1 += AR(k);
//			o2 += AG(k);
//			o3 += AB(k);
//		}
//		o1 /= N;
//		o2 /= N;
//		o3 /= N;
//	}
//	else if (op == OP_OIT)
//	{
//		float avg_1, avg_2, avg_3, avg_a1, avg_a2, avg_a3, avg_a;
//		avg_1 = avg_2 = avg_3 = avg_a1 = avg_a2 = avg_a3 = avg_a = 0;
//		//for (int k = 0; k<N; k++)
//		for (int k_on = 0; k_on<channels_On; k_on++)
//		{
//			int k = channel_index_On[k_on];
//
//			o1 = AR(k) >> 8;
//			o2 = AG(k) >> 8;
//			o3 = AB(k) >> 8;
//			//			avg_a1 += o1;
//			//			avg_a2 += o2;
//			//			avg_a3 += o3;
//			//			avg_1 += o1*o1;
//			//			avg_2 += o2*o2;
//			//			avg_3 += o3*o3;
//			float a = (float)(MAX(o1, MAX(o2, o3)));
//			//(o1+o2+o3)/3;
//			avg_a += a;
//			avg_1 += o1 *a;
//			avg_2 += o2 *a;
//			avg_3 += o3 *a;
//		}
//		//avg_alpha
//		//		avg_a1 /=N;
//		//		avg_a2 /=N;
//		//		avg_a3 /=N;
//		avg_a /= N;	avg_a1 = avg_a2 = avg_a3 = avg_a;
//
//		//avg_color
//		avg_1 /= N;
//		avg_2 /= N;
//		avg_3 /= N;
//		//(1-avg_alpha)^n
//		float bg_a1 = pow(1 - avg_a1, N);
//		float bg_a2 = pow(1 - avg_a2, N);
//		float bg_a3 = pow(1 - avg_a3, N);
//		float bg_color = 1;
//		//0.5;
//
//		// dst_color = avg_color * (1-(1-avg_alpha)^n) + bg_color * (1-avg_alpha)^n
//		o1 = (uint)(avg_1*(1 - bg_a1) + bg_color*bg_a1);
//		o2 = (uint)(avg_2*(1 - bg_a2) + bg_color*bg_a2);
//		o3 = (uint)(avg_3*(1 - bg_a3) + bg_color*bg_a3);
//
//		o1 = CLAMP(0U, 1U, o1);
//		o2 = CLAMP(0U, 1U, o2);
//		o3 = CLAMP(0U, 1U, o3);
//
//		RGB8 oC;
//		oC.r = (unsigned char)(((float)(o1))*255.0 + 0.5);
//		oC.g = (unsigned char)(((float)(o2))*255.0 + 0.5);
//		oC.b = (unsigned char)(((float)(o3))*255.0 + 0.5);
//		oC.r &= mask.r;
//		oC.g &= mask.g;
//		oC.b &= mask.b;
//		return oC;
//
//	}
//	// OP_INDEX ignored
//	uint clamp_max = 65536U;
//
//	/*o1 = CLAMP(0, clamp_max, o1)/255;
//	o2 = CLAMP(0, clamp_max, o2)/255;
//	o3 = CLAMP(0, clamp_max, o3)/255;*/
//
//	o1 = CLAMP(0U, clamp_max, o1) >> 8;
//	o2 = CLAMP(0U, clamp_max, o2) >> 8;
//	o3 = CLAMP(0U, clamp_max, o3) >> 8;
//
//	RGB8 oC;
//
//	oC.r = (unsigned char)o1;
//	oC.g = (unsigned char)o2;
//	oC.b = (unsigned char)o3;
//	oC.r &= mask.r;
//	oC.g &= mask.g;
//	oC.b &= mask.b;
//	return oC;
//
////#undef R(k)
////#undef G(k)
////#undef B(k)
////#undef A(k)
////#undef AR(k)
////#undef AG(k)
////#undef AB(k)
//}

RGBA16 testMultArray(RGBA16 mLut[3][2])
{
	RGBA16 rgb = mLut[1][1]; 
	/*rgb.r += (ushort)2;
	rgb.g += (ushort)2;
	rgb.b += (ushort)2;*/
	return rgb;

}

RGB8 test_lookup_mix_litone(unsigned char mC[2], RGBA16 mLut[3][2])
{

#define R(k) (mLut[ mC[k] ][k].r)
#define G(k) (mLut[ mC[k] ][k].g)
#define B(k) (mLut[ mC[k] ][k].b)
#define A(k) (mLut[ mC[k] ][k].a)
	//[PM] can save 100 ms
	//#define AR(k) (A(k)*R(k))
	//#define AG(k) (A(k)*G(k))
	//#define AB(k) (A(k)*B(k))
#define AR(k) (R(k))
#define AG(k) (G(k))
#define AB(k) (B(k))

	uint o1, o2, o3;

	int k0 = 1;

	o1 = R(k0);
	o2 = G(k0);
	o3 = B(k0);

	RGB8 oC;

	oC.r = (unsigned char)o1;
	oC.g = (unsigned char)o2;
	oC.b = (unsigned char)o3;
	/*oC.r &= mask.r;
	oC.g &= mask.g;
	oC.b &= mask.b;*/

	return oC;

}

uchar	mC[FILL_CHANNEL];
int		channel_index_On[FILL_CHANNEL];
int		channels_On = 0;

unsigned int hist_min[FILL_CHANNEL];
unsigned int hist_max[FILL_CHANNEL];
unsigned int v_range[FILL_CHANNEL];

RGBA16  channel_color[FILL_CHANNEL];
RGBA16	chColor;

void createHist()
{
	//construct the 
	for (int ch = 0; ch < FILL_CHANNEL; ch++)
	{
		//
		float row = (float)(ch) / (FILL_CHANNEL - 1.0);
		float colIndex = 0.0; 
		float col = 0.0 / (HistRangeCols - 1.0);

		bool channelOn = (bool)(texture(texHistRange, vec2(col, row)).r);
		channel_index_On[ch] = channelOn ? 1 : 0;

		if (channelOn)
		{
			channels_On++;
		}
		//hist min
		colIndex++;
		col = colIndex / (HistRangeCols - 1.0);
		hist_min[ch] = (unsigned int)((texture(texHistRange, vec2(col, row)).r));
		/*if (ch == 0)
		{
			rt = (unsigned short)(hist_min[ch]);
		}*/
		//hist max
		colIndex++;
		col = colIndex / (HistRangeCols - 1.0);
		hist_max[ch] = (unsigned int)((texture(texHistRange, vec2(col, row)).r));
		//v range
		colIndex++;
		col = colIndex / (HistRangeCols - 1.0);
		v_range[ch] = (unsigned int)((texture(texHistRange, vec2(col, row)).r));

		//channel color, r 
		colIndex++;
		col = colIndex / (HistRangeCols - 1.0);
		channel_color[ch].r = (unsigned short)((texture(texHistRange, vec2(col, row)).r));

		//channel color, g 
		colIndex++;
		col = colIndex / (HistRangeCols - 1.0);
		channel_color[ch].g = (unsigned short)((texture(texHistRange, vec2(col, row)).r));

		//channel color, b 
		colIndex++;
		col = colIndex / (HistRangeCols - 1.0);
		channel_color[ch].b = (unsigned short)((texture(texHistRange, vec2(col, row)).r));

		//channel color, a 
		colIndex++;
		col = colIndex / (HistRangeCols - 1.0);
		channel_color[ch].a = (unsigned short)((texture(texHistRange, vec2(col, row)).r));

	}
}

//////////////////////////////////////
void main()
{
	vec4 tex_uv = gl_TexCoord[0];
	
	createHist();

	//unsigned short rt =(unsigned short)( hist_min[0]);
	//unsigned short rt = (unsigned short)(channel_color[2].b);
	unsigned short rt = (unsigned short)(v_range[0]);
	unsigned short rtCheck = (unsigned short)(923U - 310U);
	/*int ch = 0; 

	float row = ((float)(ch)) / (FILL_CHANNEL - 1);
	float col = 1.0 / (HistRangeCols - 1.0);
*/
	//unsigned short rt = (unsigned short)(texture(texHistRange, vec2(col, row)).r);

	
	//vec2 textCoord = gl_TexCoord[0].xy; 
	//frag_color = gl_Color * oColor;

	//============= test volumn ==============

	//should rescale to [0,1] from [0, 65535] for RGB 
	/*float h0 = float(texture(channel0, tex_uv.xy).r) / 2048;
	float h1 = float(texture(channel1, tex_uv.xy).r) / 2048;
	float h2 = float(texture(channel2, tex_uv.xy).r) / 2048;

	vec4 vColor;
	
	vColor.r = vColor.g = vColor.b = 0;
	vColor.a = 255;

	vColor.r = h0; 


	frag_color = vColor;*/
	
	//============= test luts ==============
	/*vec4 lutsC1; 
	vec4 vColor = vec4( 1, 0, 0, 255);

	lutsC1.r = lutsC1.g = lutsC1.b = 0; 
	lutsC1.a = 255; 

	float ic = 2; 
	float nh0 = float(texture(texLuts, vec2(ic /255.0, 0.0 )).r);

	if (nh0 > 1024)
	{
		lutsC1.g = 255;
	}
	else if (nh0 > 512)
		lutsC1.g = 128;
	else if (nh0 > (ic*255-1) && nh0 < (ic * 255 + 1) )
	{
		lutsC1.b = 255;
	}
	else
		lutsC1.r = nh0;

	frag_color = lutsC1;
*/
	//============= test texHistRange ==============

	vec4 vColor;
	//vec4 vColor = vec4(1, 0, 0, 255);

	//vColor.r = vColor.g = vColor.b = 0;
	//vColor.a = 255;

	////float channel[][FILL_CHANNEL] = { { }}
	//float ic = 4; // 0: channelOn, 1: min, 2: max, 3: range, 4, 5, 6, 7: color
	//float ch = 2; 
	//unsigned short nh0 =(unsigned short)(texture(texHistRange, vec2(ic/7, ch/3)).r); //first channel
	//vColor.r = (float)nh0; 
	//
	//ic = 5;
	//nh0 = (unsigned short)(texture(texHistRange, vec2(ic / 7, ch / 3)).r);
	//vColor.g = (float)nh0;
	//
	//ic = 6;
	//nh0 = (unsigned short)(texture(texHistRange, vec2(ic / 7, ch / 3)).r);
	//vColor.b = (float)nh0;
	//
	//ic = 7;
	//nh0 = (unsigned short)(texture(texHistRange, vec2(ic / 7, ch / 3)).r);
	//vColor.a = (float)nh0;

	//int ch = 0; 

	vColor.r = vColor.g = vColor.b = 0;
	vColor.a = 255;

	/*if (rt > 512U )
	{
		vColor.r = 255;
	}
	else */
	if (rt == rtCheck )
	{
		vColor.b = 128;
	}
	else if (rt > 64U)
	{
		vColor.r = 64;
	}
	//else if (rt == 310)
	/*{
		vColor.b = 255;
	}*/
	else
	{
		vColor.g = 255;
	}
	/*vColor.r = (float)(channel_color[ch].r); 
	vColor.g = (float)(channel_color[ch].g);
	vColor.b = (float)(channel_color[ch].b);
	vColor.a = (float)(channel_color[ch].a);*/

	//float ic = 4; // 0: channelOn, 1: min, 2: max, 3: range, 4, 5, 6, 7: color
	//float ch = 2; 

	//bool channelOn = (bool)(texture(texHistRange, vec2(ic / 7, ch / 3)).r);
	
	//if (channelOn )
	//{
	//	vColor.r = 128;
	//}
	//else if (nh0 == 3545U)
	//{
	//	vColor.r = 255;
	//}
	//else if (nh0 > 1024U)
	//{
	//	vColor.g = 255;
	//}
	//else if (nh0 > 512U)
	//	vColor.g = 128;
	////else if (nh0 == 1)
	////{
	////	//vColor.r=vColor.g = 128;
	////	vColor.r = 255;
	////}
	//else if (nh0 > (923U - 1U) && nh0 < (923U + 1U))
	//{
	//	vColor.b = 255;
	//}
	//else
	//	vColor.b = nh0;

	frag_color = vColor;

	//[End] ============= test texHistRange ==============

	//
	MixOP mixOp; 

	bool bRescale = true;
	float fb = 1.0;
	float fc = 1.0;
	int op = OP_MAX;

	vec3 mask;
	mask.r = (mixOp.maskR) ? 255 : 0;
	mask.g = (mixOp.maskG) ? 255 : 0;
	mask.b = (mixOp.maskB) ? 255 : 0;
	unsigned int fb_128 = (unsigned int)(fb * 128.0) + 128U; //Normalized from float [-1,1) to unsigned int [0,256)
	unsigned int fc_128 = (unsigned int)(CLAMP(0.0, 128.0, fc) * 128.0) + 128;

	//lutsC1.r = nh0;


	/*lutsC1.r = float(texture(texLuts, vec2(vColor.r / 255 , channel.x)));
	lutsC1.g = float(texture(texLuts, vec2(vColor.g / 255, channel.y)));
	lutsC1.b = float(texture(texLuts, vec2(vColor.b / 255, channel.z)));*/
	/*lutsC1.a = 255.0f; */

	//vec4 vColor1 = vec4(texture(channel0, tex_uv.xy).r, 0, 0, 1.0f); //
	//vec4 vColor2 = vec4(texture(channel1, tex_uv.xy).r, 0, 0, 1.0f); //
	//vec4 vColor = vec4(h0, h1, h2, 1.0);


	if (format_bgra == 1)
	{
		vColor.rgba = vColor.bgra;
	}

	vec4 C1 = texture2D(colormap, vec2(vColor.r, channel.x));
	vec4 C2 = texture2D(colormap, vec2(vColor.g, channel.y));
	vec4 C3 = texture2D(colormap, vec2(vColor.b, channel.z));

	vec3 aC1 = C1.rgb * C1.a;
	vec3 aC2 = C2.rgb * C2.a;
	vec3 aC3 = C3.rgb * C3.a;
	float Amean = (C1.a + C2.a + C3.a) / 3.0;
	//= pow((C1.a * C2.a * C3.a), 1.0/3.0);
	float Amax = max(C1.a, max(C2.a, C3.a));

	vec4 oColor;
	if (blend_mode == 0) // cross-section
	{
		float Asec = Amean;
		oColor.rgb = (aC1 + aC2 + aC3);
		oColor.a = Asec;
	}
	else if (blend_mode == 1) // alpha-blending
	{
		float Ablend = Amean;
		//oColor.rgb = (C1.rgb + C2.rgb + C3.rgb)*Ablend;
		oColor.rgb = (aC1 + aC2 + aC3)*Ablend;
		oColor.a = Ablend;

		//		float Ablend = Amean;
		//		vec3 f3 = vec3(equal(vec3(Amax, Amax, Amax), vec3(C1.a, C2.a, C3.a)));
		//		oColor.r = dot(vec3(aC1.r, aC2.r, aC3.r), f3)*Ablend;
		//		oColor.g = dot(vec3(aC1.g, aC2.g, aC3.g), f3)*Ablend;
		//		oColor.b = dot(vec3(aC1.b, aC2.b, aC3.b), f3)*Ablend;
		//		oColor.a = Ablend;

		//		float Amip = Amean;
		//		oColor.rgb = max(aC1.rgb, max(aC2.rgb, aC3.rgb));
		//		oColor.a = Amip;
	}
	else // max-intensity
	{
		float Amip = Amean;
		oColor.rgb = max(aC1.rgb, max(aC2.rgb, aC3.rgb));
		oColor.a = Amip;

		//		float Asec = Amean;
		//		oColor.rgb = (aC1 + aC2 + aC3);
		//		oColor.a = Asec;
	}
	//gl_FragColor = oColor;
	//gl_FragColor = gl_Color * oColor; // modulated by color_proxy


	//frag_color = gl_Color * oColor; // modulated by color_proxy

	//RGBA16 mLut[3][2]; //2 row, 3 col
	RGBA16 rgb; 
	
	// mLut[1][2] = 5; 
	//for (uint i = 0U; i < 2U; i++) //row
	//{
	//	for (uint j = 0U; j < 3U; j++) //col
	//	{
	//		rgb.r = rgb.g = rgb.b = (unsigned short) (i * 3U + j);
	//		rgb.a = (unsigned short)0;

	//		mLut[j][i] = rgb;
	//	}
	//}

	
	/*
	RGBA16 rgb2 = testMultArray(mLut); 
	if (rgb2.r == 7U)
	{
		oColor = vec4(255, 0, 0, 255); 
	}
	else if (rgb2.r == 4U)
	{
		oColor = vec4(0, 255, 0, 255);
	}
	else 
	{
		oColor = vec4(0, 0, 255, 255);
	}*/
//#define uchar unsigned char
//
//	uchar mC[2];
//	mC[0] = (uchar)1;
//	mC[1] = (uchar)1;
	//mC[2] = 2;


	//RGB8 rgb3 = test_lookup_mix_litone(mC, pLuts);
	/*RGB8 rgb3;
	rgb3.r = (uchar)(pLutsTest[0].r);*/
	/*if (rgb3.r == 5U)
	{
		oColor = vec4(255, 0, 0, 255);
	}
	else if (rgb3.r == 4U)
	{
		oColor = vec4(0, 255, 0, 255);
	}
	else if (rgb3.r == 3U)
	{
		oColor = vec4(0, 0, 255, 255);
	}
	else if (rgb3.r == 2U)
	{
		oColor = vec4(128, 0, 0, 255);
	}
	else if (rgb3.r == 1U)
	{
		oColor = vec4(0, 128, 0, 255);
	}
	else
	{
		oColor = vec4(0, 0, 128, 255);
	}*/
	/*oColor = vec4(pLutsTest[0], 0, 0, 255);
	frag_color = oColor;*/

	
}