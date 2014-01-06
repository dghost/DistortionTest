#version 150

uniform vec2 ScreenSize = vec2(1280,800);
uniform vec4 ColorBase = vec4(vec3(0.0),1.0);
uniform vec4 ColorX = vec4(0.0,1.0,0.0,0.0);
uniform vec4 ColorY = vec4(0.0,0.0,1.0,0.0);

in vec2 texCoords;

out vec4 outColor;

void main(void)
{
	// apply a gradient from the lower left corner of the screen
	outColor = vec4(texCoords * ScreenSize, 0.0, 1.0);
}
