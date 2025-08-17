#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D clothTexture;

void main()
{
    FragColor = texture(clothTexture, TexCoord);
}
