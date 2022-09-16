#version 430 core//version
//input
layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexTexCoord;

//output
out vec3 TexCoord;

//uniform
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main()
{
    TexCoord = VertexTexCoord;
    mat4 mvp = ProjectionMatrix * ViewMatrix * ModelMatrix;
    gl_Position = mvp * vec4(VertexPosition,1.0);
}
