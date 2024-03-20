#version 450

out vec4 fragment_color;

in Surface
{
	vec3 world_position;
	vec3 world_normal;
	vec2 texture_coord;
}fs_in;

uniform sampler2D main_texture;
uniform float height_scale;

void main()
{
	if(fs_in.world_position.y <= 0.1)
		discard;

	vec3 color = vec3(0, 0, 0);
	color += fs_in.world_position.y / height_scale;

	fragment_color = vec4(color, 1);
}