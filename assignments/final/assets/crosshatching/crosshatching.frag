#version 450

layout(location = 2) out vec3 color_buffer;
layout(location = 4) out vec3 final_buffer;
layout(location = 5) out vec3 crosshatching_buffer;

uniform sampler2D shadow_buffer;
uniform sampler2D crosshatching_texture;
uniform sampler2D color_texture;
uniform sampler2D lighting_texture;

uniform sampler2D position_texture;
uniform vec3 camera_position;

// ImGui Properties
uniform bool  screen_space_hatching = true;
uniform int crosshatching_tiling = 10;

uniform float crosshatching_full_threshold = .75;
uniform float crosshatching_half_threshold = .5;
uniform float crosshatching_first_threshold = .2;

in vec2 uv;

void main()
{
    // Sample shadow intensity
    float shadowIntensity = texture(shadow_buffer, uv).r;

    // Sample lighting value
    vec3 lighting_value = texture(lighting_texture, uv).rgb;
    float shadow_value = clamp(lighting_value.r + shadowIntensity, 0, 1);

    vec2 crosshatching_uv = uv * crosshatching_tiling;
    if(!screen_space_hatching)
    {
        // Calculate distance from fragment to camera
        float dist = length(texture(position_texture, uv).rgb - camera_position);

        // Scale the UVs based on distance
        crosshatching_uv = uv * dist * 3;
    }

    // Sample crosshatching texture
    float crosshatching_color_r = texture(crosshatching_texture, crosshatching_uv).r;
    float crosshatching_color_g = texture(crosshatching_texture, crosshatching_uv).g;
    float crosshatching_color_b = texture(crosshatching_texture, crosshatching_uv).b;
    float crosshatching_alpha = texture(crosshatching_texture, crosshatching_uv).a;

    // Sample color texture
    vec3 color = texture(color_texture, uv).rgb;

    // Mix color and crosshatching based on shadow value
    vec4 final = vec4(color, 1);
    if(shadow_value > crosshatching_full_threshold)
    {
        float crosshatching_color = crosshatching_color_r + crosshatching_color_g + crosshatching_color_b;
        final = mix(vec4(color, 1), vec4(1 - crosshatching_color), crosshatching_alpha);
    }
    else if(shadow_value > crosshatching_half_threshold)
    {
        float crosshatching_color = crosshatching_color_r + crosshatching_color_g;
        final = mix(vec4(color, 1), vec4(1 - crosshatching_color), crosshatching_alpha);
    }
    else if(shadow_value > crosshatching_first_threshold)
    {
        float crosshatching_color = crosshatching_color_r;
        final = mix(vec4(color, 1), vec4(1 - crosshatching_color), crosshatching_alpha);
    }

    // Output to buffers
    crosshatching_buffer = final.rgb;
    color_buffer = color;
}