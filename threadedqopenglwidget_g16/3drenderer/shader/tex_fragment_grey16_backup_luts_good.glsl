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

vec4 tex_uv; // texture coordination

struct RGBA16 {
	unsigned short r, g, b, a;
};
struct RGB8 {
	unsigned char r, g, b;
};

#define HistRangeCols 8 

struct HistRange16 {
	unsigned short channelOn;  //1: enable, 0: disable
	unsigned short hist_min;
	unsigned short hist_max;
	unsigned short vrange;
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



unsigned int	mC[FILL_CHANNEL];
unsigned int	mCRaw[FILL_CHANNEL];
int		channel_index_On[FILL_CHANNEL];
int		channel_flag_On[FILL_CHANNEL];

int		channels_On = 0;

unsigned int hist_min[FILL_CHANNEL];
unsigned int hist_max[FILL_CHANNEL];
unsigned int vrange[FILL_CHANNEL];

RGBA16  channel_color[FILL_CHANNEL];
RGBA16	chColor;

MixOP	mixOp;

#define HIST_RESCALE(k,C)\
					if(C<hist_min[k])\
					{\
						C = hist_min[k];\
					}\
					else if(C>hist_max[k])\
					{\
						C=hist_max[k];\
					}


#define BRIGHTEN_TRANSFORM( C ) \
					if (fc != 1 || fb != 0) \
					{								\
						C = C*(fc)+(fb*(float)vrange[k]);		\
						C = CLAMP((float)(hist_min[k]), (float)(hist_max[k]), C);	\
					}	\
					



void createHist()
{
	//construct the 
	for (int ch = 0; ch < FILL_CHANNEL; ch++)
	{
		//
		float row = (float)(ch) / (FILL_CHANNEL - 1.0);
		float colIndex = 0.0; 
		float col = 0.0 / (HistRangeCols - 1.0);

		bool isOn = (bool)(texture(texHistRange, vec2(col, row)).r);
		channel_flag_On[ch] = isOn ? 1 : 0;

		if (isOn)
		{
			channel_index_On[channels_On] = ch;
			channels_On++;
		}
		//hist min
		colIndex++;
		col = colIndex / (HistRangeCols - 1.0);
		hist_min[ch] = (unsigned int)((texture(texHistRange, vec2(col, row)).r));
		
		//hist max
		colIndex++;
		col = colIndex / (HistRangeCols - 1.0);
		hist_max[ch] = (unsigned int)((texture(texHistRange, vec2(col, row)).r));
		//v range
		colIndex++;
		col = colIndex / (HistRangeCols - 1.0);
		vrange[ch] = (unsigned int)((texture(texHistRange, vec2(col, row)).r));

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

		// 
		mCRaw[ch] = 0U;

		if (isOn)
		{
			switch (ch)
			{
				case 0: mCRaw[ch] = (unsigned int)(texture(channel0, tex_uv.xy).r); break;
				case 1: mCRaw[ch] = (unsigned int)(texture(channel1, tex_uv.xy).r); break;
				case 2: mCRaw[ch] = (unsigned int)(texture(channel2, tex_uv.xy).r); break;
				case 3: mCRaw[ch] = (unsigned int)(texture(channel3, tex_uv.xy).r); break;
			}
		}
	}
}




//
// glsl is column first for multi-dimesio  
//
RGB8 lookup_mix_litone( int op, RGB8 mask)
{
	//
	// get the mC[k] from the lookup table
	//
	vec4 mLut[FILL_CHANNEL];

	int k; 
	vec2 pos; 

#define LOOK_UP_CHANNEL(k) \
	pos.x = (float)(mC[k]) / 255.0;		\
	pos.y = (float)(k) / (FILL_CHANNEL - 1); \
	mLut[k].r = (float)(texture(texLuts, pos.xy).r);	\
	mLut[k].g = (float)(texture(texLuts, pos.xy).g);	\
	mLut[k].b = (float)(texture(texLuts, pos.xy).b);	\
	mLut[k].a = (float)(texture(texLuts, pos.xy).a);			\

	//vec4 vColor1 = vec4(texture(channel0, tex_uv.xy).r, 0, 0, 1.0f); //
	//vec4 vColor2 = vec4(texture(channel1, tex_uv.xy).r, 0, 0, 1.0f); //
	//vec4 vColor = vec4(h0, h1, h2, 1.0);

#define R(k) ( (uint)(mLut[k].r) )
#define G(k) ( (uint)(mLut[k].g) )
#define B(k) ( (uint)(mLut[k].b) )
#define A(k) ( (uint)(mLut[k].a) )
	//[PM] can save 100 ms
	//#define AR(k) (A(k)*R(k))
	//#define AG(k) (A(k)*G(k))
	//#define AB(k) (A(k)*B(k))
#define AR(k) (R(k))
#define AG(k) (G(k))
#define AB(k) (B(k))

	//end

	//int N = mC.size();
	//assert(N <= mLut.size());
	//printf("lookup_mix:N:%d, mLut.size():%d\n", N, mLut.size()); 
	uint o1, o2, o3;

	int k0 = channel_index_On[0];

	//mC[k] is rescale to[0, 255] - unsigned int
	//LOOK_UP_CHANNEL(k0); 
	k = k0; 

	//mC[k] is rescale to[0, 255] - unsigned int
	//pos.x = ((float)(mC[k])) / 255.0f;	// texture [0, 1] - mC [0, 255]	
	//pos.y = ((float)(k)) / (FILL_CHANNEL - 1.0f); 



	//==== Test MC[k] ====
	/*vec4 vColor;
	vColor.r = vColor.g = vColor.b = 0.0f;
	vColor.a = 1.0f;
	vColor.r = ((float)(mC[1])) / 255.0f;
	frag_color = vColor;*/
	//[End] MC[k]


	//==== xy.x ====
	/*pos.x = ((float)(mC[k])) / 255.0f;
	vec4 vColor;
	vColor.r = vColor.g = vColor.b = 0.0f;
	vColor.a = 1.0f;
	vColor.r = pos.x;
	frag_color = vColor;*/
	//[End] MC[k]

	/*colIndex++;
	float row = ((float)(k)) / ((FILL_CHANNEL - 1.0f))
	channel_color[ch].g = (unsigned short)((texture(texHistRange, vec2(col, row)).r));*/

	//mC[k] = 254U; 

	
	//float col = ((float)(mC[k])) / (4.0*256.0f -1.0 );	// texture [0, 1] - mC [0, 255]	
	//float row = ((float)(0)) / ((FILL_CHANNEL - 1.0f));

	//unsigned short lutr = (unsigned short)((texture(texLuts, vec2(col, row)).r));

	//col = ((float)(mC[k]+1U)) / (4.0*256.0f - 1.0);
	//lutg = (unsigned short)((texture(texLuts, vec2(col, row)).r));

	pos.x = ((float)(mC[k])) / 255.0f;	// texture [0, 1] - mC [0, 255]	
	pos.y = ((float)(k)) / (FILL_CHANNEL - 1.0f); 

	float lutr = (float)(texture(texLuts, pos).r);	 //[0, 65536]
	float lutg = (float)(texture(texLuts, pos).g);
	float lutb = (float)(texture(texLuts, pos).b);
	float luta = (float)(texture(texLuts, pos).a);


	o1 = R(k0);
	o2 = G(k0);
	o3 = B(k0);

	//===========test lutr
	vec4 vColor;
	vColor.r = lutr / 65536; 
	vColor.g = lutg / 65536;
	vColor.b = lutb / 65536;
	vColor.a = luta / 255;
	//vColor.g = vColor.b = 0U;
	//vColor.a = 255U;
	
	//float lutrChk = (float)(mC[k]) * 255;
	////if (lutr >(lutrChk -1) && lutr > (lutrChk + 1) )
	//lutr = lutb;
	//if (lutr == lutrChk )
	//{
	//	vColor.r = 255U;
	//}
	//else if (lutr > 64)
	//{
	//	vColor.b = 255;
	//}
	//else if (lutr > 32)
	//{
	//	vColor.b = 16U;
	//}
	//else if (lutr == 1)
	//{
	//	vColor.b = 128U;
	//}
	//else if (lutr == 0)
	//{
	//	vColor.g = 16U;
	//}
	///*else if (lutr >4U )
	//{
	//	vColor.g = 128U;
	//}*/
	
	frag_color = vColor;

	//test lutr

	int N = channels_On;
	//
	if (N == 1)
	{
		uint clamp_max = 65535U;

		/*o1 = CLAMP(0, clamp_max, o1)/255;
		o2 = CLAMP(0, clamp_max, o2)/255;
		o3 = CLAMP(0, clamp_max, o3)/255;*/

		o1 = (uint)(CLAMP(0U, clamp_max, o1) >> 8U );
		o2 = (uint)(CLAMP(0U, clamp_max, o2) >> 8U);
		o3 = (uint)(CLAMP(0U, clamp_max, o3) >> 8U);

		RGB8 oC;

		oC.r = (unsigned char)o1;
		oC.g = (unsigned char)o2;
		oC.b = (unsigned char)o3;
		oC.r &= mask.r;
		oC.g &= mask.g;
		oC.b &= mask.b;
		return oC;

	}
	//end
	if (op == OP_MAX)
	{
		//for (int k = 0; k<N; k++)
		for (int k_on = 1; k_on<channels_On; k_on++)
		{
			k = channel_index_On[k_on];
			LOOK_UP_CHANNEL(k);
			o1 = MAX(o1, AR(k));
			o2 = MAX(o2, AG(k));
			o3 = MAX(o3, AB(k));
		}
	}
	else if (op == OP_SUM)
	{
		//for (int k = 0; k<N; k++)
		for (int k_on = 1; k_on<channels_On; k_on++)
		{
			int k = channel_index_On[k_on];
			LOOK_UP_CHANNEL(k);
			o1 += AR(k);
			o2 += AG(k);
			o3 += AB(k);
		}
	}
	else if (op == OP_MEAN)
	{
		//for (int k = 0; k<N; k++)
		for (int k_on = 1; k_on<channels_On; k_on++)
		{
			int k = channel_index_On[k_on];
			LOOK_UP_CHANNEL(k);
			o1 += AR(k);
			o2 += AG(k);
			o3 += AB(k);
		}
		o1 /= N;
		o2 /= N;
		o3 /= N;
	}
	else if (op == OP_OIT)
	{
		float avg_1, avg_2, avg_3, avg_a1, avg_a2, avg_a3, avg_a;
		avg_1 = avg_2 = avg_3 = avg_a1 = avg_a2 = avg_a3 = avg_a = 0;
		//for (int k = 0; k<N; k++)
		for (int k_on = 0; k_on<channels_On; k_on++)
		{
			int k = channel_index_On[k_on];
			LOOK_UP_CHANNEL(k);
			o1 = AR(k) >> 8;
			o2 = AG(k) >> 8;
			o3 = AB(k) >> 8;
			//			avg_a1 += o1;
			//			avg_a2 += o2;
			//			avg_a3 += o3;
			//			avg_1 += o1*o1;
			//			avg_2 += o2*o2;
			//			avg_3 += o3*o3;
			float a = (float)(MAX(o1, MAX(o2, o3)));
			//(o1+o2+o3)/3;
			avg_a += a;
			avg_1 += o1 *a;
			avg_2 += o2 *a;
			avg_3 += o3 *a;
		}
		//avg_alpha
		//		avg_a1 /=N;
		//		avg_a2 /=N;
		//		avg_a3 /=N;
		avg_a /= N;	avg_a1 = avg_a2 = avg_a3 = avg_a;

		//avg_color
		avg_1 /= N;
		avg_2 /= N;
		avg_3 /= N;
		//(1-avg_alpha)^n
		float bg_a1 = pow(1 - avg_a1, N);
		float bg_a2 = pow(1 - avg_a2, N);
		float bg_a3 = pow(1 - avg_a3, N);
		float bg_color = 1;
		//0.5;

		// dst_color = avg_color * (1-(1-avg_alpha)^n) + bg_color * (1-avg_alpha)^n
		o1 = (uint)(avg_1*(1 - bg_a1) + bg_color*bg_a1);
		o2 = (uint)(avg_2*(1 - bg_a2) + bg_color*bg_a2);
		o3 = (uint)(avg_3*(1 - bg_a3) + bg_color*bg_a3);

		o1 = CLAMP(0U, 1U, o1);
		o2 = CLAMP(0U, 1U, o2);
		o3 = CLAMP(0U, 1U, o3);

		RGB8 oC;
		oC.r = (unsigned char)(((float)(o1))*255.0 + 0.5);
		oC.g = (unsigned char)(((float)(o2))*255.0 + 0.5);
		oC.b = (unsigned char)(((float)(o3))*255.0 + 0.5);
		oC.r &= mask.r;
		oC.g &= mask.g;
		oC.b &= mask.b;
		return oC;

	}
	// OP_INDEX ignored
	uint clamp_max = 65536U;

	/*o1 = CLAMP(0, clamp_max, o1)/255;
	o2 = CLAMP(0, clamp_max, o2)/255;
	o3 = CLAMP(0, clamp_max, o3)/255;*/

	o1 = CLAMP(0U, clamp_max, o1) >> 8;
	o2 = CLAMP(0U, clamp_max, o2) >> 8;
	o3 = CLAMP(0U, clamp_max, o3) >> 8;

	RGB8 oC;

	oC.r = (unsigned char)o1;
	oC.g = (unsigned char)o2;
	oC.b = (unsigned char)o3;
	oC.r &= mask.r;
	oC.g &= mask.g;
	oC.b &= mask.b;
	return oC;

//#undef R(k)
//#undef G(k)
//#undef B(k)
//#undef A(k)
//#undef AR(k)
//#undef AG(k)
//#undef AB(k)
//#undef LOOK_UP_CHANNEL(k)
}

//////////////////////////////////////
void main()
{
	tex_uv = gl_TexCoord[0];
	
	createHist();
	//test hist
	vec4 vColor;
	/*int ch = 2; 
	vColor.r = (float)(channel_color[ch].r);
	vColor.g = (float)(channel_color[ch].g);
	vColor.b = (float)(channel_color[ch].b);
	vColor.a = (float)(channel_color[ch].a);
	frag_color = vColor;
	*/
	//
	//MixOP mixOp;

	mixOp.rescale = true;
	mixOp.op = OP_MAX;
	mixOp.maskR = mixOp.maskG = mixOp.maskB = true;
	mixOp.brightness = 0;
	mixOp.contrast = 1;


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
		HIST_RESCALE(k, C);  //clamp to [hist_min, hist_max] 
		CC = (float)C; 
		BRIGHTEN_TRANSFORM(CC); //contrast, brightness, still in [hist_min, hist_max]
		//rewrite to the original
		C = (unsigned int)CC;
		mC[k] = (!bRescale) ? C : ((C - hist_min[k])*255 / vrange[k]);  // rescale to [0, 255]
	} //end of K

