#version 450

out vec4 fragment_color;

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
	
	// final color
	vec3 lighting = (ambient * 1 + (diffuse + specular)) * color;
	fragment_color = vec4(lighting, 1.0);
}