#version 150

uniform vec2 in_ScreenSize = vec2(1280,800);
uniform vec4 in_ColorBase = vec4(vec3(0.0),1.0);
uniform vec4 in_ColorX = vec4(0.0,1.0,0.0,0.0);
uniform vec4 in_ColorY = vec4(0.0,0.0,1.0,0.0);

in vec2 TexCoords;

out vec4 outColor;

void main(void)
{
	// apply a gradient from the lower left corner of the screen
	outColor = in_ColorBase + in_ColorX * TexCoords.x + in_ColorY * TexCoords.y;
}
