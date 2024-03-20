#version 450

out vec4 fragment_color;
in vec2 uv;

uniform float threshold;
uniform sampler2D transition_texture;

void main() 
{
	float pixel_value = texture(transition_texture, uv).x;
	
	if (pixel_value > threshold)
		discard;

	fragment_color = vec4(.2, .3, .5, 1);
}