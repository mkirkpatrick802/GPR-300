#version 450

out vec4 fragment_color;

in vec2 texture_coord_out;

uniform sampler2D screen_texture;

void main() {

	vec3 rgb = texture(screen_texture, texture_coord_out).rgb;
	fragment_color = vec4(1.0, 0.0, 1.0, 1.0);
}