#version 450

uniform sampler2D final_buffer;

in vec2 uv;

out vec4 fragment_color;

void main()
{
	fragment_color = texture(final_buffer, uv);
}