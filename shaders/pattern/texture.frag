#version 150
uniform sampler2D Texture; //Image to be projected

in vec2 texCoords;
out vec4 outColor;

void main(void)
{
	outColor = texture2D(Texture,texCoords);
}
