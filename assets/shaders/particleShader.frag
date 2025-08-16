#version 460 core
out vec4 FragColor;

uniform vec3 color = vec3(1.0, 1.0, 1.0);

void main() {
	FragColor = vec4(color, 1.0);
}