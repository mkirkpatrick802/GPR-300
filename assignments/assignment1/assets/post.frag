#version 450

out vec4 fragment_color;

in vec2 texture_coord_out;

uniform sampler2D screen_texture;

uniform bool invert_color;
uniform bool depth_edge_detection;

uniform float screen_width;
uniform float screen_height;

void make_kernel(inout vec4 n[9], sampler2D tex, vec2 coord)
{
	float w = 1.0 / screen_width;
	float h = 1.0 / screen_height;

	n[0] = texture2D(tex, coord + vec2( -w, -h));
	n[1] = texture2D(tex, coord + vec2(0.0, -h));
	n[2] = texture2D(tex, coord + vec2(  w, -h));
	n[3] = texture2D(tex, coord + vec2( -w, 0.0));
	n[4] = texture2D(tex, coord);
	n[5] = texture2D(tex, coord + vec2(  w, 0.0));
	n[6] = texture2D(tex, coord + vec2( -w, h));
	n[7] = texture2D(tex, coord + vec2(0.0, h));
	n[8] = texture2D(tex, coord + vec2(  w, h));
}

void main() {

	vec4 n[9];
	make_kernel(n, screen_texture, texture_coord_out);

	vec4 sobel_edge_h = n[2] + (2.0*n[5]) + n[8] - (n[0] + (2.0*n[3]) + n[6]);
  	vec4 sobel_edge_v = n[0] + (2.0*n[1]) + n[2] - (n[6] + (2.0*n[7]) + n[8]);
	vec4 sobel = sqrt((sobel_edge_h * sobel_edge_h) + (sobel_edge_v * sobel_edge_v));

	vec3 rgb;
	if(depth_edge_detection)
	{
		rgb = sobel.rgb;
	}
	else
	{
		rgb = texture(screen_texture, texture_coord_out).rgb;
	}

	if(invert_color)
	{
		fragment_color = vec4(1 - rgb, 1.0);
	}
	else
	{
		fragment_color = vec4(rgb, 1.0);
	}
}