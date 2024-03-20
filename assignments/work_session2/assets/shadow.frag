#version 450

out vec4 fragment_color;

in Surface {

	vec3 world_position;
	vec3 world_normal;
	vec2 texture_coord;
    vec4 light_position;

} fs_in;

uniform sampler2D diffuse_texture;
uniform sampler2D shadow_map;

uniform vec3 view_position;

uniform vec3 light_direction = vec3(0, -1, 0);
uniform vec3 light_color = vec3(1);
uniform vec3 ambient_color = vec3(.3, .4, .46);

struct Material
{
	float ka;
	float kd;
	float ks;
	float shininess;
};

uniform float shadow_darkness = .3;

uniform Material material;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * .5 + .5;
    float closestDepth = texture(shadow_map, projCoords.xy).r;
    float currentDepth = projCoords.z;

	float bias = max(0.05 * (1.0 - dot(fs_in.world_normal, light_direction)), 0.005); 
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    if(projCoords.z > 1.0)
        shadow = 0;

    return shadow;
}

void main()
{           
	vec3 normal = normalize(fs_in.world_normal);
	vec3 to_light = -light_direction;
	vec3 to_eye = normalize(view_position - fs_in.world_position);
	vec3 h = normalize(to_light + to_eye);

	float specular_factor = pow(max(dot(normal,h),0), material.shininess);

	float diffuse_factor = max(dot(normal,to_light),0.0);
	vec3 diffuse_color = light_color * diffuse_factor;

	float shadow = ShadowCalculation(fs_in.light_position); 

	vec3 new_light_color = (material.kd * diffuse_factor + material.ks * specular_factor) * light_color;
	new_light_color += ambient_color + (material.ka - (shadow * shadow_darkness));

	vec3 object_color = texture(diffuse_texture, fs_in.texture_coord).rgb;

	fragment_color = vec4(object_color * new_light_color, 1);          
}