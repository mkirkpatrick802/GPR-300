#version 450

layout(location = 4) out vec3 final_buffer;

uniform sampler2D shadow_buffer;
uniform sampler2D crosshatching_texture;

uniform sampler2D color_texture;
uniform sampler2D lighting_texture;

in vec2 uv;

void main()
{
    float shadowIntensity = texture(shadow_buffer, uv).r;

    vec3 lighting_value = texture(lighting_texture, uv).rgb;
    float shadow_value = clamp(lighting_value.r + shadowIntensity, 0, 1);

    vec2 crosshatching_uv = uv * 10;
    float crosshatching_color_r = texture(crosshatching_texture, crosshatching_uv).r;
    float crosshatching_color_g = texture(crosshatching_texture, crosshatching_uv).g;
    float crosshatching_color_b = texture(crosshatching_texture, crosshatching_uv).b;
    float crosshatching_alpha = texture(crosshatching_texture, crosshatching_uv).a;

    vec3 color = texture(color_texture, uv).rgb;

    vec4 final = vec4(color, 1);
    if(shadow_value > .75)
    {
        float crosshatching_color = crosshatching_color_r + crosshatching_color_g + crosshatching_color_b;
        final = mix(vec4(color, 1), vec4(1 - crosshatching_color), crosshatching_alpha);
    }
    else if(shadow_value > .5)
    {
        float crosshatching_color = crosshatching_color_r + crosshatching_color_g;
        final = mix(vec4(color, 1), vec4(1 - crosshatching_color), crosshatching_alpha);
    }
    else if(shadow_value > .25)
    {
        float crosshatching_color = crosshatching_color_r;
        final = mix(vec4(color, 1), vec4(1 - crosshatching_color), crosshatching_alpha);
    }

	// render to textures
	final_buffer = final.rgb;
}