#version 450
// Reference https://gist.github.com/Hebali/6ebfc66106459aacee6a9fac029d0115
// Reference https://lettier.github.io/3d-game-shaders-for-beginners/film-grain.html

out vec4 fragment_color;

in vec2 texture_coord_out;

uniform sampler2D screen_texture;

uniform int  render_count;
uniform bool invert_color;
uniform bool depth_edge_detection;
uniform bool grayscale_edge;
uniform float film_grain_amount = .1;

uniform float screen_width;
uniform float screen_height;
uniform float time;

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

vec4 apply_outline(vec2 texture_coords);

void main() {

	vec2 texture_coords = texture_coord_out * render_count;

	vec4 diffuse = texture(screen_texture, texture_coords).rgba;
	vec4 sobel = apply_outline(texture_coords);

	if(grayscale_edge) {

		float sobel_luminance = 0.299 * sobel.r + 0.587 * sobel.g + 0.114 * sobel.b;
		sobel_luminance = 1 - sobel_luminance;
		sobel = vec4(sobel_luminance, sobel_luminance, sobel_luminance, sobel.a);	
	}

	sobel = smoothstep(0.2, 0.8, sobel);

	if(!depth_edge_detection) {

		sobel = vec4(0);
	}

	vec4 edge_color = vec4(0, 0, 0, 1);
	vec4 rgba = mix(diffuse, edge_color, sobel);

	if(invert_color) {

		fragment_color = 1 - rgba;
	}
	else {

		fragment_color = rgba;
	}
}

vec4 apply_outline(vec2 texture_coords) {

	vec4 n[9];
	make_kernel(n, screen_texture, texture_coords);

	vec4 sobel_edge_h = n[2] + (2.0*n[5]) + n[8] - (n[0] + (2.0*n[3]) + n[6]);
  	vec4 sobel_edge_v = n[0] + (2.0*n[1]) + n[2] - (n[6] + (2.0*n[7]) + n[8]);
	return sqrt((sobel_edge_h * sobel_edge_h) + (sobel_edge_v * sobel_edge_v));
}