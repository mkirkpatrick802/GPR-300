#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec3 world_position;
out vec3 world_normal;
out vec4 lightspace_position;
out mat4 camera_projection;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 lightspace_matrix;

uniform mat4 model_matrix;

void main()
{
	world_position = vec3(model_matrix * vec4(position, 1));
	world_normal = transpose(inverse(mat3(model_matrix))) * normal;
	lightspace_position = lightspace_matrix * vec4(world_position, 1);

	gl_Position = projection_matrix * view_matrix * vec4(world_position, 1);
}