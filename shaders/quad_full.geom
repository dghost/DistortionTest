#version 150

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

out vec2 texCoords;

uniform vec2 in_lowerLeft = vec2(0.0,0.0);
uniform vec2 in_upperRight = vec2(1.0,1.0);

void main()
{
    gl_Position = vec4( 1.0, 1.0, 0.0, 1.0 );
	texCoords = vec2( in_upperRight.x, in_upperRight.y );
    EmitVertex();

    gl_Position = vec4(-1.0, 1.0, 0.0, 1.0 );
	texCoords = vec2( in_lowerLeft.x, in_upperRight.y );
    EmitVertex();

    gl_Position = vec4( 1.0,-1.0, 0.0, 1.0 );
	texCoords = vec2( in_upperRight.x, in_lowerLeft.y );
    EmitVertex();

    gl_Position = vec4(-1.0,-1.0, 0.0, 1.0 );
	texCoords = vec2( in_lowerLeft.x, in_lowerLeft.y );
    EmitVertex();

    EndPrimitive();
}
