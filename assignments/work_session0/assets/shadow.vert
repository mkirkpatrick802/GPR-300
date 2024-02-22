#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texture_coord;

out Surface {

	vec3 world_position;
	vec3 world_normal;
	vec2 texture_coord;
    vec4 light_position;

} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 light_matrix;

void main() {
	
	vs_out.world_position = vec3(model * vec4(position, 1.0));
	vs_out.world_normal = transpose(inverse(mat3(model))) * normal;
	vs_out.texture_coord = texture_coord;
	vs_out.light_position = light_matrix * vec4(vs_out.world_position, 1.0);

	gl_Position = projection * view * vec4(vs_out.world_position, 1.0);
}