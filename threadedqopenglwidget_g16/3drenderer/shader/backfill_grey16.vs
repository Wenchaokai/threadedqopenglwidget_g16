#version 430 //version
//input
layout (location = 0) in vec3 VertexPosition;

//uniform
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main()
{
    mat4 mvp=ProjectionMatrix * ViewMatrix * ModelMatrix;
    gl_Position = mvp * vec4(VertexPosition,1.0);
}
