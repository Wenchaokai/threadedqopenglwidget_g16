#version 330 core

//#ifdef TEX3D
//uniform sampler3D volume;
//#else
//uniform sampler2D volume;
//#endif


out vec4 frag_color;

uniform sampler2D stex1;
uniform sampler2D stex2;
uniform sampler2D colormap;

in vec2 tex_uv;



uniform vec3 channel;
//uniform int blend_mode;
//uniform int format_bgra;

//////////////////////////////////////
void main()
{

//#ifdef TEX3D
//	vec4 vColor = texture3D(volume, gl_TexCoord[0].xyz);
//#else
//	vec4 vColor = texture2D(volume, gl_TexCoord[0].xy);
//#endif
//	if (format_bgra == 1)
//	{
//		vColor.rgba = vColor.bgra;
//	}
//
//#ifdef TEX_LOD // only works in vertex shader!!!
//	vec4 C1 = texture2DLod(colormap, vec2(vColor.r, 0), channel.x);
//	vec4 C2 = texture2DLod(colormap, vec2(vColor.g, 0), channel.y);
//	vec4 C3 = texture2DLod(colormap, vec2(vColor.b, 0), channel.z);
//#else
//	vec4 C1 = texture2D(colormap, vec2(vColor.r, channel.x));
//	vec4 C2 = texture2D(colormap, vec2(vColor.g, channel.y));
//	vec4 C3 = texture2D(colormap, vec2(vColor.b, channel.z));
//#endif
//
//	vec3 aC1 = C1.rgb * C1.a;
//	vec3 aC2 = C2.rgb * C2.a;
//	vec3 aC3 = C3.rgb * C3.a;
//	float Amean = (C1.a + C2.a + C3.a) / 3.0;
//	//= pow((C1.a * C2.a * C3.a), 1.0/3.0);
//	float Amax = max(C1.a, max(C2.a, C3.a));
//
//	vec4 oColor;
//	if (blend_mode == 0) // cross-section
//	{
//		float Asec = Amean;
//		oColor.rgb = (aC1 + aC2 + aC3);
//		oColor.a = Asec;
//	}
//	else if (blend_mode == 1) // alpha-blending
//	{
//		float Ablend = Amean;
//		//oColor.rgb = (C1.rgb + C2.rgb + C3.rgb)*Ablend;
//		oColor.rgb = (aC1 + aC2 + aC3)*Ablend;
//		oColor.a = Ablend;
//
//		//		float Ablend = Amean;
//		//		vec3 f3 = vec3(equal(vec3(Amax, Amax, Amax), vec3(C1.a, C2.a, C3.a)));
//		//		oColor.r = dot(vec3(aC1.r, aC2.r, aC3.r), f3)*Ablend;
//		//		oColor.g = dot(vec3(aC1.g, aC2.g, aC3.g), f3)*Ablend;
//		//		oColor.b = dot(vec3(aC1.b, aC2.b, aC3.b), f3)*Ablend;
//		//		oColor.a = Ablend;
//
//		//		float Amip = Amean;
//		//		oColor.rgb = max(aC1.rgb, max(aC2.rgb, aC3.rgb));
//		//		oColor.a = Amip;
//	}
//	else // max-intensity
//	{
//		float Amip = Amean;
//		oColor.rgb = max(aC1.rgb, max(aC2.rgb, aC3.rgb));
//		oColor.a = Amip;
//
//		//		float Asec = Amean;
//		//		oColor.rgb = (aC1 + aC2 + aC3);
//		//		oColor.a = Asec;
//	}
//
//	gl_FragColor = gl_Color * oColor; // modulated by color_proxy
//									  //gl_FragData[0] = gl_Color * vColor;
//									  //gl_FragData[1] = vec4(0,1,0,1);


	vec4 vColor = vec4(texture2D(stex1, tex_uv).r, texture2D(stex2, tex_uv).r, 0, 1.0f); //


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
	//
	//	//		float Ablend = Amean;
	//	//		vec3 f3 = vec3(equal(vec3(Amax, Amax, Amax), vec3(C1.a, C2.a, C3.a)));
	//	//		oColor.r = dot(vec3(aC1.r, aC2.r, aC3.r), f3)*Ablend;
	//	//		oColor.g = dot(vec3(aC1.g, aC2.g, aC3.g), f3)*Ablend;
	//	//		oColor.b = dot(vec3(aC1.b, aC2.b, aC3.b), f3)*Ablend;
	//	//		oColor.a = Ablend;
	//
	//	//		float Amip = Amean;
	//	//		oColor.rgb = max(aC1.rgb, max(aC2.rgb, aC3.rgb));
	//	//		oColor.a = Amip;
	//}
	//else // max-intensity
	{
		float Amip = Amean;
		oColor.rgb = max(aC1.rgb, max(aC2.rgb, aC3.rgb));
		oColor.a = Amip;
	
		//		float Asec = Amean;
		//		oColor.rgb = (aC1 + aC2 + aC3);
		//		oColor.a = Asec;
	}

	//frag_color = gl_Color * oColor;
	frag_color = oColor;
}
