#version 450

uniform sampler2D position_buffer;
uniform sampler2D color_buffer;
uniform sampler2D normal_buffer;
uniform sampler2D lighting_buffer;

uniform sampler2D shadow_buffer;
uniform sampler2D crosshatching_texture;

in vec2 uv;

out vec4 fragment_color;

void main()
{
    vec3 position = texture(position_buffer, uv).rgb;
    vec3 normal = texture(normal_buffer, uv).rgb;
    vec4 color = texture(color_buffer, uv);

    float lightShading = texture(lighting_buffer, uv).r;
    float shadowIntensity = texture(shadow_buffer, uv).r;

    float shadow_value = clamp(lightShading + shadowIntensity, 0, 1);

    vec2 crosshatching_uv = uv * 10;
    //vec2 crosshatching_uv = position.xz * 2;

    float angle = 0;
    //float angle = acos(dot(normalize(normal), vec3(0.0, 1.0, 0.0))); // Angle in radians

    mat2 rotationMatrix = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
    crosshatching_uv = rotationMatrix * crosshatching_uv;

    float crosshatching_color_r = texture(crosshatching_texture, crosshatching_uv).r;
    float crosshatching_color_g = texture(crosshatching_texture, crosshatching_uv).g;
    float crosshatching_color_b = texture(crosshatching_texture, crosshatching_uv).b;
    float crosshatching_alpha = texture(crosshatching_texture, crosshatching_uv).a;

    vec4 final = color;
    if(shadow_value > .75)
    {
        float crosshatching_color = crosshatching_color_r + crosshatching_color_g + crosshatching_color_b;
        final = mix(color, vec4(1 - crosshatching_color), crosshatching_alpha);
    }
    else if(shadow_value > .5)
    {
        float crosshatching_color = crosshatching_color_r + crosshatching_color_g;
        final = mix(color, vec4(1 - crosshatching_color), crosshatching_alpha);
    }
    else if(shadow_value > .25)
    {
        float crosshatching_color = crosshatching_color_r;
        final = mix(color, vec4(1 - crosshatching_color), crosshatching_alpha);
    }

    fragment_color = final;
}