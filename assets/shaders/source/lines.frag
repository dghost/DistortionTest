#version 150


uniform vec2 ScreenSize = vec2(1280,800);
uniform vec4 ColorBase = vec4(vec3(0.0),1.0);
uniform vec4 ColorX = vec4(1.0,0.0,0.0,0.0);
uniform vec4 ColorY = vec4(0.0,0.0,1.0,0.0);
uniform float Size = 1;
in vec2 TexCoords;

out vec4 outColor;

void main(void)
{
	// calculate line spacing
    vec2 warpTexCoords = TexCoords * ScreenSize;
	ivec2 color = ivec2(warpTexCoords) % ivec2(1 + Size);

	// add lines together
	outColor = ColorBase;
	outColor += (color.x == 0) ? ColorX : vec4(vec3(0.0),1.0);
	outColor += (color.y == 0) ? ColorY : vec4(vec3(0.0),1.0);
}
