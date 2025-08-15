#version 460 core
layout(location = 0) in vec3 aPos;

layout (std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

void main() {
	gl_Position = projection * view * vec4(aPos, 1.0);
	gl_PointSize = 5.0;
}