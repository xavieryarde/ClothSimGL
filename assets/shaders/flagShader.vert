#version 460 core
layout (location=0) in vec3 aPos;
layout (location=1) in vec2 aTex;
layout (location=2) in vec3 aNormal;

layout (std140, binding=0) uniform Matrices {
    mat4 projection;
    mat4 view;
};

uniform mat4 model;

out vec2 Tex;
out vec3 Normal;
out vec3 FragPos;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    Tex = aTex;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
