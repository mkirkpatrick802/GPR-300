#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 model_matrix;
uniform mat4 projection_matrix;

void main()
{
	gl_Position = projection_matrix * model_matrix * vec4(position, 1);
}