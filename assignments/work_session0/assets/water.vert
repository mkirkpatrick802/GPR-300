#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texture_coord;

uniform mat4 model_matrix;
uniform mat4 projection_matrix;

uniform float time;
uniform float wave_strength = 1.1;
uniform float wave_scale = 1.5;

out Surface
{
	vec3 world_position;
	vec3 world_normal;
	vec2 texture_coord;

}vs_out;

float calculate_surface(float x, float z) {
	float y = 0;
	y += (sin(x * .2 / wave_scale + time * .3) + sin(x * .22 / wave_scale + time * .5) + sin(x * .4 / wave_scale + time * .6)) / 3.0;
	y += (sin(z * .3 / wave_scale + time * .6) + sin(z * .22 / wave_scale + time * .5) + sin(y * .7 / wave_scale + time * .6)) / 3.0;
	return y;
}

void main()
{
	vs_out.world_position = vec3(model_matrix * vec4(position, 1));
	vs_out.world_normal = transpose(inverse(mat3(model_matrix))) * normal;
	vs_out.texture_coord = texture_coord * 50;

	vec3 pos = position;
	pos.y = calculate_surface(pos.x, pos.z) * wave_strength;

	gl_Position = projection_matrix * model_matrix * vec4(pos, 1);
}