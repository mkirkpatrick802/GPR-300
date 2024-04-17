#version 450

out vec4 fragment_color;

uniform sampler2D positionTex;
uniform sampler2D normalTex;
uniform sampler2D albedoTex;
uniform sampler2D depthTex;

uniform vec3 eye_position;
uniform vec3 light_direction = vec3(1, 1, -1);
uniform vec3 light_color = vec3(1);
uniform vec3 ambient_color = vec3(.3, .4, .46);

in vec2 uv;

struct Material
{
	float ka;
	float kd;
	float ks;
	float shininess;
};

uniform Material material;

struct PointLight
{
	vec3 position;
	float radius;
	vec3 color;
};

#define MAX_POINT_LIGHTS 100
uniform PointLight point_lights[MAX_POINT_LIGHTS];

float attenuateLinear(float dist, float radius)
{
	return clamp(((radius-dist)/radius),0.0,1.0);
}

float ShadowCalculation(vec4 lightPos, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = lightPos.xyz / lightPos.w;
    projCoords = projCoords * .5 + .5;
    float closestDepth = texture(depthTex, projCoords.xy).r;
    float currentDepth = projCoords.z;

	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005); 
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    if(projCoords.z > 1.0)
        shadow = 0;

    return shadow;
}

vec3 calcPointLight(PointLight light,vec3 normal, vec3 pos)
{
	vec3 diff = light.position - pos;
	vec3 to_light = normalize(diff);

	vec3 to_eye = normalize(eye_position - pos);
	vec3 h = normalize(to_light + to_eye);
	float d = length(diff);

	float specular_factor = pow(max(dot(normal,h),0), material.shininess);

	float diffuse_factor = max(dot(normal,to_light),0.0);

	float shadow = ShadowCalculation(vec4(light.position, 1), normal, to_light);

	vec3 new_light_color = (material.kd * diffuse_factor + material.ks * specular_factor) * light.color;
	new_light_color += ambient_color + (material.ka - (shadow * 100.f));
	//new_light_color += ambient_color + material.ka;
	new_light_color *= attenuateLinear(d,light.radius);

	return new_light_color;
}


void main()
{
	vec3 position = texture(positionTex, uv).rgb;
	vec3 albedo = texture(albedoTex, uv).rgb;
	vec3 normal = texture(normalTex, uv).rgb;

	vec3 totalLight = vec3(0);

	PointLight mainLight;
	mainLight.position = light_direction;
	mainLight.color = light_color;
	mainLight.radius = 0;

	for(int i = 0; i < MAX_POINT_LIGHTS; i++)
		totalLight += calcPointLight(point_lights[i], normal, position);

	fragment_color = vec4(albedo * totalLight, 1);
}
