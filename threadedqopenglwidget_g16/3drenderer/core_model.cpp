#include "core_model.h"

CoreModel::CoreModel()
{
	
}

CoreModel::~CoreModel()
{
}

void CoreModel::SetPos(vec3 pos)
{
	center_Pos = pos;
}

vec3 CoreModel::GetPos()
{
	return center_Pos;
}

void CoreModel::SetTextureId(GLuint textureid)
{
	textureArrayId_rgb = textureid;
}

void CoreModel::SetTextureId(GLuint textureid, int channel)
{
	textureArrayId[channel] = textureid;
}

GLuint CoreModel::GetTextureId()
{
	return textureArrayId_rgb;
}

GLuint CoreModel::GetTextureId(int channel)
{
	return textureArrayId[channels];
}

mat4 CoreModel::GetModelMatrix()
{
	return modelmatrix;
}

void CoreModel::SetTexID(unsigned int textID)
{
	//m_iTexture = textID;
}

unsigned int CoreModel::GetTexID()
{
	return 0;
}
void CoreModel::Init()
{
	center_Pos = vec3(0.5f, 0.5f, 0.5f);
	modelmatrix = mat4(1.0f);
}

void CoreModel::InitStack()
{
	//非法情况，无切片保留(判断外移)
	if ((s1 - s0 < 0) || (h1 - h0 < 0) || (w1 - w0 < 0)) return; // no draw

	// only support thickness>=1(判断外移)
	if (thickness < 1) return; 

	// cross-section模式下，每个方向仅保留一个切面,此时range控件仅有一个可拖拽的按钮(判断外移)
	if (section > 0) { 
		h0 = 0;		h1 = 1;
		w0 = 0;		w1 = 1;
		s1 = s0;
		slice1 = slice0;
	}

	//根据不同绘制方向调整slice顺序，循环从大到小或者从小到大(跟坐标准备方式有关)
	//建议
	double step, slice, s;
	if (direction < 0) {
		step = (+1); ///moreslice);
		slice = slice0;
		s = s0;
	}
	else {
		step = (-1); ///moreslice);
		slice = slice1;
		s = s1;
	}

	double tw0 = tw*w0;  double tw1 = tw*w1;
	double th0 = th*h0;  double th1 = th*h1;

	point_num = 6 * slice1;
	vertex_num = 3 * point_num;
	textcoord_num = 3 * point_num;
	//定义顶点数组
	GLfloat *vertc = new GLfloat[vertex_num];
	//纹理坐标
	GLfloat *textcor = new GLfloat[textcoord_num];
	int vindex = 0;
	int tindex = 0;

	for (; slice < slice1; slice += step, s += step * ds)
	{
		//one slice with two trangles
		vertc[vindex++] = w0; vertc[vindex++] = h0; vertc[vindex++] = s;
		vertc[vindex++] = w1; vertc[vindex++] = h0; vertc[vindex++] = s;
		vertc[vindex++] = w0; vertc[vindex++] = h1; vertc[vindex++] = s;
		vertc[vindex++] = w0; vertc[vindex++] = h1; vertc[vindex++] = s;
		vertc[vindex++] = w1; vertc[vindex++] = h0; vertc[vindex++] = s;
		vertc[vindex++] = w1; vertc[vindex++] = h1; vertc[vindex++] = s;
		textcor[tindex++] = tw0; textcor[tindex++] = th0; textcor[tindex++] = slice;
		textcor[tindex++] = tw1; textcor[tindex++] = th0; textcor[tindex++] = slice;
		textcor[tindex++] = tw0; textcor[tindex++] = th1; textcor[tindex++] = slice;
		textcor[tindex++] = tw0; textcor[tindex++] = th1; textcor[tindex++] = slice;
		textcor[tindex++] = tw1; textcor[tindex++] = th0; textcor[tindex++] = slice;
		textcor[tindex++] = tw1; textcor[tindex++] = th1; textcor[tindex++] = slice;
	}

	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);

	glGenBuffers(2, vboHandle);
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle[0]);
	glBufferData(GL_ARRAY_BUFFER, vertex_num * sizeof(GLfloat), vertc, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)));
	glEnableVertexAttribArray(0); 

	glBindBuffer(GL_ARRAY_BUFFER, vboHandle[1]);
	glBufferData(GL_ARRAY_BUFFER, textcoord_num * sizeof(GLfloat), textcor, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	delete []vertc;
	delete []textcor;
}

void CoreModel::UpdateParam(double ts, double th, double tw, double s0, double s1, double h0, double h1, double w0, double w1, double ds, int slice0, int slice1, int thickness, GLuint texs[], int stack_i, float direction, int section, int channels)
{
	this->ts = ts; this->th = th; this->tw = tw;
	this->s0 = s0; this->s1 = s1;
	this->h0 = h0; this->h1 = h1;
	this->w0 = w0; this->w1 = w1;

	this->ds = ds;
	this->slice0 = slice0; this->slice1 = slice1;
	this->thickness = thickness; 
	this->texs = texs;
	this->stack_i = stack_i;
	this->direction = direction;
	this->section = section;
	this->channels = channels;
	
}
void CoreModel::Render()
{
	int draw_startpos = 0;
	

#ifndef GREY16_TEXT
	int draw_endpos = point_num;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayId_rgb); //[0] reserved for pbo tex

	glBindVertexArray(vaoHandle);
	
	glDrawArrays(GL_TRIANGLES, draw_startpos, draw_endpos);
#else
#ifndef TEXTURE_ARRAY
	int draw_endpos = 12;
	for (int i = 0; i < slice1; i++)
	{
		for (int ch = 0; ch < channels; ch++)
		{
			glActiveTexture(GL_TEXTURE1 + ch);
			glBindTexture(GL_TEXTURE_2D, texs[int(i * channels + ch) + 1]); //[0] reserved for pbo tex
		}
		glBindVertexArray(vaoHandle);
		glDrawArrays(GL_QUADS, draw_startpos, draw_endpos);
		draw_startpos = draw_endpos;
		draw_endpos += 12;
	}
#else
	int draw_endpos = point_num;
	for (int ch = 0; ch < channels; ch++)
	{
		glActiveTexture(GL_TEXTURE1 + ch);
		glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayId[ch]); //[0] reserved for pbo tex
	}
	glBindVertexArray(vaoHandle);
	//GL_QUADS is abandoned
	//glDrawArrays(GL_QUADS, draw_startpos, draw_endpos);
	glDrawArrays(GL_TRIANGLES, draw_startpos, draw_endpos);
#endif
#endif
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void CoreModel::rotateModelMatrix(mat4 rotatemat)
{
	modelmatrix = mat4(1.0f);
	modelmatrix *= rotatemat;
}

void CoreModel::CalModelMatrix(const BoundingBox BB)
{
	float DX = BB.Dx();
	float DY = BB.Dy();
	float DZ = BB.Dz();
	float maxD = BB.Dmax();

	float s[3];

	s[0] = DX * flip_X / maxD * 2;
	s[1] = DY * flip_Y / maxD * 2;
	s[2] = DZ * flip_Z / maxD * 2;

	modelmatrix = glm::scale(modelmatrix, vec3(s[0], s[1], s[2]));
	modelmatrix = glm::translate(modelmatrix, -center_Pos);
}
