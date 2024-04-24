#version 450

uniform sampler2D position_buffer;
uniform sampler2D color_buffer;
uniform sampler2D normal_buffer;

uniform sampler2D shadow_buffer;
uniform sampler2D crosshatching_texture;

in vec2 uv;

out vec4 fragment_color;

void main()
{
    vec3 position = texture(position_buffer, uv).rgb;
    vec3 normal = texture(normal_buffer, uv).rgb;
    vec4 color = texture(color_buffer, uv);

    float shadowIntensity = texture(shadow_buffer, uv).r;

    // TODO: Sample texture with vec3
    vec2 crosshatching_uv = position.xy + normal.xy;
    float crosshatching_color = 1 - texture(crosshatching_texture, crosshatching_uv).r;
    float crosshatching_alpha = texture(crosshatching_texture, crosshatching_uv).a;

    vec4 final = color;

    if(shadowIntensity > 0)
        final = mix(color, vec4(crosshatching_color), crosshatching_alpha);

    fragment_color = final;
}