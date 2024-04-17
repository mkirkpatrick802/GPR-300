#version 450

out vec4 fragment_color;

in Surface
{
	vec3 world_position;
	vec3 world_normal;
	vec2 texture_coord;
	vec4 lightspace_position;
}fs_in;

uniform sampler2D diffuse_texture;
uniform sampler2D shadow_map;

uniform vec3 light_position;
uniform vec3 view_position;

float ShadowCalculation(vec4 lightspace_position)
{
    // perform perspective divide
    vec3 projCoords = lightspace_position.xyz / lightspace_position.w;

    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadow_map, projCoords.xy).r; 

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

	// calculate shadows
	vec3 normal = normalize(fs_in.world_normal);
	vec3 light_direction = normalize(light_position - fs_in.world_position);

	float bias = max(0.05 * (1.0 - dot(normal, light_direction)), 0.005); 
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	// PCF
	vec2 texelSize = 3.f / textureSize(shadow_map, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadow_map, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}

	shadow /= 9.0;

	if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

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

	// shadows
	float shadow = ShadowCalculation(fs_in.lightspace_position);
	
	// final color
	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
	//fragment_color = vec4(lighting, 1.0);
	fragment_color = vec4(vec3(shadow), 1);
}