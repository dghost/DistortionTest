#version 150

uniform vec2 ScreenSize = vec2(1280,800);
uniform vec4 ColorBase = vec4(vec3(0.0),1.0);
uniform vec4 ColorX = vec4(1.0,0.0,0.0,0.0);
uniform vec4 ColorY = vec4(0.0,0.0,1.0,0.0);
uniform float Size = 1;

in vec2 texCoords;

out vec4 outColor;

void main(void)
{
	// calculate a checker pattern
    vec2 warpTexCoords = texCoords * ScreenSize / vec2(Size);
	ivec2 color = ivec2(warpTexCoords) % ivec2(2);
	
	outColor = (color.x == color.y) ? vec4(vec3(0.0),1.0) : ColorBase + ColorX + ColorY;
}
