#version 150

uniform sampler2D Texture; //Image to be projected

uniform vec2 ScaleIn = vec2(4.0,2.0);
uniform vec2 Scale = vec2(0.25,0.5);

uniform vec4 HmdWarpParam = vec4(1.0,0.22,0.24,0.0);

invariant in vec2 ScreenCenter;
invariant in vec2 LensCenter;

in vec2 TexCoords;

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

vec4 Filter(vec2 tc)
{
	return texture2Dclamp(Texture,tc);
}

void main(void)
{
	// scale the texture coordinates for better noise
	vec2 warpCoords = Warp(TexCoords);
	outColor = Filter(warpCoords);
}
