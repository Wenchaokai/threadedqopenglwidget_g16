#pragma once
#include "glsl_r.h"
class BaseModel
{
public:
	BaseModel();
	virtual ~BaseModel(void);
	virtual void Render()=0;
	virtual mat4 GetModelMatrix()=0;
protected:
	unsigned int vboHandle[3];
	unsigned int vaoHandle;
	unsigned int ebOHandle;
	static const int flip_X = +1, flip_Y = -1, flip_Z = -1; // make y-axis downward conformed with image coordinate
	mat4 modelmatrix;
	int point_num = 0;
	int vertex_num = 0;
	int textcoord_num = 0;
};