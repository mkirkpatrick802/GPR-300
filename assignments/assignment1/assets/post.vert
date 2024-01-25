#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texture_coord;

out vec2 texture_coord_out;

void main() {

	texture_coord_out = texture_coord;
	gl_Position = vec4(position, 0, 1);
}