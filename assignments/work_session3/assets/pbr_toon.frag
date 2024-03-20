#version 450

out vec4 fragment_color;

in Surface {
	vec3 world_position;
	vec3 world_normal;
	vec2 texture_coord;
}fs_in;

uniform vec3 eye_position;
uniform vec3 light_direction = vec3(0, -1, 0);
uniform vec3 light_color = vec3(1);
uniform vec3 ambient_color = vec3(.3, .4, .46);

uniform sampler2D toon_index;

struct PBRMaterial {
	sampler2D color;
	sampler2D albedo;
	sampler2D metallic;
	sampler2D occulusion;
	sampler2D specular;
};

uniform PBRMaterial materials;

const float PIE = 3.14;

// cache
vec3 col;
float mtl;
float rgh;
float spec;
float ao;

float NdotH;
float NdotV;
float VdotH;
float VdotN;
float LdotN;
float VdotL;

// GGX
float D(float alpha)
{
	float denominator = PIE * pow(pow(NdotH, 2) * (pow(alpha, 2) - 1) + 1, 2);
	return pow(alpha, 2) / denominator;
}

// Backmann Function
float G1(float alpha, float X)
{
	float k = alpha;
	float denominator = X * (1 - k) + k; 
	return X / denominator;
}

// Smith Model
float G(float alpha)
{
	return G1(alpha, LdotN) * G1(alpha, VdotN);
}

// Fresnel Function
vec3 F(vec3 F0)
{
	return F0 + (vec3(1) - F0) * pow((1 - VdotH), 5);
}

vec3 PBR()
{
	vec3 F0 = vec3(.4);
	F0 = col;
	F0 = vec3(mtl);

	vec3 kS = F(F0);
	vec3 kD = (vec3(1) - kS) * (1 - mtl);

	vec3 lambert = col / PIE;

	float alpha = pow(rgh, 2);

	vec3 cookNumerator = D(alpha) * G(alpha) * kS;
	float cookDenominator = max(4 * VdotN * LdotN, 0.000001);
	vec3 cook = cookNumerator / cookDenominator;

	vec3 BDRF = (kD * lambert) + cook;

	return BDRF * light_color * LdotN;
}

void main()
{
	// pre-sample
	col  = texture(materials.color, fs_in.texture_coord).rgb;
	mtl  = texture(materials.metallic, fs_in.texture_coord).r;
	rgh  = texture(materials.occulusion, fs_in.texture_coord).r;
	spec = texture(materials.specular, fs_in.texture_coord).r;
	ao   = texture(materials.albedo, fs_in.texture_coord).r;

	vec3 N = normalize(fs_in.world_normal);
	vec3 V = normalize(eye_position);
	vec3 L = normalize(light_direction);
	vec3 H = normalize(V + L);

	// pre-compute
	NdotH = max(dot(N, H), 0);
	NdotV = max(dot(N, V), 0);
	VdotH = max(dot(V, H), 0);
	VdotN = max(dot(V, N), 0);
	LdotN = max(dot(L, N), 0);
	VdotL = max(dot(V, L), 0);

	vec3 reflection_direction = reflect(-L, N);
	float spec_amount = pow(max(dot(V, reflection_direction), 0), 32);
	vec3 light_spot = spec * spec_amount * light_color;

	//float shaded_value = (PBR() + light_spot + .15).x;
	//float index_value = texture(toon_index, vec2(.2, .2)).x;


	vec3 final_color = (col * ambient_color * ao) + PBR() + light_spot;
	fragment_color = vec4(final_color + (col * .5), 1);

	//fragment_color = vec4(vec3(index_value), 1);
}