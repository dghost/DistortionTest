#version 150

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

out vec2 texCoords;

uniform vec2 lowerLeft = vec2(0.0,0.0);
uniform vec2 upperRight = vec2(1.0,1.0);

void main()
{
    gl_Position = vec4( 1.0, 1.0, 0.0, 1.0 );
	texCoords = vec2( upperRight.x, upperRight.y );
    EmitVertex();

    gl_Position = vec4(-1.0, 1.0, 0.0, 1.0 );
	texCoords = vec2( lowerLeft.x, upperRight.y );
    EmitVertex();

    gl_Position = vec4( 1.0,-1.0, 0.0, 1.0 );
	texCoords = vec2( upperRight.x, lowerLeft.y );
    EmitVertex();

    gl_Position = vec4(-1.0,-1.0, 0.0, 1.0 );
	texCoords = vec2( lowerLeft.x, lowerLeft.y );
    EmitVertex();

    EndPrimitive();
}
