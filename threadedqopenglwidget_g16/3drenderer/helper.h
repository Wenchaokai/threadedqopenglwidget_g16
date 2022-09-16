#ifndef __HELPER_H
#define __HELPER_H


void checkGLError(const char* file, int line);

#ifdef _DEBUG
#define GL_CALL(x) do{x;checkGLError(__FILE__,__LINE__);}while(0)
#else
#define GL_CALL(x) x
#endif 

#endif