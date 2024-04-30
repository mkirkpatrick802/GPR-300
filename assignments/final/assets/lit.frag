#version 450

layout(location = 0) out vec3 position_buffer;
layout(location = 1) out vec3 normal_buffer;
layout(location = 2) out vec3 color_buffer;
layout(location = 3) out vec3 lighting_buffer;

in Surface
{
	vec3 world_position;
	vec3 world_normal;
	vec2 texture_coord;
}fs_in;

uniform sampler2D diffuse_texture;

uniform vec3 light_position;
uniform vec3 view_position;

void main()
{
	vec3 color = texture(diffuse_texture, fs_in.texture_coord).rgb;
	vec3 normal = normalize(fs_in.world_normal);
	vec3 light_color = vec3(1);

	// ambient
	vec3 ambient = vec3(0.3, 0.4, 0.46) * light_color;

	// diffuse
	vec3 light_direction = normalize(light_position - fs_in.world_position);
	float diff = max(dot(light_direction, normal), 0);
	vec3 diffuse = diff * light_color;

	// specular
	vec3 view_direction = normalize(view_position - fs_in.world_position);
	float spec = 0;
	vec3 halfway_direction = normalize(light_direction + view_direction);
	spec = pow(max(dot(normal, halfway_direction), 0), 64);
	vec3 specular = spec * light_color;
	
	// render to textures
	lighting_buffer = vec3(1 - (ambient * 1 + (diffuse + specular)).r);
	color_buffer = color;
	position_buffer = fs_in.world_position;
	normal_buffer = fs_in.world_normal;
}