#version 450

uniform sampler2D crosshatching_texture;
uniform sampler2D outline_texture;

in vec2 uv;

out vec4 fragment_color;

void main()
{
	vec3 crosshatching = texture(crosshatching_texture, uv).rgb;
	vec3 outine = texture(outline_texture, uv).rgb;

	fragment_color = vec4(crosshatching * outine, 1);
}