#version 450

uniform sampler2D screen_texture;
uniform sampler2D crosshatching_texture;

uniform sampler2D shadow_map;

in vec2 uv;

out vec4 fragment_color;

void main()
{
    vec3 sceneColor = texture(screen_texture, uv).rgb;
    float shadowIntensity = texture(shadow_map, uv).r;



    fragment_color = vec4(vec3(shadowIntensity), 1);
}