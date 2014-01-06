#version 150

uniform vec2 ScreenSize = vec2(1280,800);
uniform vec4 ColorBase = vec4(1.0);
uniform vec4 ColorLine = vec4(0.67,0.85,0.9,1.0);
uniform int LineSpacing = 20;
uniform int LineSize = 1;

in vec2 TexCoords;

out vec4 outColor;

void main(void)
{
	// calculate line spacing
    vec2 warpTexCoords = TexCoords * ScreenSize;
	ivec2 color = ivec2(warpTexCoords) % ivec2(LineSpacing);

	// add lines together
	
	if (any(lessThan(color,ivec2(LineSize))))
	{
		outColor = ColorLine;
	} else {
		outColor = ColorBase;		
	}
}
