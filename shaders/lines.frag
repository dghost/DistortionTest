#version 150


uniform vec2 in_ScreenSize = vec2(1280,800);
uniform vec4 in_ColorBase = vec4(vec3(0.0),1.0);
uniform vec4 in_ColorX = vec4(1.0,0.0,0.0,0.0);
uniform vec4 in_ColorY = vec4(0.0,0.0,1.0,0.0);
uniform float in_Size = 1;
in vec2 texCoords;

out vec4 outColor;

void main(void)
{
	// calculate line spacing
    vec2 warpTexCoords = texCoords * in_ScreenSize;
	ivec2 color = ivec2(warpTexCoords) % ivec2(1 + in_Size);

	// add lines together
	outColor = in_ColorBase;
	outColor += (color.x == 0) ? in_ColorX : vec4(vec3(0.0),1.0);
	outColor += (color.y == 0) ? in_ColorY : vec4(vec3(0.0),1.0);
}
