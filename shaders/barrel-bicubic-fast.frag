#version 150

// texture properties
uniform sampler2D Texture; //Image to be projected
uniform vec2 TextureSize;
in vec2 TexCoords;

// distortion parameters
uniform vec2 ScaleIn = vec2(4.0,2.0);
uniform vec2 Scale = vec2(0.25,0.5);
uniform vec4 HmdWarpParam = vec4(1.0,0.22,0.24,0.0);
invariant in vec2 ScreenCenter;
invariant in vec2 LensCenter;


//layout(location = 0) out vec4 outColor; // GLSL 3.30 or higher only

out vec4 outColor; // GLSL 1.50 or higher

// not always necessary, but prevents image problems with certain AMD configurations

vec4 texture2Dclamp(sampler2D texture, vec2 tc)
{
	if (!all(equal(clamp(tc, ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25,0.5)), tc)))
	{
		return vec4(vec3(0.0),1.0);
	} else {
		return texture2D(texture,tc);
	}
}

vec2 Warp(vec2 in01)
{
	vec2 theta = (in01 - LensCenter) * ScaleIn; // Scales to [-1, 1]
	float rSq = theta.x * theta.x + theta.y * theta.y;
	vec2 rvector= theta * (	HmdWarpParam.x + HmdWarpParam.y * rSq +
		HmdWarpParam.z * rSq * rSq 
		+ HmdWarpParam.w * rSq * rSq * rSq
		);
	return (LensCenter + Scale * rvector);
}

vec4 Filter(vec2 texCoord)
{
	// calculate the center of the texel
	texCoord *= TextureSize;
	vec2 texelSize = 1.0 / TextureSize;
	vec2 texelCenter   = floor( texCoord - 0.5 ) + 0.5;
	vec2 fracOffset    = texCoord - texelCenter;
	vec2 fracOffset_x2 = fracOffset * fracOffset;
	vec2 fracOffset_x3 = fracOffset * fracOffset_x2;
	// calculate bspline weight function
	vec2 weight0 = fracOffset_x2 - 0.5 * ( fracOffset_x3 + fracOffset );
	vec2 weight1 = 1.5 * fracOffset_x3 - 2.5 * fracOffset_x2 + 1.0;
	vec2 weight3 = 0.5 * ( fracOffset_x3 - fracOffset_x2 );
	vec2 weight2 = 1.0 - weight0 - weight1 - weight3;
	// calculate texture coordinates
	vec2 scalingFactor0 = weight0 + weight1;
	vec2 scalingFactor1 = weight2 + weight3;
	vec2 f0 = weight1 / ( weight0 + weight1 );
	vec2 f1 = weight3 / ( weight2 + weight3 );
	vec2 texCoord0 = texelCenter - 1.0 + f0;
	vec2 texCoord1 = texelCenter + 1.0 + f1;
	texCoord0 *= texelSize;
	texCoord1 *= texelSize;
	// sample texture and apply weighting
	return texture2Dclamp( Texture, (vec2( texCoord0.x, texCoord0.y )) ) * scalingFactor0.x * scalingFactor0.y +
		   texture2Dclamp( Texture, (vec2( texCoord1.x, texCoord0.y )) ) * scalingFactor1.x * scalingFactor0.y +
		   texture2Dclamp( Texture, (vec2( texCoord0.x, texCoord1.y )) ) * scalingFactor0.x * scalingFactor1.y +
		   texture2Dclamp( Texture, (vec2( texCoord1.x, texCoord1.y )) ) * scalingFactor1.x * scalingFactor1.y;
}

void main(void)
{
	// scale the texture coordinates for better noise
	vec2 warpCoords = Warp(TexCoords);
	outColor = Filter(warpCoords);
}
