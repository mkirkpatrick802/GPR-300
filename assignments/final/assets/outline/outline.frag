#version 450

layout(location = 1) out vec3 normal_buffer;
layout(location = 4) out vec3 final_buffer;

in vec2 uv;

uniform sampler2D color_texture;
uniform sampler2D normal_texture;

uniform int screen_width;
uniform int screen_height;

vec4 apply_outline(sampler2D texture_buffer);
void make_kernel(inout vec4 n[9], sampler2D tex, vec2 coord);

void main()
{
	vec4 sobel_color = apply_outline(color_texture);
	{
		float sobel_luminance = 0.299 * sobel_color.r + 0.587 * sobel_color.g + 0.114 * sobel_color.b;
		sobel_luminance = 1 - sobel_luminance;
		sobel_color = vec4(sobel_luminance, sobel_luminance, sobel_luminance, sobel_color.a);
		sobel_color = smoothstep(0.2, 0.8, sobel_color);
	}

	vec4 sobel_normal = apply_outline(normal_texture);
	{
		float sobel_luminance = 0.299 * sobel_normal.r + 0.587 * sobel_normal.g + 0.114 * sobel_normal.b;
		sobel_luminance = 1 - sobel_luminance;
		sobel_normal = vec4(sobel_luminance, sobel_luminance, sobel_luminance, sobel_normal.a);
		sobel_normal = smoothstep(0.2, 0.8, sobel_normal);
	}

	final_buffer = vec3(sobel_color.r * sobel_normal.r);
	//final_buffer = vec3(sobel_color);
	//final_buffer = vec3(sobel_normal);
}

vec4 apply_outline(sampler2D texture_buffer) 
{
	vec4 n[9];
	make_kernel(n, texture_buffer, uv);

	vec4 sobel_edge_h = n[2] + (2.0*n[5]) + n[8] - (n[0] + (2.0*n[3]) + n[6]);
  	vec4 sobel_edge_v = n[0] + (2.0*n[1]) + n[2] - (n[6] + (2.0*n[7]) + n[8]);
	return sqrt((sobel_edge_h * sobel_edge_h) + (sobel_edge_v * sobel_edge_v));
}

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