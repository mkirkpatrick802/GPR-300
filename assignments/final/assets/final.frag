#version 450

uniform sampler2D crosshatching_texture;
uniform sampler2D outline_texture;

uniform sampler2D noise_texture;
uniform float noise_factor = .1;

in vec2 uv;

out vec4 fragment_color;

void main()
{
	vec3 crosshatching = texture(crosshatching_texture, uv).rgb;
	vec3 outline = texture(outline_texture, uv).rgb;
	float noise = texture(noise_texture, uv).r;
	vec4 final_effect = vec4(crosshatching * outline, 1);

	fragment_color = final_effect + noise_factor * noise;
}