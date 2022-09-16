#include "backfill_model.h"

BackFillModel::BackFillModel()
{
}

BackFillModel::~BackFillModel()
{
}

void BackFillModel::SetPos(vec3 pos)
{
	center_Pos = pos;
}

vec3 BackFillModel::GetPos()
{
	return center_Pos;
}

mat4 BackFillModel::GetModelMatrix()
{
	return modelmatrix;
}

void BackFillModel::UpdateData()
{
	
}

void BackFillModel::Init()
{
	center_Pos = vec3(0.5f, 0.5f, 0.5f);
	modelmatrix = mat4(1.0f);
}

void BackFillModel::InitStack(float VOL_X1, float VOL_X0, float VOL_Y1, float VOL_Y0, float VOL_Z1, float VOL_Z0)
{
	point_num = 6 * 6;
	vertex_num = 3 * point_num;
	//定义顶点数组
	GLfloat *vertc = new GLfloat[vertex_num];
	int vindex = 0;
	//yx0
	vertc[vindex++] = VOL_X0; vertc[vindex++] = VOL_Y0; vertc[vindex++] = VOL_Z0;
	vertc[vindex++] = VOL_X0; vertc[vindex++] = VOL_Y1; vertc[vindex++] = VOL_Z0;
	vertc[vindex++] = VOL_X1; vertc[vindex++] = VOL_Y0; vertc[vindex++] = VOL_Z0;
	vertc[vindex++] = VOL_X1; vertc[vindex++] = VOL_Y0; vertc[vindex++] = VOL_Z0;
	vertc[vindex++] = VOL_X0; vertc[vindex++] = VOL_Y1; vertc[vindex++] = VOL_Z0;
	vertc[vindex++] = VOL_X1; vertc[vindex++] = VOL_Y1; vertc[vindex++] = VOL_Z0;
	//x0z
	vertc[vindex++] = VOL_X0; vertc[vindex++] = VOL_Y0; vertc[vindex++] = VOL_Z0;
	vertc[vindex++] = VOL_X1; vertc[vindex++] = VOL_Y0; vertc[vindex++] = VOL_Z0;
	vertc[vindex++] = VOL_X0; vertc[vindex++] = VOL_Y0; vertc[vindex++] = VOL_Z1;
	vertc[vindex++] = VOL_X0; vertc[vindex++] = VOL_Y0; vertc[vindex++] = VOL_Z1;
	vertc[vindex++] = VOL_X1; vertc[vindex++] = VOL_Y0; vertc[vindex++] = VOL_Z0;
	vertc[vindex++] = VOL_X1; vertc[vindex++] = VOL_Y0; vertc[vindex++] = VOL_Z1;
	//0zy
	vertc[vindex++] = VOL_X0; vertc[vindex++] = VOL_Y0; vertc[vindex++] = VOL_Z0;
	vertc[vindex++] = VOL_X0; vertc[vindex++] = VOL_Y0; vertc[vindex++] = VOL_Z1;
	vertc[vindex++] = VOL_X0; vertc[vindex++] = VOL_Y1; vertc[vindex++] = VOL_Z0;
	vertc[vindex++] = VOL_X0; vertc[vindex++] = VOL_Y1; vertc[vindex++] = VOL_Z0;
	vertc[vindex++] = VOL_X0; vertc[vindex++] = VOL_Y0; vertc[vindex++] = VOL_Z1;
	vertc[vindex++] = VOL_X0; vertc[vindex++] = VOL_Y1; vertc[vindex++] = VOL_Z1;
	//xy1
	vertc[vindex++] = VOL_X0; vertc[vindex++] = VOL_Y0; vertc[vindex++] = VOL_Z1;
	vertc[vindex++] = VOL_X1; vertc[vindex++] = VOL_Y0; vertc[vindex++] = VOL_Z1;
	vertc[vindex++] = VOL_X0; vertc[vindex++] = VOL_Y1; vertc[vindex++] = VOL_Z1;
	vertc[vindex++] = VOL_X0; vertc[vindex++] = VOL_Y1; vertc[vindex++] = VOL_Z1;
	vertc[vindex++] = VOL_X1; vertc[vindex++] = VOL_Y0; vertc[vindex++] = VOL_Z1;
	vertc[vindex++] = VOL_X1; vertc[vindex++] = VOL_Y1; vertc[vindex++] = VOL_Z1;
	//z1x
	vertc[vindex++] = VOL_X0; vertc[vindex++] = VOL_Y1; vertc[vindex++] = VOL_Z0;
	vertc[vindex++] = VOL_X0; vertc[vindex++] = VOL_Y1; vertc[vindex++] = VOL_Z1;
	vertc[vindex++] = VOL_X1; vertc[vindex++] = VOL_Y1; vertc[vindex++] = VOL_Z0;
	vertc[vindex++] = VOL_X1; vertc[vindex++] = VOL_Y1; vertc[vindex++] = VOL_Z0;
	vertc[vindex++] = VOL_X0; vertc[vindex++] = VOL_Y1; vertc[vindex++] = VOL_Z1;
	vertc[vindex++] = VOL_X1; vertc[vindex++] = VOL_Y1; vertc[vindex++] = VOL_Z1;
	//1yz
	vertc[vindex++] = VOL_X1; vertc[vindex++] = VOL_Y0; vertc[vindex++] = VOL_Z0;
	vertc[vindex++] = VOL_X1; vertc[vindex++] = VOL_Y1; vertc[vindex++] = VOL_Z0;
	vertc[vindex++] = VOL_X1; vertc[vindex++] = VOL_Y0; vertc[vindex++] = VOL_Z1;
	vertc[vindex++] = VOL_X1; vertc[vindex++] = VOL_Y0; vertc[vindex++] = VOL_Z1;
	vertc[vindex++] = VOL_X1; vertc[vindex++] = VOL_Y1; vertc[vindex++] = VOL_Z0;
	vertc[vindex++] = VOL_X1; vertc[vindex++] = VOL_Y1; vertc[vindex++] = VOL_Z1;
	
	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);

	glGenBuffers(1, vboHandle);
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle[0]);//激活（绑定）当前缓冲区
	glBufferData(GL_ARRAY_BUFFER, vertex_num * sizeof(GLfloat), vertc, GL_STATIC_DRAW);//内存数据复制到显存
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)));//Vertex Shader的顶点坐标输入属性（序号0，坐标）对应
	glEnableVertexAttribArray(0);  // 允许Vertex着色器中输入变量0读取显存数据。

	glBindVertexArray(0);

	delete[]vertc;
}

void BackFillModel::Render()
{
	glBindVertexArray(vaoHandle);
	glDrawArrays(GL_TRIANGLES, 0, point_num);
	glBindVertexArray(0);
}

void BackFillModel::rotateModelMatrix(mat4 rotatemat)
{
	modelmatrix = mat4(1.0f);
	modelmatrix *= rotatemat;
}

void BackFillModel::CalModelMatrix(const BoundingBox BB)
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
