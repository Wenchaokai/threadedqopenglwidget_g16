#version 430

out vec4 frag_color;

void main()
{
    vec3 dafault_color = vec3(0.0, 0.0, 0.0);
    frag_color = vec4(dafault_color, 1.0);
}