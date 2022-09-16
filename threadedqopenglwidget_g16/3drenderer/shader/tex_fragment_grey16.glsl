#version 330 compatibility
//enable the pointer data load
//#extension GL_NV_shader_buffer_load : enable

//in vec4 tex_uv;
out vec4 frag_color;

//volume channel, support maximum 6 channels 
uniform highp usampler2D channel0;
uniform highp usampler2D channel1;
uniform highp usampler2D channel2;
//uniform highp usampler2D channel3;
//uniform highp usampler2D channel4;
//uniform highp usampler2D channel5;

uniform highp usampler2D texLuts;  //grey 16 rgba, channels*256
uniform highp usampler2D texHistRange; //channels * [channelOn, min, max, vrange, color]
//
//uniform sampler2D colormap;
//
//uniform vec3 channel;
//uniform int blend_mode;
//uniform int format_bgra;

vec4 tex_uv; // texture coordination

//struct RGBA16 {
//	unsigned short r, g, b, a;
//};
//struct RGB8 {
//	unsigned char r, g, b;
//};

#define HistRangeCols 8 

//struct HistRange16 {
//	unsigned short channelOn;  //1: enable, 0: disable
//	unsigned short hist_min;
//	unsigned short hist_max;
//	unsigned short vrange;
//	RGBA16 color;
//};
//
//#define ushort unsigned short
//#define uchar unsigned char

//uniform int pLutsTest[6];

//
//uniform RGBA16 pLuts[3][2];

//#define FILL_CHANNEL 3
//
//
//
//// lookup and mix multi-channel to RGBA8
//#define OP_MAX	0
//#define OP_SUM	1
//#define OP_MEAN	2
//#define OP_OIT	3  //Order Independent Transparency
//#define OP_INDEX	-1


//struct MixOP
//{
//	int op;
//	bool rescale;
//	bool maskR, maskG, maskB;
//	float brightness, contrast; //ratio
//	/*MixOP() {
//		op = OP_MAX;  rescale = true;
//		maskR = maskG = maskB = true;
//		brightness = 0; contrast = 1;
//	}*/
//};

//
//unsigned int	mCRaw[FILL_CHANNEL];  //raw data for each channel
//unsigned int	mC[FILL_CHANNEL];	  //data after rescale and brighteness  	
//
//int		channel_index_On[FILL_CHANNEL]; //which channels are on 
//int		channel_flag_On[FILL_CHANNEL];
//
//int		channels_On = 0;
//
//unsigned int hist_min[FILL_CHANNEL];
//unsigned int hist_max[FILL_CHANNEL];
//unsigned int vrange[FILL_CHANNEL];
//
//RGBA16  channel_color[FILL_CHANNEL];
//RGBA16	chColor;
//
//MixOP	mixOp;

