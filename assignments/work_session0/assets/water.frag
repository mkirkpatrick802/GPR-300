#version 450

out vec4 fragment_color;

in Surface
{
	vec3 world_position;
	vec3 world_normal;
	vec2 texture_coord;
}fs_in;

uniform sampler2D main_texture;
uniform float time;

const float scale = 1;
const vec3 water_color = vec3(0,.31,.85);

void main()
{

	vec2 uv = fs_in.texture_coord * scale + (time * .2);
	uv.x += (sin(uv.y + time) + sin(uv.y * .16 + time * .3) + sin(time * .8)) / 3.0;
	uv.y += (sin(uv.x + time) + sin(uv.x * .22 + time * .5)+ sin(time * .6)) / 3.0;

	vec4 object_color = texture(main_texture, uv).rgba;
	vec4 depth_offset = texture(main_texture, uv * 1.5 + vec2(.2)).rgba;

	fragment_color = vec4(water_color + vec3(object_color * 0.9 - depth_offset * .1), 1);
}