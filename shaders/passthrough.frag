#version 330

uniform sampler2D Texture; //Image to be projected

in vec2 texCoords;

layout(location = 0) out vec4 out_Color;

void main(void)
{
	out_Color =  texture(Texture,texCoords);
}
