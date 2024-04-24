#version 450

out vec4 fragment_color;

in vec3 world_position;
in vec3 world_normal;
in vec4 lightspace_position;

uniform sampler2D depth_map;
uniform vec3 light_position;
uniform vec3 view_position;

float ShadowCalculation(vec4 lightspace_position)
{
    // perform perspective divide
    vec3 projCoords = lightspace_position.xyz / lightspace_position.w;

    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(depth_map, projCoords.xy).r; 

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

	// calculate shadows
	vec3 normal = normalize(world_normal);
	vec3 light_direction = normalize(light_position - world_position);

	float bias = max(0.05 * (1.0 - dot(normal, light_direction)), 0.005); 
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	// PCF
	vec2 texelSize = 3.f / textureSize(depth_map, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(depth_map, projCoords.xy + vec2(x, y) * texelSize).r; 
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
	vec3 normal = normalize(world_normal);

	// ambient
	float ambient = 0.0;

	// diffuse
	vec3 light_direction = normalize(light_position - world_position);
	float diff = max(dot(light_direction, normal), 0);

	// specular
	vec3 view_direction = normalize(view_position - world_position);
	float spec = 0;
	vec3 halfway_direction = normalize(light_direction + view_direction);
	spec = pow(max(dot(normal, halfway_direction), 0), 64);

	// final shadow
	float shadow = ShadowCalculation(lightspace_position);

	fragment_color = vec4(vec3(shadow), 1);
}