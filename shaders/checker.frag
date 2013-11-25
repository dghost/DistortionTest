#version 150

uniform vec2 in_ScreenSize = vec2(1280,800);
uniform vec4 in_ColorBase = vec4(1.0,0.0,0.0,0.0);
uniform vec4 in_ColorX = vec4(0.0,1.0,0.0,0.0);
uniform vec4 in_ColorY = vec4(0.0,0.0,1.0,0.0);
uniform float in_Size = 1;

in vec2 texCoords;

out vec4 outColor;

void main(void)
{
	// calculate a checker pattern
    vec2 warpTexCoords = texCoords * in_ScreenSize / vec2(in_Size);
	ivec2 color = ivec2(warpTexCoords) % ivec2(2);
	
	outColor = (color.x == color.y) ? vec4(0.0) : in_ColorBase + in_ColorX * texCoords.x + in_ColorY * texCoords.y;
}
