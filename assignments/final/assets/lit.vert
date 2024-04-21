#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texture_coord;

uniform mat4 model_matrix;
uniform mat4 projection_matrix;
uniform mat4 view_matrix;

out Surface
{
	vec3 world_position;
	vec3 world_normal;
	vec2 texture_coord;
}vs_out;

void main()
{
	vs_out.world_position = vec3(model_matrix * vec4(position, 1));
	vs_out.world_normal = transpose(inverse(mat3(model_matrix))) * normal;
	vs_out.texture_coord = texture_coord;

	gl_Position = projection_matrix * view_matrix * vec4(vs_out.world_position, 1);
}