//#define HIST_RESCALE(k,C)\
//					if(C<hist_min[k])\
//					{\
//						C = hist_min[k];\
//					}\
//					else if(C>hist_max[k])\
//					{\
//						C=hist_max[k];\
//					} \
//
//
//#define BRIGHTEN_TRANSFORM( C ) \
//					if (fc != 1 || fb != 0) \
//					{								\
//						C = C*(fc)+(fb*(float)vrange[k]);		\
//						C = CLAMP((float)(hist_min[k]), (float)(hist_max[k]), C);	\
//					}	\
//					
//
//
//
//void createHist()
//{
//	//construct the 
//	float FILL_CHANNEL_FACTOR = (FILL_CHANNEL - 1.0);
//	float HistRangeCols_FACTOR = (HistRangeCols - 1.0);
//
//	for (int ch = 0; ch < FILL_CHANNEL; ch++)
//	{
//		//
//		//float row = (float)(ch) / (FILL_CHANNEL - 1.0);
//		float row = (float)(ch) / FILL_CHANNEL_FACTOR;
//		float colIndex = 0.0; 
//		//float col = 0.0 / (HistRangeCols - 1.0);
//		float col = 0.0 / HistRangeCols_FACTOR;
//
//		bool isOn = (bool)(texture(texHistRange, vec2(col, row)).r);
//		//channel_flag_On[ch] = isOn ? 1 : 0;
//		channel_flag_On[ch] = 1; 
//		//if (isOn)
//		{
//			channel_index_On[channels_On] = ch;
//			channels_On++;
//		}
//		//hist min
//		colIndex++;
//		/*col = colIndex / (HistRangeCols - 1.0);*/
//		col = colIndex / HistRangeCols_FACTOR;
//		hist_min[ch] = (unsigned int)((texture(texHistRange, vec2(col, row)).r));
//		
//		//hist max
//		colIndex++;
//		col = colIndex / HistRangeCols_FACTOR;
//		hist_max[ch] = (unsigned int)((texture(texHistRange, vec2(col, row)).r));
//		//v range
//		colIndex++;
//		col = colIndex / HistRangeCols_FACTOR;
//		vrange[ch] = (unsigned int)((texture(texHistRange, vec2(col, row)).r));
//
//		//channel color, r 
//		colIndex++;
//		col = colIndex / HistRangeCols_FACTOR;
//		channel_color[ch].r = (unsigned short)((texture(texHistRange, vec2(col, row)).r));
//
//		//channel color, g 
//		colIndex++;
//		col = colIndex / HistRangeCols_FACTOR;
//		channel_color[ch].g = (unsigned short)((texture(texHistRange, vec2(col, row)).r));
//
//		//channel color, b 
//		colIndex++;
//		col = colIndex / HistRangeCols_FACTOR;
//		channel_color[ch].b = (unsigned short)((texture(texHistRange, vec2(col, row)).r));
//
//		//channel color, a 
//		colIndex++;
//		col = colIndex / HistRangeCols_FACTOR;
//		channel_color[ch].a = (unsigned short)((texture(texHistRange, vec2(col, row)).r));
//
//		// 
//		mCRaw[ch] = 0U;
//
//		//if (isOn)
//		/*{
//			switch (ch)
//			{
//				case 0: mCRaw[ch] = (unsigned int)(texture(channel0, tex_uv.xy).r); break;
//				case 1: mCRaw[ch] = (unsigned int)(texture(channel1, tex_uv.xy).r); break;
//				case 2: mCRaw[ch] = (unsigned int)(texture(channel2, tex_uv.xy).r); break;
//				case 3: mCRaw[ch] = (unsigned int)(texture(channel3, tex_uv.xy).r); break;
//			
//			}
//		}*/
//	}
//
//	mCRaw[0] = (unsigned int)(texture(channel0, tex_uv.xy).r); 
//	mCRaw[1] = (unsigned int)(texture(channel1, tex_uv.xy).r); 
//	mCRaw[2] = (unsigned int)(texture(channel2, tex_uv.xy).r); 
//
//}
//
//
////#define LOOKUP_CHANNEL( k ) \
////	pos.x = ((float)(mC[k])) / (255.0f);	\ 	
////	pos.y = ((float)(k)) / (FILL_CHANNEL-(1.0f));	\
////	mLut[k].r = (float)(texture(texLuts, pos).r);	\
////	mLut[k].g = (float)(texture(texLuts, pos).g);	\
////	mLut[k].b = (float)(texture(texLuts, pos).b);	\
////	mLut[k].a = (float)(texture(texLuts, pos).a);			\
////
////#define LOOKUP_CHANNEL( k ) \
////	pos.x = ((float)(mC[k])) / (255.0f);	\ 	
////pos.y = ((float)(k)) / (FILL_CHANNEL - (1.0f));
//
//
////[PM] can save 100 ms
////#define AR(k) (A(k)*R(k))
////#define AG(k) (A(k)*G(k))
////#define AB(k) (A(k)*B(k))
//#define AR(k) (R(k))
//#define AG(k) (G(k))
//#define AB(k) (B(k))
//
////
//// glsl is column first for multi-dimesio  
////
//RGB8 lookup_mix_litone( int op, RGB8 mask)
//{
////
//// get the mC[k] from the lookup table
////
//#define LOOKUP_CHANNEL(k)\
//					pos.x = ((float)(mC[k])) / (255.0f); \
//					pos.y = ((float)(k)) / (FILL_CHANNEL - (1.0f)); \
//					mLut[k] = texture(texLuts, pos);	\
//
//
//#define R(k) ( (uint)(mLut[k].r) )
//#define G(k) ( (uint)(mLut[k].g) )
//#define B(k) ( (uint)(mLut[k].b) )
//#define A(k) ( (uint)(mLut[k].a) )
//
//	vec4 mLut[FILL_CHANNEL];
//
//	 
//	vec2 pos; 
//	
//	uint o1, o2, o3;
//
//	int k, k0; 
//	k = k0 = channel_index_On[0];
//	//mC[k] is rescale to[0, 255] - unsigned int
//	LOOKUP_CHANNEL(k0); 
//	
//	o1 = R(k0);
//	o2 = G(k0);
//	o3 = B(k0);
//
//	//===========test lutr
//	vec4 vColor;
//	
//	/*
//	int k = k0;
//	vColor.r = mLut[k].r / 65535;
//	vColor.g = mLut[k].g / 65535;
//	vColor.b = mLut[k].b / 65535;
//	vColor.a = mLut[k].a / 255;*/
//	/*vColor.r = ((float)o1) / 65535;
//	vColor.g = ((float)o2) / 65535;
//	vColor.b = ((float)o3) / 65535;
//	vColor.a = mLut[k0].a / 255;
//
//	frag_color = vColor;*/
//
//	//test lutr
//
//	int N = channels_On;
//	//
//	//N = 1; 
//
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
//		
//		RGB8 oC;
//
//		oC.r = (unsigned char)o1;
//		oC.g = (unsigned char)o2;
//		oC.b = (unsigned char)o3;
//		oC.r &= mask.r;
//		oC.g &= mask.g;
//		oC.b &= mask.b;
//
//		//vColor.r = ((float)(oC.r)) / 255;
//		//vColor.g = ((float)(oC.g)) /255;
//		//vColor.b = ((float)(oC.b)) / 255;
//		////vColor.a = ((float)(oC.a));
//		//vColor.a = 1.0;
//
//		//frag_color = vColor;
//
//		return oC;
//
//	}
//	//end
//	if (op == OP_MAX)
//	{
//		//for (int k = 0; k<N; k++)
//		for (int k_on = 1; k_on<channels_On; k_on++)
//		{
//			k = channel_index_On[k_on];
//			LOOKUP_CHANNEL(k);
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
//			k = channel_index_On[k_on];
//			LOOKUP_CHANNEL(k);
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
//			k = channel_index_On[k_on];
//			LOOKUP_CHANNEL(k);
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
//			k = channel_index_On[k_on];
//			LOOKUP_CHANNEL(k);
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
//
//	//vColor.r = ((float)(oC.r)) / 255;
//	//vColor.g = ((float)(oC.g)) /255;
//	//vColor.b = ((float)(oC.b)) / 255;
//	////vColor.a = ((float)(oC.a));
//	//vColor.a = 1.0;
//
//	//frag_color = vColor;
//
//	return oC;
//
////#undef R(k)
////#undef G(k)
////#undef B(k)
////#undef A(k)
////#undef AR(k)
////#undef AG(k)
////#undef AB(k)
////#undef LOOKUP_CHANNEL(k)
//}

