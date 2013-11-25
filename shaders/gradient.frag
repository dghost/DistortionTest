#version 330


uniform vec2 in_ScreenSize = vec2(1280,800);
uniform vec4 in_ColorBase = vec4(0.0);
uniform vec4 in_ColorX = vec4(0.0,1.0,0.0,0.0);
uniform vec4 in_ColorY = vec4(0.0,0.0,1.0,0.0);

in vec2 texCoords;

out vec4 outColor;

void main(void)
{

    // scale the texture coordinates for better noise

	outColor = in_ColorBase + in_ColorX * texCoords.x + in_ColorY * texCoords.y;
}