	RGB8 mixC = lookup_mix_litone(op, mask);

	/*unsigned int t = (rgba.r + rgba.g + rgba.b) / 3;
	rgba.a = (unsigned char)t;
*/
	//should normalized to [0£¬1]
	/*vec4 vColor;
	vColor.r = vColor.g = vColor.b = 0.0f;
	vColor.a = 1.0f;*/

	////vColor.r = ((float)(mC[1])) / 255.0f;
	//vColor.r = ((float)(mixC.r)) / 255.0f;

	///*vColor.r = vColor.g = vColor.b = 0U;
	//vColor.a = 255U;
	//vColor.r = (float)(mC[1]);*/

	////vColor.r = ((float)(mC[1])) / 255.0f;


	//frag_color = vColor;

	//RGB8 o = lookup_mix_litone(mC, size4, channel_index_On, channels_On, p_luts16, op, mask);
	//rgba.r = o.r;
	//rgba.g = o.g;
	//rgba.b = o.b;
	////float t = (0.f + rgba.r + rgba.g + rgba.b) / 3.0;
	//unsigned int t = (rgba.r + rgba.g + rgba.b) / 3;
	//rgba.a = (unsigned char)t;
	//rgbaBuf[oxOffset++] = rgba;

	

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
	
	//gl_FragColor = gl_Color * oColor; // modulated by color_proxy


	//frag_color = gl_Color * oColor; // modulated by color_proxy

	

	
}