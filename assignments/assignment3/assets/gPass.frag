#version 450

layout(location = 0) out vec3 position;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec3 albedo;

in Surface
{
	vec3 world_position;
	vec3 world_normal;
	vec2 texture_coord;
}fs_in;

uniform sampler2D main_texture;
uniform int uv_scale = 1;

void main()
{
	position = fs_in.world_position;
	normal = normalize(fs_in.world_normal);
	albedo = texture(main_texture,fs_in.texture_coord * uv_scale).rgb;
}
