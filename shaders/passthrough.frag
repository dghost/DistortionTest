#version 150
uniform sampler2D Texture;
in vec2 texCoords;

out vec4 out_Color;

void main(void)
{
	out_Color =  texture(Texture,texCoords);
}
