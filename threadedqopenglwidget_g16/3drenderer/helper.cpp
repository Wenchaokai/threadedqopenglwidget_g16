#include "helper.h"
//#include "glad/glad.h"
#include <GL\glew.h>
#include <cstdio>
#include <cassert>
#include <QDebug>

void checkGLError(const char* file, int line) {

	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		switch (error) {
		case GL_INVALID_ENUM:
			qDebug("GL Error: GL_INVALID_ENUM %s : %d \n", file, line);
			break;
		case GL_INVALID_VALUE:
			qDebug("GL Error: GL_INVALID_VALUE %s : %d \n", file, line);
			break;
		case GL_INVALID_OPERATION:
			qDebug("GL Error: GL_INVALID_OPERATION %s : %d \n", file, line);
			break;
		case GL_STACK_OVERFLOW:
			qDebug("GL Error: GL_STACK_OVERFLOW %s : %d \n", file, line);
			break;
		case GL_STACK_UNDERFLOW:
			qDebug("GL Error: GL_STACK_UNDERFLOW %s : %d \n", file, line);
			break;
		case GL_OUT_OF_MEMORY:
			qDebug("GL Error: GL_OUT_OF_MEMORY %s : %d \n", file, line);
			break;
		default:
			qDebug("GL Error: 0x%x %s : %d \n", error, file, line);
			break;
		}
		//Q_ASSERT(false);
	}
	return;
}
