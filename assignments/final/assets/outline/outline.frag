#version 450

layout(location = 4) out vec3 final_buffer;
in vec2 uv;

uniform sampler2D _ColorBuffer;
uniform sampler2D _DepthBuffer;
uniform vec3 _OutlineColor;

const float offset = 1.0 / 600.0;

void main()
{
	
	//offset
	vec2 offsets[9] = vec2[]
	(
		vec2(-offset, offset), // top left
		vec2(0.0f, offset),    // top center
		vec2(offset, offset),  // top right
		vec2(-offset, 0.0f),   // center left
		vec2(0.0f, 0.0f),      // center center
		vec2(offset, 0.0f),    // center right
		vec2(-offset, -offset),// bottom left
		vec2(0.0f, -offset),   // bottom center
		vec2(offset, -offset)  // bottom right
	);

	float kernel[9] = float[](
		1,  1, 1,
		1,  -9, 1,
		1,  1, 1
	);

	vec3 sampleTex[9];
	vec2 copy_uv = uv;
	for(int i = 0; i < 9; i++)
	{
		sampleTex[i] = vec3(texture(_ColorBuffer, copy_uv.st + offsets[i]));
	}
	vec3 col = vec3(0);
	for(int i = 0; i < 9; i++)
	{
		col += sampleTex[i] * kernel[i];
	}

	vec3 finalColor = col;
	vec3 rgb = texture(_ColorBuffer, uv).rgb;
	if(col.r >= -1.3 && col.r <1)
	{
		finalColor = rgb;
	}
	else
	{
		finalColor = _OutlineColor;
	}
	

	final_buffer = finalColor;
}