//////////////////////////////////////
void main()
{
	tex_uv = gl_TexCoord[0];
	
	//createHist();

	//float FILL_CHANNEL_FACTOR = (FILL_CHANNEL - 1.0);
	//float HistRangeCols_FACTOR = (HistRangeCols - 1.0);
	//int ch = 0; 
	////for (int ch = 0; ch < FILL_CHANNEL; ch++)
	//{
	//	//
	//	//float row = (float)(ch) / (FILL_CHANNEL - 1.0);
	//	float row = (float)(ch) / FILL_CHANNEL_FACTOR;
	//	float colIndex = 0.0;
	//	//float col = 0.0 / (HistRangeCols - 1.0);
	//	float col = 0.0 / HistRangeCols_FACTOR;

	//	bool isOn = (bool)(texture(texHistRange, vec2(col, row)).r);
	//	//channel_flag_On[ch] = isOn ? 1 : 0;
	//	channel_flag_On[ch] = 1;
	//	//if (isOn)
	//	{
	//		channel_index_On[channels_On] = ch;
	//		channels_On++;
	//	}
	//	//hist min
	//	colIndex++;
	//	/*col = colIndex / (HistRangeCols - 1.0);*/
	//	col = colIndex / HistRangeCols_FACTOR;
	//	hist_min[ch] = (unsigned int)((texture(texHistRange, vec2(col, row)).r));

	//	//hist max
	//	colIndex++;
	//	col = colIndex / HistRangeCols_FACTOR;
	//	hist_max[ch] = (unsigned int)((texture(texHistRange, vec2(col, row)).r));
	//	//v range
	//	colIndex++;
	//	col = colIndex / HistRangeCols_FACTOR;
	//	vrange[ch] = (unsigned int)((texture(texHistRange, vec2(col, row)).r));

	//	////channel color, r 
	//	//colIndex++;
	//	//col = colIndex / HistRangeCols_FACTOR;
	//	//channel_color[ch].r = (unsigned short)((texture(texHistRange, vec2(col, row)).r));

	//	////channel color, g 
	//	//colIndex++;
	//	//col = colIndex / HistRangeCols_FACTOR;
	//	//channel_color[ch].g = (unsigned short)((texture(texHistRange, vec2(col, row)).r));

	//	////channel color, b 
	//	//colIndex++;
	//	//col = colIndex / HistRangeCols_FACTOR;
	//	//channel_color[ch].b = (unsigned short)((texture(texHistRange, vec2(col, row)).r));

	//	////channel color, a 
	//	//colIndex++;
	//	//col = colIndex / HistRangeCols_FACTOR;
	//	//channel_color[ch].a = (unsigned short)((texture(texHistRange, vec2(col, row)).r));

	//	// 
	//	//mCRaw[ch] = 0U;

	//	//if (isOn)
	//	/*{
	//	switch (ch)
	//	{
	//	case 0: mCRaw[ch] = (unsigned int)(texture(channel0, tex_uv.xy).r); break;
	//	case 1: mCRaw[ch] = (unsigned int)(texture(channel1, tex_uv.xy).r); break;
	//	case 2: mCRaw[ch] = (unsigned int)(texture(channel2, tex_uv.xy).r); break;
	//	case 3: mCRaw[ch] = (unsigned int)(texture(channel3, tex_uv.xy).r); break;

	//	}
	//	}*/
	//}

	//mCRaw[0] = (unsigned int)(texture(channel0, tex_uv.xy).r);
	//mCRaw[1] = (unsigned int)(texture(channel1, tex_uv.xy).r);
	//mCRaw[2] = (unsigned int)(texture(channel2, tex_uv.xy).r);

	////test hist
	vec4 vColor;
	///*int ch = 0; 
	//vColor.r = (float)(channel_color[ch].r);
	//vColor.g = (float)(channel_color[ch].g);
	//vColor.b = (float)(channel_color[ch].b);
	//vColor.a = (float)(channel_color[ch].a);
	//frag_color = vColor;*/

	////[Test]
	//vColor.r = ((float)(mCRaw[0])) / 1024;
	//vColor.g = ((float)(mCRaw[1])) / 1024;
	//vColor.b = ((float)(mCRaw[2])) / 1024;
	////vColor.a = ((float)(oC.a));
	//vColor.a = 255;

	vColor = vec4(texture(channel0, tex_uv.xy).r /1024.0, texture(channel1, tex_uv.xy).r / 1024.0, texture(channel2, tex_uv.xy).r / 1024.0, 255.0); 
	//vColor = vec4(texture(channel0, tex_uv.xy).r / 1024.0, 0, 0, 255.0);
	frag_color = vColor;
	//[End]
	//
	//MixOP mixOp;

	//mixOp.rescale = true;
	//mixOp.op = OP_MAX;
	//mixOp.maskR = mixOp.maskG = mixOp.maskB = true;
	//mixOp.brightness = 0;
	//mixOp.contrast = 1;


	//bool bRescale = mixOp.rescale;
	//float fb = mixOp.brightness;
	//float fc = mixOp.contrast;
	//int op = mixOp.op;
	//RGB8 mask;
	//mask.r = (mixOp.maskR) ? (unsigned char)255 : (unsigned char)0;
	//mask.g = (mixOp.maskG) ? (unsigned char)255 : (unsigned char)0;
	//mask.b = (mixOp.maskB) ? (unsigned char)255 : (unsigned char)0;

	//int k;
	//unsigned int C;
	//float CC;

	//for (int k_on = 0; k_on < channels_On; k_on++)
	//{
	//	k = channel_index_On[k_on];
	//	C = mCRaw[k];
	//	HIST_RESCALE(k, C);  //clamp to [hist_min, hist_max] 
	//	CC = (float)C; 
	//	BRIGHTEN_TRANSFORM(CC); //contrast, brightness, still in [hist_min, hist_max]
	//	//rewrite to the original
	//	C = (unsigned int)CC;
	//	mC[k] = (!bRescale) ? C : ((C - hist_min[k])*255 / vrange[k]);  // rescale to [0, 255]
	//} //end of K

	//RGB8 mixC = lookup_mix_litone(op, mask);
	//	
	//if (format_bgra == 1)
	//{
	//	vColor.rgba = vColor.bgra;
	//}
	////[Test]
	//vColor.r = ((float)(mixC.r)) / 255;
	//vColor.g = ((float)(mixC.g)) / 255;
	//vColor.b = ((float)(mixC.b)) / 255;
	////vColor.a = ((float)(oC.a));
	//vColor.a = 1.0;

	//frag_color = vColor;
	//[End]
	//vColor.r = ((float)(mixC.r)) / 255;
	//vColor.g = ((float)(mixC.g)) / 255;
	//vColor.b = ((float)(mixC.b)) / 255;
	//vColor.a = (vColor.r + vColor.g + vColor.b) / 3; 

	//End of lookup_mix_litone 

	//vec4 C1 = texture2D(colormap, vec2(vColor.r, channel.x));
	//vec4 C2 = texture2D(colormap, vec2(vColor.g, channel.y));
	//vec4 C3 = texture2D(colormap, vec2(vColor.b, channel.z));

	//vec3 aC1 = C1.rgb * C1.a;
	//vec3 aC2 = C2.rgb * C2.a;
	//vec3 aC3 = C3.rgb * C3.a;
	//float Amean = (C1.a + C2.a + C3.a) / 3.0;
	////= pow((C1.a * C2.a * C3.a), 1.0/3.0);
	//float Amax = max(C1.a, max(C2.a, C3.a));

	//vec4 oColor;
	//if (blend_mode == 0) // cross-section
	//{
	//	float Asec = Amean;
	//	oColor.rgb = (aC1 + aC2 + aC3);
	//	oColor.a = Asec;
	//}
	//else if (blend_mode == 1) // alpha-blending
	//{
	//	float Ablend = Amean;
	//	//oColor.rgb = (C1.rgb + C2.rgb + C3.rgb)*Ablend;
	//	oColor.rgb = (aC1 + aC2 + aC3)*Ablend;
	//	oColor.a = Ablend;

	//	//		float Ablend = Amean;
	//	//		vec3 f3 = vec3(equal(vec3(Amax, Amax, Amax), vec3(C1.a, C2.a, C3.a)));
	//	//		oColor.r = dot(vec3(aC1.r, aC2.r, aC3.r), f3)*Ablend;
	//	//		oColor.g = dot(vec3(aC1.g, aC2.g, aC3.g), f3)*Ablend;
	//	//		oColor.b = dot(vec3(aC1.b, aC2.b, aC3.b), f3)*Ablend;
	//	//		oColor.a = Ablend;

	//	//		float Amip = Amean;
	//	//		oColor.rgb = max(aC1.rgb, max(aC2.rgb, aC3.rgb));
	//	//		oColor.a = Amip;
	//}
	//else // max-intensity
	//{
	//	float Amip = Amean;
	//	oColor.rgb = max(aC1.rgb, max(aC2.rgb, aC3.rgb));
	//	oColor.a = Amip;

	//	//		float Asec = Amean;
	//	//		oColor.rgb = (aC1 + aC2 + aC3);
	//	//		oColor.a = Asec;
	//}
	//
	////gl_FragColor = gl_Color * oColor; // modulated by color_proxy

	//frag_color = oColor;
	////frag_color = gl_Color * oColor; // modulated by color_proxy

	//

	
}